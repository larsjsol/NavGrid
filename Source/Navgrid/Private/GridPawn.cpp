// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"
#if WITH_EDITORONLY_DATA
#include "Editor.h"
#endif

// Sets default values
AGridPawn::AGridPawn()
	: Super()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(SceneRoot);

	BoundsCapsule = CreateDefaultSubobject<UCapsuleComponent>("Capsule");
	BoundsCapsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	BoundsCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // So we get mouse over events
	BoundsCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block); // So we get mouse over events
	BoundsCapsule->ShapeColor = FColor::Magenta;
	BoundsCapsule->SetupAttachment(SceneRoot);

	MovementComponent = CreateDefaultSubobject<UGridMovementComponent>("MovementComponent");
	MovementComponent->OnMovementEnd().AddUObject(this, &AGridPawn::OnMoveEnd);
	MovementComponent->SetUpdatedComponent(BoundsCapsule);

	TurnComponent = CreateDefaultSubobject<UTurnComponent>("TurnComponent");

	MovementCollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>("MovementCollisionCapsule");
	MovementCollisionCapsule->SetupAttachment(SceneRoot);
	MovementCollisionCapsule->SetRelativeLocation(FVector(0, 0, 100));
	MovementCollisionCapsule->SetCapsuleHalfHeight(50);
	MovementCollisionCapsule->SetCapsuleRadius(30);
	MovementCollisionCapsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	SelectedHighlight = CreateDefaultSubobject<UStaticMeshComponent>("SelectedHighlight");
	SelectedHighlight->SetupAttachment(SceneRoot);
	UStaticMesh *Selected = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'")).Object;
	SelectedHighlight->SetStaticMesh(Selected);
	SelectedHighlight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectedHighlight->SetVisibility(false);

	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetupAttachment(MovementCollisionCapsule);
	Arrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bHumanControlled = true;

	/* bind mouse events*/
	OnClicked.AddDynamic(this, &AGridPawn::Clicked);
#if WITH_EDITORONLY_DATA
	USelection::SelectObjectEvent.AddUObject(this, &AGridPawn::OnObjectSelectedInEditor);
#endif
}

void AGridPawn::BeginPlay()
{
	Super::BeginPlay();

	ATurnManager *TurnManager = GetWorld()->GetGameState<ANavGridGameState>()->GetTurnManager();
	TurnManager->OnRoundStart().AddDynamic(this, &AGridPawn::OnRoundStart);
	TurnManager->OnTurnStart().AddDynamic(this, &AGridPawn::OnAnyTurnStart);
	TurnManager->OnTurnEnd().AddDynamic(this, &AGridPawn::OnAnyTurnEnd);
	TurnManager->OnReadyForInput().AddDynamic(this, &AGridPawn::OnAnyPawnReadyForInput);

	SetGenericTeamId(TeamId);

#if WITH_EDITORONLY_DATA
	GEditor->GetTimerManager()->ClearTimer(PreviewTimerHandle);
#endif //WITH_EDITORONLY_DATA
}

void AGridPawn::OnConstruction(const FTransform & Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA
	if (bPreviewTiles && IsSelectedInEditor())
	{
		GEditor->GetTimerManager()->SetTimer(PreviewTimerHandle, this, &AGridPawn::UpdatePreviewTiles, 1, true);
	}
	else
	{
		GEditor->GetTimerManager()->ClearTimer(PreviewTimerHandle);
	}
#endif //WITH_EDITORONLY_DATA
}

void AGridPawn::SetGenericTeamId(const FGenericTeamId & InTeamId)
{
	ANavGridGameState *State = GetWorld()->GetGameState<ANavGridGameState>();
	ATurnManager *NewTM = State->GetTurnManager();
	if (IsValid(NewTM))
	{
		NewTM->RegisterTurnComponent(TurnComponent);
	}

	TeamId = InTeamId;
}

void AGridPawn::OnAnyTurnStart(UTurnComponent *InTurnComponent)
{
	if (InTurnComponent == TurnComponent)
	{
		OnTurnStart();
	}
}

void AGridPawn::OnTurnStart()
{
	ANavGrid *Grid = MovementComponent->GetNavGrid();
	if (IsValid(Grid) && Grid->EnableVirtualTiles)
	{
		GenerateVirtualTiles();
	}
	MovementComponent->ConsiderUpdateCurrentTile();
	if (SnapToGrid)
	{
		MovementComponent->SnapToGrid();
	}

	SelectedHighlight->SetVisibility(true);

	TurnComponent->OwnerReadyForInput();
	if (!bHumanControlled)
	{
		PlayAITurn();
	}
}

void AGridPawn::OnAnyTurnEnd(UTurnComponent *InTurnComponent)
{
	if (InTurnComponent == TurnComponent)
	{
		OnTurnEnd();
	}
}

void AGridPawn::OnTurnEnd()
{
	SelectedHighlight->SetVisibility(false);
	MovementComponent->HidePath();
}

void AGridPawn::OnMoveEnd()
{
	//Moving costs one action point
	TurnComponent->RemainingActionPoints--;
	TurnComponent->EndTurn();
}

void AGridPawn::OnAnyPawnReadyForInput(UTurnComponent * InTurnComponent)
{
	if (InTurnComponent == TurnComponent)
	{
		OnPawnReadyForInput();
	}
}

void AGridPawn::PlayAITurn()
{
	//default implementation is simply to end turn
	TurnComponent->RemainingActionPoints = 0;
	TurnComponent->EndTurn();
}

EGridPawnState AGridPawn::GetState() const
{
	if (!TurnComponent->MyTurn())
	{
		return EGridPawnState::WaitingForTurn;
	}
	else if (MovementComponent->Velocity.Size() > 0)
	{
		return EGridPawnState::Busy;
	}
	else
	{
		return EGridPawnState::Ready;
	}
}

/** Can this pawn start its turn right now?
  *  1) It must be the pawns teams turn
  *  2) It must be in the WaitingForTurn state
  *  3) The pawn currently in its turn must be idle
*/
bool AGridPawn::CanBeSelected()
{
	ANavGridGameState *GameState = Cast<ANavGridGameState>(GetWorld()->GetGameState());
	if (GameState)
	{
		ATurnManager *TurnManager = GameState->GetTurnManager();
		if (TurnManager && TurnManager->GetCurrentTeam() == TeamId && GetState() == EGridPawnState::WaitingForTurn)
		{
			AGridPawn *CurrentPawn = Cast<AGridPawn>(TurnManager->GetCurrentComponent()->GetOwner());
			return CurrentPawn->GetState() != EGridPawnState::Busy;
		}
	}
	return false;
}

bool AGridPawn::CanMoveTo(const UNavTileComponent & Tile)
{
	if (MovementComponent->GetTile() != &Tile &&
		Tile.LegalPositionAtEndOfTurn(MovementComponent->MaxWalkAngle, MovementComponent->AvailableMovementModes))
	{
		TArray<UNavTileComponent *> InRange;
		MovementComponent->GetNavGrid()->GetTilesInRange(this, true, InRange);
		if (Tile.Distance <= MovementComponent->MovementRange)
		{
			return true;
		}
	}
	return false;
}

void AGridPawn::MoveTo(const UNavTileComponent & Tile)
{
	MovementComponent->MoveTo(Tile);
	MovementComponent->HidePath();
}

UNavTileComponent *AGridPawn::ConsiderGenerateVirtualTile()
{
	if (!IsValid(MovementComponent->GetTile()) && MovementComponent->GetNavGrid()->EnableVirtualTiles)
	{
		return MovementComponent->GetNavGrid()->ConsiderGenerateVirtualTile(GetActorLocation());
	}
	return MovementComponent->GetTile();
}

void AGridPawn::GenerateVirtualTiles()
{
	MovementComponent->GetNavGrid()->GenerateVirtualTiles(this);
	MovementComponent->ConsiderUpdateCurrentTile();
}

void AGridPawn::Clicked(AActor *ClickedActor, FKey PressedKey)
{
	if (CanBeSelected())
	{
		TurnComponent->RequestStartTurn(true);
	}
}

#if WITH_EDITORONLY_DATA
void AGridPawn::OnObjectSelectedInEditor(UObject * SelectedObject)
{
	AGridPawn *SelectedPawn = Cast<AGridPawn>(SelectedObject);
	if (SelectedPawn && SelectedPawn->bPreviewTiles)
	{
		if (SelectedPawn == this)
		{
			GEditor->GetTimerManager()->SetTimer(PreviewTimerHandle, this, &AGridPawn::UpdatePreviewTiles, 1, true);
		}
		else
		{
			GEditor->GetTimerManager()->ClearTimer(PreviewTimerHandle);
		}
	}
}

void AGridPawn::UpdatePreviewTiles()
{
	// check if a previewgrid already exist
	if (!IsValid(PreviewGrid))
	{
		for (TActorIterator<ANavGrid> Itr(GetWorld()); Itr; ++Itr)
		{
			if (Itr->Tags.Contains(FName("PreviewGrid")))
			{
				PreviewGrid = *Itr;
				break;
			}
		}
	}

	// create a preview grid if no grid already exists in the level
	if (!IsValid(PreviewGrid))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bAllowDuringConstructionScript = true;
		SpawnParams.bTemporaryEditorActor = true;
		SpawnParams.Name = FName(*FString::Printf(TEXT("PreviewNavGrid_%s"), *GetName()));
		PreviewGrid = GetWorld()->SpawnActor<ANavGrid>(SpawnParams);
		PreviewGrid->TileSize = PreviewTileSize;
		PreviewGrid->Tags.Add(FName("PreviewGrid"));

		TArray<UNavTileComponent *> Tiles;
		ANavGrid::GetEveryTile(Tiles, GetWorld());
		for (UNavTileComponent *Tile : Tiles)
		{
			Tile->SetGrid(PreviewGrid);
		}
	}

	GenerateVirtualTiles();
	TArray<UNavTileComponent *> Tiles;
	PreviewGrid->GetTilesInRange(this, true, Tiles);
	for (UNavTileComponent *Tile : Tiles)
	{
		Tile->SetHighlight("Movable");
	}
}
#endif //WITH_EDITORONLY_DATA

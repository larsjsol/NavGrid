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
	BoundsCapsule->SetCollisionProfileName("Pawn");
	BoundsCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
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

	// dont place tiles on top of pawns
	Tags.AddUnique(ANavGrid::DisableVirtualTilesTag);
}

void AGridPawn::BeginPlay()
{
	Super::BeginPlay();

	ATurnManager *TurnManager =TurnComponent->GetTurnManager();
	if (IsValid(TurnManager))
	{
		TurnManager->OnRoundStart().AddDynamic(this, &AGridPawn::OnRoundStart);
		TurnManager->OnTurnStart().AddDynamic(this, &AGridPawn::OnAnyTurnStart);
		TurnManager->OnTurnEnd().AddDynamic(this, &AGridPawn::OnAnyTurnEnd);
		TurnManager->OnTeamTurnStart().AddDynamic(this, &AGridPawn::OnAnyTeamTurnStart);
		TurnManager->OnTeamTurnEnd().AddDynamic(this, &AGridPawn::OnAnyTeamTurnEnd);
		TurnManager->OnReadyForInput().AddDynamic(this, &AGridPawn::OnAnyPawnReadyForInput);
	}

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
	// we must unregister before we change the team id
	TurnComponent->UnregisterWithTurnManager();
	TeamId = InTeamId;
	TurnComponent->RegisterWithTurnManager();
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

void AGridPawn::OnAnyTeamTurnStart(const FGenericTeamId & InTeamId)
{
	if (InTeamId == GetGenericTeamId())
	{
		OnTeamTurnStart();
	}
}

void AGridPawn::OnAnyTeamTurnEnd(const FGenericTeamId & InTeamId)
{
	if (InTeamId == GetGenericTeamId())
	{
		OnTeamTurnEnd();
	}
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
  *  4) It must not have used all its action points
*/
bool AGridPawn::CanBeSelected()
{
	ANavGridGameState *GameState = Cast<ANavGridGameState>(GetWorld()->GetGameState());
	if (IsValid(GameState))
	{
		ATurnManager *TurnManager = GameState->GetTurnManager();
		if (IsValid(TurnManager))
		{
			// 1) It must be the pawns teams turn
			if (TurnManager->GetCurrentTeam() != TeamId)
			{
				return false;
			}
			// 2) It must be in the WaitingForTurn state
			if (GetState() != EGridPawnState::WaitingForTurn)
			{
				return false;
			}
			// 3) The pawn currently in its turn must be idle
			AGridPawn* CurrentPawn = Cast<AGridPawn>(TurnManager->GetCurrentComponent()->GetOwner());
			if (!IsValid(CurrentPawn) || CurrentPawn->GetState() == EGridPawnState::Busy)
			{
				return false;
			}
			// 4) It must action points remaining
			return TurnComponent->RemainingActionPoints > 0;
		}
	}
	return false;
}

bool AGridPawn::CanMoveTo(const UNavTileComponent & Tile)
{
	if (MovementComponent->GetTile() != &Tile &&
		Tile.LegalPositionAtEndOfTurn(MovementComponent->AvailableMovementModes))
	{
		TArray<UNavTileComponent *> InRange;
		MovementComponent->GetNavGrid()->GetTilesInRange(this, InRange);
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

void AGridPawn::Clicked(AActor *ClickedActor, FKey PressedKey)
{
	if (CanBeSelected())
	{
		TurnComponent->RequestStartTurn();
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

	TArray<UNavTileComponent *> Tiles;
	PreviewGrid->GetTilesInRange(this, Tiles);
	for (UNavTileComponent *Tile : Tiles)
	{
		Tile->SetHighlight("Movable");
	}
}
#endif //WITH_EDITORONLY_DATA

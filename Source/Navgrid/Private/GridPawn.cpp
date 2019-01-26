// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

// Sets default values
AGridPawn::AGridPawn()
	: Super()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(Scene);

	MovementComponent = CreateDefaultSubobject<UGridMovementComponent>("MovementComponent");
	MovementComponent->OnMovementEnd().AddUObject(this, &AGridPawn::OnMoveEnd);

	TurnComponent = CreateDefaultSubobject<UTurnComponent>("TurnComponent");

	MovementCollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>("MovementCollisionCapsule");
	MovementCollisionCapsule->SetupAttachment(Scene);
	MovementCollisionCapsule->SetRelativeLocation(FVector(0, 0, 100)); //Above the ground to avoid collisions
	MovementCollisionCapsule->SetCapsuleHalfHeight(50);
	MovementCollisionCapsule->SetCapsuleRadius(30);
	MovementCollisionCapsule->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	MovementComponent->SetUpdatedComponent(MovementCollisionCapsule);

	SelectedHighlight = CreateDefaultSubobject<UStaticMeshComponent>("SelectedHighlight");
	SelectedHighlight->SetupAttachment(Scene);
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
}

void AGridPawn::BeginPlay()
{
	Super::BeginPlay();

	TurnComponent->OnRoundStart().AddUObject(this, &AGridPawn::OnRoundStart);
	TurnComponent->OnTurnStart().AddUObject(this, &AGridPawn::OnTurnStart);
	// manually fire OnRoundStart() and OnTurnStart() if its our turn as that means we have missed the event
	if (TurnComponent->MyTurn())
	{
		OnRoundStart();
		OnTurnStart();
	}
	TurnComponent->OnTurnEnd().AddUObject(this, &AGridPawn::OnTurnEnd);
	TurnComponent->OnPawnReady().AddUObject(this, &AGridPawn::OnPawnReady);

	auto *State = GetWorld()->GetGameState<ANavGridGameState>();
	check(State && State->Grid);
	Grid = State->Grid;
	SelectedHighlight->SetRelativeLocation(FVector(0, 0, Grid->UIOffset));

	SetGenericTeamId(TeamId);
}

void AGridPawn::SetGenericTeamId(const FGenericTeamId & InTeamId)
{
	ANavGridGameState *State = GetWorld()->GetGameState<ANavGridGameState>();
	ATurnManager *OldTM = State->GetTurnManager(TeamId);
	if (IsValid(OldTM))
	{
		OldTM->UnRegister(TurnComponent);
	}
	ATurnManager *NewTM = State->GetTurnManager(InTeamId);
	if (IsValid(NewTM))
	{
		NewTM->Register(TurnComponent);
	}

	TeamId = InTeamId;
}

void AGridPawn::OnTurnStart()
{
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

	if (bHumanControlled)
	{
		TurnComponent->BroadcastReadyForPlayerInput();
	}
	else
	{
		PlayAITurn();
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
  *  1) It's turn manager must be in its turn
  *  2) It must be in the WaitingForTurn state
  *  3) The pawn currently in its turn must be idle
*/
bool AGridPawn::CanBeSelected()
{
	ANavGridGameState *GameState = Cast<ANavGridGameState>(GetWorld()->GetGameState());
	if (GameState)
	{
		ATurnManager *TurnManager = GameState->GetTurnManager(TeamId);
		if (TurnManager && TurnManager->MyTurn() && GetState() == EGridPawnState::WaitingForTurn)
		{
			AGridPawn *CurrentPawn = Cast<AGridPawn>(TurnManager->GetCurrentComponent()->GetOwner());
			return CurrentPawn->GetState() != EGridPawnState::Busy;
		}
	}
	return false;
}

bool AGridPawn::CanMoveTo(const UNavTileComponent & Tile)
{
	if (Tile.LegalPositionAtEndOfTurn(MovementComponent->MaxWalkAngle, MovementComponent->AvailableMovementModes))
	{
		TArray<UNavTileComponent *> InRange;
		Grid->GetTilesInRange(this, true, InRange);
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
	if (!IsValid(MovementComponent->GetTile()) && Grid->EnableVirtualTiles)
	{
		Grid->GenerateVirtualTile(this);
		MovementComponent->ConsiderUpdateCurrentTile();
	}
	return MovementComponent->GetTile();
}

void AGridPawn::GenerateVirtualTiles()
{
	check(Grid);
	Grid->GenerateVirtualTiles(this);
	MovementComponent->ConsiderUpdateCurrentTile();
}

void AGridPawn::Clicked(AActor *ClickedActor, FKey PressedKey)
{
	if (CanBeSelected())
	{
		TurnComponent->RequestStartTurn();
	}
}

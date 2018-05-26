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

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
	CapsuleComponent->SetupAttachment(Scene);
	CapsuleComponent->SetRelativeLocation(FVector(0, 0, 100)); //Above the ground to avoid collisions
	CapsuleComponent->SetCapsuleHalfHeight(50);
	CapsuleComponent->SetCapsuleRadius(30);
	CapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // So we get mouse events

	SelectedHighlight = CreateDefaultSubobject<UStaticMeshComponent>("SelectedHighlight");
	SelectedHighlight->SetupAttachment(Scene);
	UStaticMesh *Selected = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'")).Object;
	SelectedHighlight->SetStaticMesh(Selected);
	SelectedHighlight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectedHighlight->SetVisibility(false);

	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetupAttachment(Scene);
	Arrow->SetRelativeLocation(FVector(0, 0, 50));
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
	TurnComponent->OnTurnEnd().AddUObject(this, &AGridPawn::OnTurnEnd);

	auto *State = GetWorld()->GetGameState<ANavGridGameState>();
	Grid = State->Grid;
	SelectedHighlight->SetRelativeLocation(FVector(0, 0, Grid->UIOffset));

	ATurnManager *TM = State->GetTurnManager(TeamID);
	check(TM);
	TM->Register(TurnComponent);

	if (SnapToGrid)
	{
		MovementComponent->SnapToGrid();
	}
}

void AGridPawn::OnTurnStart()
{
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

void AGridPawn::Clicked(AActor *ClickedActor, FKey PressedKey)
{
	TurnComponent->RequestStartTurn();
}

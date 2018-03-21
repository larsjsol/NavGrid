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
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	SelectedHighlight = CreateDefaultSubobject<UStaticMeshComponent>("SelectedHighlight");
	SelectedHighlight->SetupAttachment(Scene);
	UStaticMesh *Selected = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'")).Object;
	SelectedHighlight->SetStaticMesh(Selected);
	SelectedHighlight->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SelectedHighlight->SetVisibility(false);

	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetupAttachment(Scene);
	Arrow->SetRelativeLocation(FVector(0, 0, 50));
}

void AGridPawn::BeginPlay()
{
	Super::BeginPlay();

	TurnComponent->OnRoundStart().BindUObject(this, &AGridPawn::OnRoundStart);
	TurnComponent->OnTurnStart().BindUObject(this, &AGridPawn::OnTurnStart);
	TurnComponent->OnTurnEnd().BindUObject(this, &AGridPawn::OnTurnEnd);

	auto *State = GetWorld()->GetGameState<ANavGridGameState>();
	Grid = State->Grid;
	SelectedHighlight->SetRelativeLocation(FVector(0, 0, Grid->UIOffset));

	ATurnManager *TM = State->GetTurnManager(TeamId);
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

	if (TurnComponent->GetPlayerController())
	{
		// figure out which tiles are in range and wait for player input
		Grid->CalculateTilesInRange(this, true);
	}
	else
	{
		PlayAITurn();
	}
}

void AGridPawn::OnTurnEnd()
{
	SelectedHighlight->SetVisibility(false);
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

bool AGridPawn::IsBusy()
{
	return MovementComponent->Velocity.Size() > 0;
}

bool AGridPawn::CanMoveTo(const UNavTileComponent & Tile)
{
	UNavTileComponent *Location = Grid->GetTile(GetActorLocation());
	if (Location && Location != &Tile && Tile.LegalPositionAtEndOfTurn(MovementComponent->MaxWalkAngle, MovementComponent->AvailableMovementModes))
	{
		TArray<UNavTileComponent *> InRange;
		Grid->GetTilesInRange(InRange);
		if (InRange.Contains(&Tile) && MovementComponent->CreatePath(Tile))
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

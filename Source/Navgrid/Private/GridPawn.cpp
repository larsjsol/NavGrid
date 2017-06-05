// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

// Sets default values
AGridPawn::AGridPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Scene = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SetRootComponent(Scene);
	MovementComponent = CreateDefaultSubobject<UGridMovementComponent>("MovementComponent");
	TurnComponent = CreateDefaultSubobject<UTurnComponent>("TurnComponent");

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComponent");
	CapsuleComponent->SetupAttachment(Scene);
	CapsuleComponent->SetRelativeLocation(FVector(0, 0, 100)); //just above the floor for the default height (44 * 2)
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

	SelectedHighlight = CreateDefaultSubobject<UStaticMeshComponent>("SelectedHighlight");
	SelectedHighlight->SetupAttachment(Scene);
	UStaticMesh *Selected = ConstructorHelpers::FObjectFinder<UStaticMesh>(
		TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'")).Object;
	SelectedHighlight->SetStaticMesh(Selected);
	SelectedHighlight->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SelectedHighlight->SetVisibility(false);
	SelectedHighlight->SetRelativeLocation(FVector(0, 0, 50));

	TurnComponent->OnRoundStart().AddUObject(this, &AGridPawn::OnRoundStart);
	TurnComponent->OnTurnStart().AddUObject(this, &AGridPawn::OnTurnStart);
	TurnComponent->OnTurnEnd().AddUObject(this, &AGridPawn::OnTurnEnd);

	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetupAttachment(Scene);
}

void AGridPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AGridPawn::OnRoundStart()
{
	TurnComponent->bCanStillActThisRound = true;
}

void AGridPawn::OnTurnStart()
{
	SelectedHighlight->SetVisibility(true);

	Grid = ANavGrid::GetNavGrid(GetWorld());
	if (Grid)
	{
		if (SnapToGrid)
		{
			SetActorLocation(Grid->ToRoundedTileLocation(GetActorLocation()));
			Grid->GenerateVirtualTiles(this);
			Grid->CalculateTilesInRange(Grid->GetTile(GetActorLocation()), this, true);
		}
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("%s was unable to find a NavGrid in level"), *this->GetName());
	}
}

void AGridPawn::OnTurnEnd()
{
	SelectedHighlight->SetVisibility(false);
	TurnComponent->bCanStillActThisRound = false;
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

// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "GridPawn.h"
#include "GridMovementComponent.h"
#include "TurnComponent.h"


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
	SelectedHighlight->SetRelativeLocation(FVector(0, 0, 20));

	TurnComponent->OnTurnStart().AddUObject(this, &AGridPawn::OnTurnStart);
	TurnComponent->OnTurnEnd().AddUObject(this, &AGridPawn::OnTurnEnd);

	Arrow = CreateDefaultSubobject<UArrowComponent>("Arrow");
	Arrow->SetupAttachment(Scene);
}

void AGridPawn::BeginPlay()
{
	Super::BeginPlay();

	TActorIterator<ANavGrid>GridItr(GetWorld());
	if (SnapToGrid && GridItr)
	{
		UNavTileComponent *Tile = GridItr->GetTile(GetActorLocation());
		if (Tile) { SetActorLocation(Tile->GetComponentLocation()); }
	}
}

void AGridPawn::OnTurnStart()
{
	SelectedHighlight->SetVisibility(true);
}

void AGridPawn::OnTurnEnd()
{
	SelectedHighlight->SetVisibility(false);
}

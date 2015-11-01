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
	CapsuleComponent->AttachParent = Scene;
	CapsuleComponent->SetRelativeLocation(FVector(0, 0, 60)); //just above the floor the default height (44 * 2)
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "ExampleGridPawn.h"

#include "Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"

AExampleGridPawn::AExampleGridPawn()
	:Super()
{
	MovementCollisionCapsule->SetCapsuleHalfHeight(30);
	MovementCollisionCapsule->SetRelativeLocation(FVector(0, 0, 50));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	UStaticMesh *Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cone.Cone'")).Object;
	StaticMesh->SetStaticMesh(Mesh);
	StaticMesh->SetRelativeLocation(-MovementCollisionCapsule->GetRelativeLocation() + FVector(0, 0, 50));
	StaticMesh->SetupAttachment(GetRootComponent());

	/* Show the arrow in-game so we can which way the pawn is facing */
	Arrow->SetHiddenInGame(false);
}

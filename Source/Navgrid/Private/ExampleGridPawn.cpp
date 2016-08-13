// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"
#include "ExampleGridPawn.h"

#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"

AExampleGridPawn::AExampleGridPawn()
	:Super()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	UStaticMesh *Mesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cone.Cone'")).Object;
	StaticMesh->SetStaticMesh(Mesh);
	StaticMesh->SetRelativeLocation(FVector(0, 0, 50));
	StaticMesh->SetupAttachment(Scene);

	/* Show the arrow in game so we can which way the cone is facing */
	Arrow->SetHiddenInGame(false);
	Arrow->SetRelativeLocation(FVector(0, 0, 50));
}

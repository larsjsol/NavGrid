// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "GridPawn.h"
#include "GridMovementComponent.h"


// Sets default values
AGridPawn::AGridPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MovementComponent = CreateDefaultSubobject<UGridMovementComponent>("MovementComponent");
}

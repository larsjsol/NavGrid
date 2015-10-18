// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "GridPawn.generated.h"

class UGridMovementComponent;

UCLASS()
class BOARDGAME_API AGridPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGridPawn();

	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UGridMovementComponent *MovementComponent = NULL;
};

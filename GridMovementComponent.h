// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/MovementComponent.h"
#include "GridMovementComponent.generated.h"

class ANavGrid;

/**
 * 
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class BOARDGAME_API UGridMovementComponent : public UMovementComponent
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	/* bound to the first NavGrid found in the level */
	UPROPERTY(BlueprintReadOnly, Category = "Movement") ANavGrid *Grid = NULL;
	/* How far (in tile cost) the actor can move in one go */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement") float MovementRange = 4;
};

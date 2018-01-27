// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NavGridGameState.generated.h"

/**
 * 
 */
UCLASS()
class NAVGRID_API ANavGridGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	virtual void HandleBeginPlay() override;

	UPROPERTY(BlueprintReadWrite, VisibleAnyWhere, Category = "NavGrid")
	ANavGrid *Grid;

	UPROPERTY(BlueprintReadWrite, VisibleAnyWhere, Category = "NavGrid")
	ATurnManager *TurnManager;
};

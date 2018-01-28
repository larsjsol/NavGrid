// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NavGridGameState.generated.h"

class ATurnManager;
class ANavGrid;

/**
 * 
 */
UCLASS()
class NAVGRID_API ANavGridGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	virtual void HandleBeginPlay() override;

	/* spawn the default turn manager object, override this if you need to modify it */
	virtual ATurnManager *SpawnTurnManager();
	/* spawn the default navgrid object, override this if you need to modify it */
	virtual ANavGrid *SpawnNavGrid();

	UPROPERTY(BlueprintReadWrite, VisibleAnyWhere, Category = "NavGrid")
	ANavGrid *Grid;

	UPROPERTY(BlueprintReadWrite, VisibleAnyWhere, Category = "NavGrid")
	ATurnManager *TurnManager;
};

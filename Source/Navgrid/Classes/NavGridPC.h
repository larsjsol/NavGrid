// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "NavGridPC.generated.h"

class ANavGrid;
class ATurnManager;
class UTurnComponent;
class AGridPawn;
class UNavTileComponent;

/**
 * An example PlayerController that lets you move a single GridPawn by
 * clicking on a NavGrid
 */
UCLASS()
class NAVGRID_API ANavGridPC : public APlayerController
{
	GENERATED_BODY()
public:
	ANavGridPC(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnTileClicked(const UNavTileComponent *Tile);
	UFUNCTION()
	virtual void OnTileCursorOver(const UNavTileComponent *Tile);
	UFUNCTION()
	virtual void OnEndTileCursorOver(const UNavTileComponent *Tile);

	/* Called when a new round starts*/
	UFUNCTION()
	virtual void OnRoundStart() {};
	/* Called when a new turn starts*/
	UFUNCTION()
	virtual void OnTurnStart(UTurnComponent *Component);
	/* Called when a turn ends */
	UFUNCTION()
	virtual void OnTurnEnd(UTurnComponent *Component);
	/* Called at the start of each team turn */
	UFUNCTION()
	virtual void OnTeamTurnStart(const FGenericTeamId &TeamId) {}

	virtual void SetTurnManager(ATurnManager * InTurnManager);
	virtual void SetGrid(ANavGrid * InGrid);

	/* The pawn we're currently controlling */
	UPROPERTY(BlueprintReadWrite)
	AGridPawn *GridPawn;
	/* The NavGrid in the current game */
	UPROPERTY(BlueprintReadWrite)
	ANavGrid *Grid;
	/* The TurnManager in the current game */
	UPROPERTY(BlueprintReadWrite)
	ATurnManager *TurnManager;
};

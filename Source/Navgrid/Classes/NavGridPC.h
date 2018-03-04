// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
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

	/*
	Called when a tile is clicked

	- Highlight all tiles in range when we click on the character tile
	- Clear highlights when any other tile is clicked
	*/
	virtual void OnTileClicked(const UNavTileComponent &Tile);
	virtual void OnTileCursorOver(const UNavTileComponent &Tile);
	virtual void OnEndTileCursorOver(const UNavTileComponent &Tile);

	/* Called when a new turn starts*/
	UFUNCTION()
	virtual void OnTurnStart(UTurnComponent *Component);
	/* Called when a turn ends */
	UFUNCTION()
	virtual void OnTurnEnd(UTurnComponent *Component);

	void SetTurnManager(ATurnManager * InTurnManager);
	void SetGrid(ANavGrid * InGrid);

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

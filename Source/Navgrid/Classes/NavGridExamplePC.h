// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "NavGridExamplePC.generated.h"

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
class NAVGRID_API ANavGridExamplePC : public APlayerController
{
	GENERATED_BODY()
public:
	ANavGridExamplePC(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform &Transform) override;

	/*
	Called when a tile is clicked

	- Highlight all tiles in range when we click on the character tile
	- Clear highlights when any other tile is clicked
	*/
	void OnTileClicked(const UNavTileComponent &Tile);
	void OnTileCursorOver(const UNavTileComponent &Tile);
	void OnEndTileCursorOver(const UNavTileComponent &Tile);

	/* Called when a new turn starts*/
	void OnTurnStart(UTurnComponent *Component);
	/* Called when the current pawn is done moving*/
	void OnMovementEnd();
	
	/* The pawn we're currently controlling */
	AGridPawn *Pawn = NULL;
	ANavGrid *Grid = NULL;
	UPROPERTY() ATurnManager *TurnManager = NULL;
};

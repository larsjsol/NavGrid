// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "NavGridExamplePC.generated.h"

class ATile;
class ANavGrid;
class AGridPawn;
class UGridMovementComponent;

/**
 * An example PlayerController that lets you move a single GridPawn by
 * clicking on a NavGrid
 */
UCLASS()
class ANavGridExamplePC : public APlayerController
{
	GENERATED_BODY()
public:
	ANavGridExamplePC(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;

	/*
	Called when a tile is clicked

	- Highlight all tiles in range when we click on the character tile
	- Clear highlights when any other tile is clicked
	*/
	void OnTileClicked(const ATile &Tile);
	void OnTileCursorOver(const ATile &Tile);
	void OnEndTileCursorOver(const ATile &Tile);

	ANavGrid *Grid = NULL;
	AGridPawn *Pawn = NULL;
	UGridMovementComponent *MovementComponent = NULL;
};

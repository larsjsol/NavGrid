// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "NavGridExamplePC.generated.h"

class ATile;
class ANavGrid;

/**
 * 
 */
UCLASS()
class BOARDGAME_API ANavGridExamplePC : public APlayerController
{
	GENERATED_BODY()
public:
	ANavGridExamplePC(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;

	/* Handle mouse events*/
	void OnTileClicked(const ATile &Tile);
	void OnTileCursorOver(const ATile &Tile);

	ANavGrid *Grid = NULL;
};

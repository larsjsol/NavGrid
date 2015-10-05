// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "NavGridExamplePC.h"

#include "NavGrid.h"


ANavGridExamplePC::ANavGridExamplePC(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bShowMouseCursor = true;
	/* Enable mouse events */
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ANavGridExamplePC::BeginPlay()
{
	/* Grab a reference to the NavGrid and register handlers for mouse events */
	TActorIterator<ANavGrid> NavGridItr(GetWorld());
	ANavGrid *Grid = *NavGridItr;
	if (Grid)
	{
		Grid->OnTileClicked().AddUObject(this, &ANavGridExamplePC::OnTileClicked);
		Grid->OnTileCursorOver().AddUObject(this, &ANavGridExamplePC::OnTileCursorOver);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("Unable to get reference to Navgrid. Have you forgotten to place it in the level?"));
	}
}

void ANavGridExamplePC::OnTileClicked(const ATile &Tile)
{
	// Do something cool
}

void ANavGridExamplePC::OnTileCursorOver(const ATile &Tile)
{
	// Do something cool
}

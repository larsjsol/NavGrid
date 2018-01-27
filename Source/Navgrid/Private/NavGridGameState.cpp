// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

void ANavGridGameState::HandleBeginPlay()
{
	// if a navgrid exists in the game world, grab it
	TActorIterator<ANavGrid> GridItr(GetWorld());
	if (GridItr)
	{
		Grid = *GridItr;
	}
	else
	{
		// spawn grid if it does not already exist
		Grid = GetWorld()->SpawnActor<ANavGrid>();
		Grid->SetOwner(this);
		Grid->EnableVirtualTiles = true;
	}
	// make sure that every tile belongs to a grid
	TArray<UNavTileComponent *> AllTiles;
	Grid->GetEveryTile(AllTiles, GetWorld());
	for (UNavTileComponent *Tile : AllTiles)
	{
		if (!Tile->GetGrid())
		{
			Tile->SetGrid(Grid);
		}
	}

	// Spawn and configure Turn Manager
	TurnManager = GetWorld()->SpawnActor<ATurnManager>();
	TurnManager->SetOwner(this);

	/* Call parent */
	Super::HandleBeginPlay();

	/* start the first round */
	TurnManager->StartFirstRound();
}


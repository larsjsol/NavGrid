// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

ANavGrid* ANavGridGameState::GetNavGrid()
{
	if (!IsValid(Grid))
	{
		// if a navgrid exists in the game world, grab it
		TActorIterator<ANavGrid> GridItr(GetWorld());
		if (GridItr)
		{
			Grid = *GridItr;
		}
		else
		{
			Grid = SpawnNavGrid();
		}

		// make sure that every tile belongs to a grid
		TArray<UNavTileComponent*> AllTiles;
		Grid->GetEveryTile(AllTiles, GetWorld());
		for (UNavTileComponent* Tile : AllTiles)
		{
			if (!Tile->GetGrid())
			{
				Tile->SetGrid(Grid);
			}
		}
	}
	return Grid;
}

ATurnManager* ANavGridGameState::GetTurnManager()
{
	if (!IsValid(TurnManager))
	{
		TurnManager = SpawnTurnManager();
	}
	return TurnManager;
}

ATurnManager * ANavGridGameState::SpawnTurnManager()
{
	ATurnManager *Manager = GetWorld()->SpawnActor<ATurnManager>();
	Manager->SetOwner(this);
	return Manager;
}

ANavGrid * ANavGridGameState::SpawnNavGrid()
{
	ANavGrid *NewGrid = GetWorld()->SpawnActor<ANavGrid>();
	NewGrid->SetOwner(this);
	return NewGrid;
}

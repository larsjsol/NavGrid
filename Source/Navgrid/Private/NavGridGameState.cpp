// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

void ANavGridGameState::HandleBeginPlay()
{
	//spawn turn manager
	TurnManager = SpawnTurnManager();
	// register every pawn in the world
	for (TActorIterator<AGridPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		TurnManager->Register(ActorItr->TurnComponent);
	}

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
	TArray<UNavTileComponent *> AllTiles;
	Grid->GetEveryTile(AllTiles, GetWorld());
	for (UNavTileComponent *Tile : AllTiles)
	{
		if (!Tile->GetGrid())
		{
			Tile->SetGrid(Grid);
		}
	}

	// with the essentials in place, continue with begin play
	Super::HandleBeginPlay();
}

ATurnManager * ANavGridGameState::SpawnTurnManager()
{
	ATurnManager *Manager = GetWorld()->SpawnActor<ATurnManager>();
	Manager->SetOwner(this);
	return Manager;
}

ANavGrid * ANavGridGameState::SpawnNavGrid()
{
	ANavGrid *Grid = GetWorld()->SpawnActor<ANavGrid>();
	Grid->SetOwner(this);
	return Grid;
}

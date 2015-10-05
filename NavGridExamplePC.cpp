// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "NavGridExamplePC.h"

#include "NavGrid.h"
#include "GridMovementComponent.h"


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
	Grid = *NavGridItr;
	if (Grid)
	{
		Grid->OnTileClicked().AddUObject(this, &ANavGridExamplePC::OnTileClicked);
		Grid->OnTileCursorOver().AddUObject(this, &ANavGridExamplePC::OnTileCursorOver);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("Unable to get reference to Navgrid. Have you forgotten to place it in the level?"));
	}

	/* Grab the first character we find in the level */
	TActorIterator<ACharacter> CharItr(GetWorld());
	Character = *CharItr;
	if (!Character) { UE_LOG(NavGrid, Error, TEXT("No character found. A CharacterBP with a GridMovementComponent should be placed in the level.")); }
}

void ANavGridExamplePC::OnTileClicked(const ATile &Tile)
{
	/* 
	Highlight all tiles in range when we click on the character tile
	Clear highlights when any other tile is clicked	
	*/


	/* Get the tile the character is standing on and its movementcomponent*/
	ATile *CharacterLocation = Grid->GetTile(Character->GetActorLocation());
	UGridMovementComponent *MovementComponent = Character->FindComponentByClass<UGridMovementComponent>();

	if (CharacterLocation && MovementComponent)
	{
		if (CharacterLocation == &Tile)
		{
			/* find tiles in movement range and highlight them */
			TArray<ATile *> Tiles;
			Grid->TilesInRange(CharacterLocation, Tiles, MovementComponent->MovementRange);
			for (ATile *T : Tiles)
			{
				T->MovableHighlight->SetVisibility(true);
			}
		}
		else
		{
			/* hide the movable highlight on the entire grid */
			for (ATile *T : Grid->Tiles)
			{
				T->MovableHighlight->SetVisibility(false);
			}
		}
	}
}

void ANavGridExamplePC::OnTileCursorOver(const ATile &Tile)
{
	// Do something cool
}

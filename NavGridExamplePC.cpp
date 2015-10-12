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
		Grid->OnEndTileCursorOver().AddUObject(this, &ANavGridExamplePC::OnEndTileCursorOver);
	}
	else
	{
		UE_LOG(NavGrid, Fatal, TEXT("Unable to get reference to Navgrid. Have you forgotten to place it in the level?"));
	}

	/* Grab the first character we find in the level and find its gridmovementcomponent */
	TActorIterator<ACharacter> CharItr(GetWorld());
	Character = *CharItr;
	if (!Character)
	{
		UE_LOG(NavGrid, Fatal, TEXT("No character found. A CharacterBP with a GridMovementComponent should be placed in the level."));
	}
	MovementComponent = Character->FindComponentByClass<UGridMovementComponent>();
	if (!MovementComponent)
	{
			UE_LOG(NavGrid, Fatal, TEXT("No GridMovementComponent found. A CharacterBP with a GridMovementComponent should be placed in the level."));
	}

}

void ANavGridExamplePC::OnTileClicked(const ATile &Tile)
{
	/* Get the tile the character is standing on and its movementcomponent*/
	ATile *CharacterLocation = Grid->GetTile(Character->GetActorLocation());

	if (CharacterLocation && MovementComponent)
	{
		if (CharacterLocation == &Tile)
		{
			/* find tiles in movement range and highlight them */
			TArray<ATile *> Tiles;
			Grid->TilesInRange(CharacterLocation, Tiles, MovementComponent->MovementRange);
			for (ATile *T : Tiles) { T->MovableHighlight->SetVisibility(true); }
		}
		else
		{
			/* hide the movable highlight on the entire grid */
			TArray<ATile *> Tiles;
			Grid->GetTiles(Tiles);
			for (ATile *T : Tiles) { T->MovableHighlight->SetVisibility(false); }
		}
	}
}

void ANavGridExamplePC::OnTileCursorOver(const ATile &Tile)
{
	/* Try to reate path to the hovered tile and show it */
	if (MovementComponent->CreatePath(Tile))
	{
		MovementComponent->ShowPath();
	}
}

void ANavGridExamplePC::OnEndTileCursorOver(const ATile &Tile)
{
	/* Hide the previously shown path */
	MovementComponent->HidePath();
}

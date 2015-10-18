// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "NavGridExamplePC.h"

#include "NavGrid.h"
#include "GridPawn.h"
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

	/* Grab the first GridPawn we find in the level */
	TActorIterator<AGridPawn>PawnItr(GetWorld());
	Pawn = *PawnItr;
	if (!Pawn)
	{
		UE_LOG(NavGrid, Fatal, TEXT("No GridPawn found"));
	}
	MovementComponent = Pawn->FindComponentByClass<UGridMovementComponent>();
}

void ANavGridExamplePC::OnTileClicked(const ATile &Tile)
{
	/* Get the tile the pawn is standing on and its movementcomponent*/
	ATile *Location = Grid->GetTile(Pawn->GetActorLocation());

	if (Location && MovementComponent && Grid)
	{
		if (Location != &Tile && MovementComponent->Velocity.Size() == 0)
		{
			TArray<ATile *> InRange;
			Grid->TilesInRange(Location, InRange, MovementComponent->MovementRange);
			if (InRange.Contains(&Tile))
			{
				MovementComponent->MoveTo(Tile);
			}
		}
	}
}

void ANavGridExamplePC::OnTileCursorOver(const ATile &Tile)
{
	/* If the pawn is not moving, try to create a path to the hovered tile and show it */
	if (MovementComponent->Velocity.Size() == 0 && MovementComponent->CreatePath(Tile))
	{
		MovementComponent->ShowPath();
	}
}

void ANavGridExamplePC::OnEndTileCursorOver(const ATile &Tile)
{
	/* Hide the previously shown path */
	MovementComponent->HidePath();
}

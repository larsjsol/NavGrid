// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "NavGridExamplePC.h"
#include "GridPawn.h"
#include "GridMovementComponent.h"
#include "TurnComponent.h"
#include "TurnManager.h"


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

	/* Grab all gridpawns we find */
	TActorIterator<AGridPawn>PawnItr(GetWorld());
	while (PawnItr)
	{
		TurnManager->Register(PawnItr->TurnComponent);
		/* register handler for movement end*/
		PawnItr->MovementComponent->OnMovementEnd().AddUObject(this, &ANavGridExamplePC::OnMovementEnd);
	
		++PawnItr;
	}

	/* register handler for turn start*/
	TurnManager->OnTurnStart().AddUObject(this, &ANavGridExamplePC::OnTurnStart);
}

void ANavGridExamplePC::OnConstruction(const FTransform &Transform)
{
	FActorSpawnParameters sp;
	sp.bAllowDuringConstructionScript = true;
	TurnManager = GetWorld()->SpawnActor<ATurnManager>(sp);
	TurnManager->SetOwner(this);
}

void ANavGridExamplePC::OnTileClicked(const ATile &Tile)
{
	/* Try to move the current pawn to the clicked tile */
	if (Pawn)
	{
		UGridMovementComponent *MovementComponent = Pawn->MovementComponent;
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
}

void ANavGridExamplePC::OnTileCursorOver(const ATile &Tile)
{
	/* If the pawn is not moving, try to create a path to the hovered tile and show it */
	if (Pawn)
	{
		UGridMovementComponent *MovementComponent = Pawn->MovementComponent;
		if (MovementComponent->Velocity.Size() == 0 && MovementComponent->CreatePath(Tile))
		{
			MovementComponent->ShowPath();
		}
	}
}

void ANavGridExamplePC::OnEndTileCursorOver(const ATile &Tile)
{
	/* Hide the previously shown path */
	if (Pawn)
	{
		UGridMovementComponent *MovementComponent = Pawn->MovementComponent;
		MovementComponent->HidePath();
	}
}

void ANavGridExamplePC::OnTurnStart(const UTurnComponent &Component)
{
	Pawn = (AGridPawn *) Component.GetOwner();
}

void ANavGridExamplePC::OnMovementEnd()
{
	Pawn->MovementComponent->HidePath();
	TurnManager->EndTurn(Pawn->TurnComponent);
}

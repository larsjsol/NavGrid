// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

ANavGridPC::ANavGridPC(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bShowMouseCursor = true;
	/* Enable mouse events */
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	bEnableTouchEvents = true;
	bEnableTouchOverEvents = true;
}

void ANavGridPC::BeginPlay()
{
	// grab turn manager and grid from the game state
	auto *State = GetWorld()->GetGameState<ANavGridGameState>();
	SetTurnManager(State->GetTurnManager(FGenericTeamId()));
	SetGrid(State->Grid);
}

void ANavGridPC::OnTileClicked(const UNavTileComponent *Tile)
{
	/* Try to move the current pawn to the clicked tile */
	if (GridPawn && GridPawn->GetState() == EGridPawnState::Ready)
	{
		if (GridPawn->CanMoveTo(*Tile))
		{
			GridPawn->MoveTo(*Tile);
		}
	}
}

void ANavGridPC::OnTileCursorOver(const UNavTileComponent *Tile)
{
	/* If the pawn is not moving, try to create a path to the hovered tile and show it */
	if (GridPawn && GridPawn->GetState() == EGridPawnState::Ready)
	{
		Grid->Cursor->SetWorldLocation(Tile->GetPawnLocation() + FVector(0, 0, Grid->UIOffset));
		Grid->Cursor->SetVisibility(true);

		UGridMovementComponent *MovementComponent = GridPawn->MovementComponent;
		if (GridPawn->CanMoveTo(*Tile))
		{
			MovementComponent->CreatePath(*Tile);
			MovementComponent->ShowPath();
		}
	}
}

void ANavGridPC::OnEndTileCursorOver(const UNavTileComponent *Tile)
{
	Grid->Cursor->SetVisibility(false);
	/* Hide the previously shown path */
	if (GridPawn)
	{
		UGridMovementComponent *MovementComponent = GridPawn->MovementComponent;
		MovementComponent->HidePath();
	}
}

void ANavGridPC::OnTurnStart(UTurnComponent *Component)
{
	GridPawn = Cast<AGridPawn>(Component->GetOwner());
	check(GridPawn);
}

void ANavGridPC::OnTurnEnd(UTurnComponent * Component)
{
	GridPawn = NULL;
}

void ANavGridPC::SetTurnManager(ATurnManager * InTurnManager)
{
	check(InTurnManager);

	// unregister any delegates from the previous manager
	if (TurnManager)
	{
		TurnManager->OnTurnStart.RemoveDynamic(this, &ANavGridPC::OnTurnStart);
		TurnManager->OnTurnEnd.RemoveDynamic(this, &ANavGridPC::OnTurnEnd);
	}

	TurnManager = InTurnManager;
	TurnManager->OnTurnStart.AddDynamic(this, &ANavGridPC::OnTurnStart);
	TurnManager->OnTurnEnd.AddDynamic(this, &ANavGridPC::OnTurnEnd);
}

void ANavGridPC::SetGrid(ANavGrid * InGrid)
{
	check(InGrid);
	if (Grid)
	{
		Grid->OnTileClicked.RemoveDynamic(this, &ANavGridPC::OnTileClicked);
		Grid->OnTileCursorOver.RemoveDynamic(this, &ANavGridPC::OnTileCursorOver);
		Grid->OnEndTileCursorOver.RemoveDynamic(this, &ANavGridPC::OnEndTileCursorOver);
	}

	Grid = InGrid;
	Grid->OnTileClicked.AddDynamic(this, &ANavGridPC::OnTileClicked);
	Grid->OnTileCursorOver.AddDynamic(this, &ANavGridPC::OnTileCursorOver);
	Grid->OnEndTileCursorOver.AddDynamic(this, &ANavGridPC::OnEndTileCursorOver);
}

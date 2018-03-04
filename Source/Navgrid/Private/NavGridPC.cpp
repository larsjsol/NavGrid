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
	SetTurnManager(State->GetTurnManager());
	SetGrid(State->Grid);
}

void ANavGridPC::OnTileClicked(const UNavTileComponent &Tile)
{
	/* Try to move the current pawn to the clicked tile */
	if (GridPawn && !GridPawn->IsBusy())
	{
		if (GridPawn->CanMoveTo(Tile))
		{
			GridPawn->MoveTo(Tile);
		}
	}
}

void ANavGridPC::OnTileCursorOver(const UNavTileComponent &Tile)
{
	/* If the pawn is not moving, try to create a path to the hovered tile and show it */
	if (GridPawn && !GridPawn->IsBusy())
	{
		UGridMovementComponent *MovementComponent = GridPawn->MovementComponent;
		if (GridPawn->CanMoveTo(Tile))
		{
			MovementComponent->CreatePath((UNavTileComponent &)Tile);
			MovementComponent->ShowPath();
		}
	}
}

void ANavGridPC::OnEndTileCursorOver(const UNavTileComponent &Tile)
{
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
	TurnManager = InTurnManager;
	TurnManager->PlayerController = this;
	TurnManager->OnTurnStart.AddDynamic(this, &ANavGridPC::OnTurnStart);
	TurnManager->OnTurnEnd.AddDynamic(this, &ANavGridPC::OnTurnEnd);
}

void ANavGridPC::SetGrid(ANavGrid * InGrid)
{
	check(InGrid);
	Grid = InGrid;
	Grid->OnTileClicked().AddUObject(this, &ANavGridPC::OnTileClicked);
	Grid->OnTileCursorOver().AddUObject(this, &ANavGridPC::OnTileCursorOver);
	Grid->OnEndTileCursorOver().AddUObject(this, &ANavGridPC::OnEndTileCursorOver);
	DefaultClickTraceChannel = Grid->ECC_NavGridWalkable;
}

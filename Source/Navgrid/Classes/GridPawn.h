// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "GridPawn.generated.h"

class UGridMovementComponent;
class UTurnComponent;
class UCapsuleComponent;
class UArrowComponent;
class UNavTileComponent;
class ANavGrid;

/**
 * A pawn that can move around on a NavGrid.
 *
 * Currently simply a pawn with a GridMovementComponent.
 */
UCLASS()
class NAVGRID_API AGridPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGridPawn();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnyWhere, Category = "NavGrid")
	int32 TeamId;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent *Scene;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UGridMovementComponent *MovementComponent;
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTurnComponent *TurnComponent;
	/* Used to test if thre's room for pawn on a tile*/
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCapsuleComponent *CapsuleComponent;
	/* Shown when the pawn is selected/has its turn */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent *SelectedHighlight;
	/* An arrow pointing forward */
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UArrowComponent *Arrow;

	/* Should this pawn snap to grid at the start of each turn */
	UPROPERTY(EditAnyWhere, Category = "NavGrid")
	bool SnapToGrid = true;

	/* Callend on round start */
	virtual void OnRoundStart() {}
	/* Called on turn start */
	virtual void OnTurnStart();
	/* Called on turn end */
	virtual void OnTurnEnd();
	/* Called when done moving */
	virtual void OnMoveEnd();

	/* override this class and implement your own AI here. The default implementation just ends the turn */
	virtual void PlayAITurn();

	/* Is this pawn doing something that should not be interrupted by the player?
	*  Base implentation only checks if the pawn is moving
	*/
	virtual bool IsBusy();

	virtual bool CanMoveTo(const UNavTileComponent & Tile);
	virtual void MoveTo(const UNavTileComponent & Tile);

protected:
	UPROPERTY()
	ANavGrid *Grid;
};

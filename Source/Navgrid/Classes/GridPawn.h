// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "GridPawn.generated.h"

class UGridMovementComponent;
class UTurnComponent;
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

	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components") USceneComponent *Scene = NULL;

	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UGridMovementComponent *MovementComponent = NULL;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UTurnComponent *TurnComponent = NULL;
	/* Used to test if thre's room for pawn on a tile*/
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UCapsuleComponent *CapsuleComponent = NULL;
	/* Shown when the pawn is selected/has its turn */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *SelectedHighlight = NULL;
	/* An arrow pointing forward */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components") 
	UArrowComponent *Arrow = NULL;

	/* Should this pawn snap to grid when the game starts */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Default")
	bool SnapToGrid = true;

	/* Callend on round start */
	virtual void OnRoundStart();
	/* Called on turn start */
	virtual void OnTurnStart();
	/* Called on turn end */
	virtual void OnTurnEnd();
};

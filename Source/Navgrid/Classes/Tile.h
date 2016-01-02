// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Tile.generated.h"

class ANavGrid;
/**
 * A single 'space' in a navigation grid.
 *
 * It should usually not be neccacery to manually place these in the level.
 */
UCLASS()
class NAVGRID_API ATile : public AActor
{
	GENERATED_BODY()

public:
	ATile();

	virtual void BeginPlay() override;

	/* Cost of moving into this tile*/
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Tile") float Cost = 1;
	/* X Coordinate in NavGrid space */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tile") int32 X;
	/* Y Coordinate in NavGrid space */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Tile") int32 Y;
	
	/* Distance from starting point of path */
	float Distance;
	/* Previous tile in path */
	ATile *Backpointer = NULL;
	/* Is this node in the 'visited' set? - Helper var for pathfinding */
	bool Visited;
	/* Reset variables used in pathfinding */
	virtual void ResetPath();

	/* Scene component (root) */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	USceneComponent *SceneComponent = NULL;
	/* The tile mesh */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *Mesh = NULL;
	/* Cursor for highlighting the selected tile */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *SelectCursor = NULL;
	/* Cursor for highlighting the hovered tile */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *HoverCursor = NULL;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *MovableHighlight = NULL;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *DangerousHighlight = NULL;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *SpecialHighlight = NULL;

	/* The grid this tile is part of */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Tile") ANavGrid *Grid;

private:
	UFUNCTION() void Clicked();
	UFUNCTION() void CursorOver();
	UFUNCTION() void EndCursorOver();
};

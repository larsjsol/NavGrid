// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SceneComponent.h"
#include "NavTileComponent.generated.h"

class ANavGrid;

/**
* A single tile in a navigation grid
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class NAVGRID_API UNavTileComponent : public USceneComponent
{
	GENERATED_BODY()
public:
	UNavTileComponent(const FObjectInitializer &ObjectInitializer);
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "Default") ANavGrid *Grid;

// Pathing
	/* Cost of moving into this tile*/
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Pathfinding") float Cost = 1;
	/* World extent of this tile */
	UPROPERTY() UBoxComponent *Extent;
	/* Distance from starting point of path */
	float Distance;
	/* Previous tile in path */
	UNavTileComponent *Backpointer = NULL;
	/* Is this node in the 'visited' set? - Helper var for pathfinding */
	bool Visited;
	/* Reset variables used in pathfinding */
	virtual void ResetPath();

	/* 
		Nodes are considered neighbours if at least one 
		of their contact points are close to each other
	*/
	TArray<FVector> *GetContactPoints();
protected:
	TArray<FVector> ContactPoints;

public:
	/* List of neighbouring tiles */
	TArray<UNavTileComponent *> *GetNeighbours();
protected:
	TArray<UNavTileComponent *> Neighbours;
public:
	/* Is there anythoing blocking between this tile and the Target tile? Uses the capsule for collision testing */
	bool Obstructed(const UNavTileComponent &Tile, const UCapsuleComponent &CollisionCapsule);
	/* Return the neighbours that are not Obstructed() */
	void GetUnobstructedNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutNeighbours);

// Visualisation
public:
	/* Cursor for highlighting the hovered tile */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *HoverCursor;

// User interface
	UFUNCTION() void Clicked(UPrimitiveComponent* TouchedComponent);
	UFUNCTION() void CursorOver(UPrimitiveComponent* TouchedComponent);
	UFUNCTION() void EndCursorOver(UPrimitiveComponent* TouchedComponent);
};
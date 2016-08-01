// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/SplineComponent.h"
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
	UPROPERTY(EditAnywhere) UBoxComponent *Extent;
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
	virtual TArray<FVector> *GetContactPoints();
protected:
	TArray<FVector> ContactPoints;

public:
	/* List of neighbouring tiles */
	virtual TArray<UNavTileComponent *> *GetNeighbours();
protected:
	TArray<UNavTileComponent *> Neighbours;
public:
	/* is there anything blocking an actor from moving from FromPos to this tile? Uses the capsule for collision testing */
	virtual bool Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule);
	/* is there anything blocking an actor from moving between From and To? Uses the capsule for collision testing */
	bool static Obstructed(const FVector &From, const FVector &To, const UCapsuleComponent &CollisionCapsule);
	/* Return the neighbours that are not Obstructed() */
	virtual void GetUnobstructedNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutNeighbours);

	/* Placement for any pawn occupying this tile */
	UPROPERTY() USceneComponent *PawnLocationOffset;

// Visualisation
public:
	/* Cursor for highlighting the hovered tile */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *HoverCursor;

// User interface
	UFUNCTION() void Clicked(UPrimitiveComponent* TouchedComponent, FKey Key);
	UFUNCTION() void CursorOver(UPrimitiveComponent* TouchedComponent);
	UFUNCTION() void EndCursorOver(UPrimitiveComponent* TouchedComponent);

	/*
	* Add points for moving into this tile from FromPos
	* Return number of points added
	*/
	virtual int32 AddSplinePoints(const FVector &FromPos, USplineComponent &OutSpline);
	/* Return a suitable upvector for a splinemesh moving across this tile */
	virtual FVector GetSplineMeshUpVector();
};
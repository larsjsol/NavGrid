// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NavTileComponent.h"
#include "NavLadderComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class NAVGRID_API UNavLadderComponent : public UNavTileComponent
{
	GENERATED_BODY()
public:
	UNavLadderComponent(const FObjectInitializer &ObjectInitializer);

	virtual void UpdateBodySetup() override;

	virtual FVector GetPawnLocation() const override;
	virtual void GetUnobstructedNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutNeighbours) override;
	virtual bool Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule) const override;
	virtual bool Traversable(float MaxWalkAngle, const TSet<EGridMovementMode> &PawnMovementModes) const override;
	virtual void AddPathSegments(USplineComponent &OutSpline, TArray<FPathSegment> &OutPathSegments, bool EndTile) const override;

	/* Helpers for determining walkable paths through this tile */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere)
	USceneComponent *BottomPathPoint;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere)
	USceneComponent *TopPathPoint;
	UPROPERTY(BlueprintReadWrite)
	UArrowComponent *ArrowComponent;

	virtual FVector GetSplineMeshUpVector() override;
};
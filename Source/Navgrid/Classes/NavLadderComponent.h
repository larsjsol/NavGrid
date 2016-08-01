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
	TArray<FVector> *GetContactPoints() override;

	virtual void GetUnobstructedNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutNeighbours) override;
	virtual bool Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule) override;
	virtual int32 AddSplinePoints(const FVector &FromPos, USplineComponent &OutSpline) override;

	/* Helpers for determining walkable paths through this tile */
	UPROPERTY(EditAnywhere) USceneComponent *BottomPathPoint;
	UPROPERTY(EditAnywhere) USceneComponent *TopPathPoint;

	virtual FVector GetSplineMeshUpVector() override;
};
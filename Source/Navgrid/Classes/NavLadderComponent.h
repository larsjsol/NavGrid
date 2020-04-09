// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "NavTileComponent.h"
#include "NavLadderComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class NAVGRID_API UNavLadderComponent : public UNavTileComponent
{
	GENERATED_BODY()
public:
	UNavLadderComponent();

	virtual void SetGrid(ANavGrid *InGrid) override;
	virtual FVector GetPawnLocation() const override { return ToWorldSpace(FVector(TileSize / 4, 0, 25)); }
	virtual void GetNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutUnObstructed, TArray<UNavTileComponent *> &OutObstructed) override;
	virtual bool Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule) const override;
	virtual void AddPathSegments(USplineComponent &OutSpline, TArray<FPathSegment> &OutPathSegments, bool EndTile) const override;

	virtual FVector GetSplineMeshUpVector() override;
protected:
	// local copy of tile ANavGrid::TileSize, value is set in SetGrid()
	float TileSize;
	FVector GetTopPathPoint() const { return ToWorldSpace(FVector(TileSize / 4, 0, BoxExtent.Z - 25)); }
	FVector GetBottomPathPoint() const { return ToWorldSpace(FVector(TileSize / 4, 0, -100)); }
	FVector ToWorldSpace(const FVector &CompSpace) const { return GetComponentLocation() + GetComponentRotation().RotateVector(CompSpace); }
};
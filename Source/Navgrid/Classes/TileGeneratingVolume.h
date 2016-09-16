#pragma once

#include "TileGeneratingVolume.generated.h"

/**
* A volume for batch placing NavTiles
*/
UCLASS(Blueprintable)
class NAVGRID_API ATileGeneratingVolume : public AVolume
{
	GENERATED_BODY()

public:
	/*	Destroy existing tiles and generate a new set

		Autoresets
	*/
	UPROPERTY(EditAnyWhere, Category = "Tiles")
	bool RegenerateTiles;

	UPROPERTY(BlueprintReadOnly, Category = "Tiles")
	TArray<UNavTileComponent *> Tiles;

	UFUNCTION(BlueprintCallable, Category = "Tiles")
	void GenerateTiles();

	virtual void OnConstruction(const FTransform &Transform) override;

protected:
	bool TraceTileLocation(const FVector &TraceStart, const FVector &TraceEnd, FVector &OutTilePos);
};
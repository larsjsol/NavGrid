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

	/*	Maximim number of tiles that this volume will create

		If a Tile Generating Volume has too many tile components, the editor will
		freeze up when it is selected.
	*/
	UPROPERTY(EditAnyWhere, Category = "Tiles")
	int32 MaxNumberOfTiles = 25 * 25; //it's more satisfying to place square volumes

	UPROPERTY()
	TArray<UNavTileComponent *> Tiles;

	UFUNCTION(BlueprintCallable, Category = "Tiles")
	void GenerateTiles();

	UFUNCTION(BlueprintCallable, Category = "Tiles")
	void DestroyTiles();

	virtual void OnConstruction(const FTransform &Transform) override;
	virtual void Destroyed() override;
};
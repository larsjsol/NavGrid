// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "GameFramework/Actor.h"

#include "NavTileComponent.h"

#include "NavGrid.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(NavGrid, Log, All);

/**
 * A grid that pawns can move around on.
 *
 */
UCLASS()
class NAVGRID_API ANavGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANavGrid();

	virtual void OnConstruction(const FTransform &Transform) override;

	/* Scene Component (root) */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Components") USceneComponent *SceneComponent = NULL;

	/* Get tile from world location, may return NULL */
	virtual UNavTileComponent *GetTile(const FVector &WorldLocation);

	void TileClicked(UNavTileComponent &Tile);
	void TileCursorOver(UNavTileComponent &Tile);
	void EndTileCursorOver(UNavTileComponent &Tile);

	/* Find all tiles in range */
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	virtual void TilesInRange(UNavTileComponent * Tile, TArray<UNavTileComponent*>& OutArray, float Range, bool DoCollisionTests, UCapsuleComponent * Capsule);

	//Event delegates
	DECLARE_EVENT_OneParam(ANavGrid, FOnTileClicked, const UNavTileComponent& );
	DECLARE_EVENT_OneParam(ANavGrid, FOnTileCursorOver, const UNavTileComponent&);
	DECLARE_EVENT_OneParam(ANavGrid, FOnEndTileCursorOver, const UNavTileComponent&);

	/* Triggered by mouse clicks on tiles*/
	FOnTileClicked& OnTileClicked() { return OnTileClickedEvent; }	
	/* Triggered when the cursor enters a tile */
	FOnTileCursorOver& OnTileCursorOver() { return OnTileCursorOverEvent; }
	/* Triggered when the cursor leaves a tile */
	FOnEndTileCursorOver& OnEndTileCursorOver() { return OnEndTileCursorOverEvent; }

private:
	FOnTileClicked OnTileClickedEvent;
	FOnTileCursorOver OnTileCursorOverEvent;
	FOnEndTileCursorOver OnEndTileCursorOverEvent;

public:
	/** return every tile in the supplied world
	* You should probably store the result in a UPROPRTY to avoid the
	* garbage collector
	*/
	static void GetEveryTile(TArray<UNavTileComponent* > &OutTiles, UWorld *World);

	static ECollisionChannel ECC_Walkable;
};

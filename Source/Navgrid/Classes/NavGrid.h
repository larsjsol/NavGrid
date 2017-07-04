// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "GameFramework/Actor.h"

#include "NavTileComponent.h"
#include "GridMovementComponent.h"

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
	ANavGrid();

	/* Collision channel used when tracing for tiles */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "NavGrid")
	TEnumAsByte<ECollisionChannel> ECC_NavGridWalkable = ECollisionChannel::ECC_GameTraceChannel1;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "NavGrid")
	float TileSize = 199;
	/* Distance between the centers of two adjacent tiles */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "NavGrid")
	float TileSpacing = 200;
	/* Should virtual tiles be placed on empty areas */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "NavGrid")
	bool EnableVirtualTiles = false;

	/* Scene Component (root) */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Components")
	USceneComponent *SceneComponent = NULL;

	/* Cursor for highlighting tiles */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *Cursor;

	/* Number of tiles that exist in the current level */
	UPROPERTY(VisibleAnywhere, Category = "NavGrid")
	int32 NumPersistentTiles = 0;
	/* Current number of virtual tiles */
	UPROPERTY(VisibleAnywhere, Category = "NavGrid")
	int32 NumVirtualTiles = 0;

	static ANavGrid *GetNavGrid(UWorld *World);

	/* Get tile from world location, may return NULL */
	virtual UNavTileComponent *GetTile(const FVector &WorldLocation, bool FindFloor = true);
protected:
	UNavTileComponent *LineTraceTile(const FVector &Start, const FVector &End);
public:
	void TileClicked(UNavTileComponent &Tile);
	void TileCursorOver(UNavTileComponent &Tile);
	void EndTileCursorOver(UNavTileComponent &Tile);

	/* Find all tiles in range */
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	virtual void CalculateTilesInRange(AGridPawn *Pawn, bool DoCollisionTests);
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	void GetTilesInRange(TArray<UNavTileComponent *> &OutTiles);
protected:
	/* Contains tiles found in the last call to CalculateTilesInRange() */
	UPROPERTY(Transient)
	TArray<UNavTileComponent *> TilesInRange;
public:

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

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pathfinding")
	bool TraceTileLocation(const FVector & TraceStart, const FVector & TraceEnd, FVector & OutTilePos);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pathfinding")
	UNavTileComponent *PlaceTile(const FVector &Location, AActor *TileOwner = NULL);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pathfinding")
	UNavTileComponent *ConsiderPlaceTile(const FVector &TraceStart, const FVector &TraceEnd, AActor *TileOwner = NULL);

protected:
	UPROPERTY(Transient)
	TArray<UNavTileComponent *> VirtualTiles;
public:
	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	void GenerateVirtualTiles(const AGridPawn *Pawn);
	void DestroyVirtualTiles();

	virtual void Destroyed() override;
private:
	FOnTileClicked OnTileClickedEvent;
	FOnTileCursorOver OnTileCursorOverEvent;
	FOnEndTileCursorOver OnEndTileCursorOverEvent;

public:
	/** return every tile in the supplied world */
	static void GetEveryTile(TArray<UNavTileComponent* > &OutTiles, UWorld *World);
};

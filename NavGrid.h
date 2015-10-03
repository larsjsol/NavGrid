// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"

#include "Tile.h"

#include "NavGrid.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(NavGrid, Log, All);


UCLASS()
class BOARDGAME_API ANavGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANavGrid();
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;
	virtual void OnConstruction(const FTransform &Transform) override;
	virtual void Destroyed() override;

	/* Width in tiles */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Tiles") int32 XSize = 3;
	/* Height in tiles */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Tiles") int32 YSize = 3;

	/* Convert NavGrid space coords to offset from actorlocation */
	virtual FVector LocalPosition(int32 X, int32 Y);

	/* Default tile mesh */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Appearance") UStaticMesh *DefaultMesh = NULL;
	/* Shown when the mouse cursor hovers a tile */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Appearance") UStaticMesh *HoverCursor = NULL;
	/* Shown when a tile is selected */
	UPROPERTY(EditAnyWhere, BlueprintReadOnly, Category = "Appearance") UStaticMesh *SelectCursor = NULL;

	/* Sceene Coponent (root) */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Components") USceneComponent *SceneComponent = NULL;

	/* Get tile from coords, may return NULL */
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Tiles")
	virtual ATile *GetTile(int32 X, int32 Y);

	UFUNCTION() virtual void TileClicked(ATile *Tile);
	UFUNCTION() virtual void TileCursorOver(ATile *Tile);

private:
	/* Holds the actual tiles */
	UPROPERTY() TArray<ATile *> Tiles;
	/* Create or destroy tiles so we have XSize * YSize */
	virtual void AdjustNumberOfTiles();
};

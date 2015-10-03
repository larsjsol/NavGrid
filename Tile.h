// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Tile.generated.h"

/**
 * 
 */
UCLASS()
class BOARDGAME_API ATile : public AActor
{
	GENERATED_BODY()
	
public:
	ATile();

	/* Cost of moving into this tile*/
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Tile") int32 Cost = 1;
	/* X Coordinate in NavGrid space */
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Tile") int32 X;
	/* Y Coordinate in NavGrid space */
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Tile") int32 Y;

	/* Scene component (root) */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	USceneComponent *SceneComponent = NULL;
	/* The tile mesh */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *Mesh = NULL;
	/* Cursor for highlighting the selected tile */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *SelectCursor = NULL;
	/* Cursor for highlighting the hovered tile */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *HoverCursor = NULL;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *MovableHighlight = NULL;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *DangerousHighlight = NULL;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")
	UStaticMeshComponent *SpecialHighlight = NULL;

	/* Draw a mesh hovering this tile */
	void Highlight(UStaticMesh *HighlightMesh);

private:
	UFUNCTION() void Clicked();
	UFUNCTION() void CursorOver();
};

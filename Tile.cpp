// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "Tile.h"
#include "NavGrid.h"

ATile::ATile()
	:Super()
{
	/* Set up components */
	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	Mesh->AttachParent = SceneComponent;
	HoverCursor = CreateDefaultSubobject<UStaticMeshComponent>("HoverCursor");
	HoverCursor->AttachParent = SceneComponent;
	SelectCursor = CreateDefaultSubobject<UStaticMeshComponent>("SelectCursor");
	SelectCursor->AttachParent = SceneComponent;

	// position and hide ui elements
	FTransform UIOffset;
	UIOffset.SetLocation(FVector(0, 0, 10));
	HoverCursor->AddLocalTransform(UIOffset);
	SelectCursor->AddLocalTransform(UIOffset);
	HoverCursor->SetVisibility(false);
	SelectCursor->SetVisibility(false);
	
	// bind click events
	OnClicked.AddDynamic(this, &ATile::Clicked);
	OnBeginCursorOver.AddDynamic(this, &ATile::CursorOver);
}

void ATile::Clicked()
{
	ANavGrid *Grid = CastChecked<ANavGrid>(GetOwner());
	if (Grid)
	{
		Grid->TileClicked(this);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("%s.Clicked(): Unable to find owning NavGrid"), *GetName());
	}
}

void ATile::CursorOver()
{
	ANavGrid *Grid = CastChecked<ANavGrid>(GetOwner());
	if (Grid)
	{
		Grid->TileCursorOver(this);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("%s.CursorOver(): Unable to find owning NavGrid"), *GetName());
	}
}

void ATile::Highlight(UStaticMesh *HighlightMesh)
{
	
}

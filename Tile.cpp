// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "Tile.h"
#include "NavGrid.h"

#include <limits>

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
	MovableHighlight = CreateDefaultSubobject<UStaticMeshComponent>("MovableHighlight");
	MovableHighlight->AttachParent = SceneComponent;
	DangerousHighlight = CreateDefaultSubobject<UStaticMeshComponent>("DangerousHighlight");
	DangerousHighlight->AttachParent = SceneComponent;
	SpecialHighlight = CreateDefaultSubobject<UStaticMeshComponent>("SpecialHighlight");
	SpecialHighlight->AttachParent = SceneComponent;
	BackpointerMesh = CreateDefaultSubobject<UStaticMeshComponent>("BackpointerArrow");
	BackpointerMesh->AttachParent = SceneComponent;

	// position and hide ui elements
	FTransform HighlightOffset;
	HighlightOffset.SetLocation(FVector(0, 0, 10));
	FTransform UIOffset;
	UIOffset.SetLocation(FVector(0, 0, 15));

	HoverCursor->AddLocalTransform(UIOffset);
	SelectCursor->AddLocalTransform(UIOffset);
	MovableHighlight->AddLocalTransform(HighlightOffset);
	DangerousHighlight->AddLocalTransform(HighlightOffset);
	SpecialHighlight->AddLocalTransform(HighlightOffset);
	BackpointerMesh->AddLocalTransform(UIOffset);
	HoverCursor->SetVisibility(false);
	SelectCursor->SetVisibility(false);
	MovableHighlight->SetVisibility(false);
	DangerousHighlight->SetVisibility(false);
	SpecialHighlight->SetVisibility(false);
	BackpointerMesh->SetVisibility(false);

	// bind click events
	OnClicked.AddDynamic(this, &ATile::Clicked);
	OnBeginCursorOver.AddDynamic(this, &ATile::CursorOver);
	OnEndCursorOver.AddDynamic(this, &ATile::EndCursorOver);
}

void ATile::BeginPlay()
{
	Grid = CastChecked<ANavGrid>(GetOwner());
	if (Grid)
	{
		Grid->TileClicked(*this);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("%s.BeginPlay: Unable to find owning NavGrid"), *GetName());
	}
}

void ATile::ResetPath()
{
	Distance = std::numeric_limits<float>::infinity();
	Backpointer = NULL;
	Visited = false;
}

void ATile::SetBackpointerVisibility(bool Visible)
{
	/* rotate the arrow mesh */
	if (Backpointer && Visible)
	{
		FVector Direction = Backpointer->GetActorLocation() - GetActorLocation();
		Direction.Z = 0; // ignore height differences 
		BackpointerMesh->SetRelativeRotation(Direction.Rotation().Quaternion());
	}
	BackpointerMesh->SetVisibility(Visible);
}

void ATile::Clicked()
{
	if (Grid) { Grid->TileClicked(*this); }
}

void ATile::CursorOver()
{
	if (Grid)
	{
		// leave the visibility alone if are not responsible for it
		if (Grid->ShowHoverCursor) { HoverCursor->SetVisibility(true); } 
		Grid->TileCursorOver(*this);
	}
}

void ATile::EndCursorOver()
{
	if (Grid)
	{
		// leave the visibility alone if are not responsible for it
		if (Grid->ShowHoverCursor) { HoverCursor->SetVisibility(false); }
		Grid->EndTileCursorOver(*this);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "NavGrid.h"
#include "Tile.h"

#include "Editor.h"

DEFINE_LOG_CATEGORY(NavGrid);

// Sets default values
ANavGrid::ANavGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;

	AssignDefaultAssets();
}

// Called when the game starts or when spawned
void ANavGrid::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANavGrid::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void ANavGrid::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	// adjust tile count
	AdjustNumberOfTiles();

	// apply the default mesh if it is set

	for (ATile *Tile : Tiles)
	{
		if (Tile)
		{
			if (DefaultTileMesh) { Tile->Mesh->SetStaticMesh(DefaultTileMesh); }
			if (DefaultHoverCursor) { Tile->HoverCursor->SetStaticMesh(DefaultHoverCursor); }
			if (DefaultSelectCursor) { Tile->SelectCursor->SetStaticMesh(DefaultSelectCursor); }
		}
	}
}

void ANavGrid::Destroyed()
{
	Super::Destroyed();

	for (ATile *Tile : Tiles)
	{
		Tile->Destroy();
	}
	Tiles.Empty();
}


FVector ANavGrid::LocalPosition(int32 X, int32 Y)
{
	return FVector(X * 200.0, Y * 200.0, 0);
}

ATile *ANavGrid::GetTile(int32 X, int32 Y)
{
	if (X < XSize && Y < YSize && X >= 0 && Y >= 0)
	{
		return Tiles[Y * XSize + X];
	}
	else
	{
		return NULL;
	}
}

void ANavGrid::AdjustNumberOfTiles()
{
	int32 PrevSize = Tiles.Num();
	int32 NewSize = XSize * YSize;


	// rebuild all tiles if the dimensions have changed
	if (NewSize != PrevSize)
	{
		for (ATile *Tile : Tiles)
		{
			if (Tile)
			{
				Tile->Destroy();
			}
		}

		Tiles.SetNum(NewSize, true);

		for (int32 Idx = 0; Idx < NewSize; Idx++)
		{
			// Spawn a new tile
			FActorSpawnParameters fp;
			fp.bAllowDuringConstructionScript = true;
			UPROPERTY() ATile *Tile = GetWorld()->SpawnActor<ATile>(ATile::StaticClass(), fp);
			Tile->SetOwner(this);
			GEditor->ParentActors(this, Tile, NAME_None);

			Tiles[Idx] = Tile;

			//set position
			Tile->X = Idx % XSize;
			Tile->Y = Idx / XSize;

			FVector Position = GetActorLocation() + LocalPosition(Tile->X, Tile->Y);
			Tile->SetActorLocation(Position);
		}
	}
}

void ANavGrid::TileClicked(ATile *Tile)
{
	if (SelectedTile)
	{
		SelectedTile->SelectCursor->SetVisibility(false);
	}
	Tile->SelectCursor->SetVisibility(true);
	SelectedTile = Tile;
}

void ANavGrid::TileCursorOver(ATile *Tile)
{
	if (HoveredTile)
	{
		HoveredTile->HoverCursor->SetVisibility(false);
	}
	Tile->HoverCursor->SetVisibility(true);
	HoveredTile = Tile;
}

void ANavGrid::AssignDefaultAssets()
{
	// FIXME: DRY this up

	if (!DefaultTileMesh)
	{
		auto TM = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Tile_Square.SM_Tile_Square'"));
		if (TM.Succeeded())
		{
			DefaultTileMesh = TM.Object;
		}
	}
	if (!DefaultHoverCursor)
	{
		auto HC = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Frame_Hover.SM_Frame_Hover'"));
		if (HC.Succeeded())
		{
			DefaultHoverCursor = HC.Object;
		}
	}
	if (!DefaultSelectCursor)
	{
		auto SC = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Frame_Current_Unit.SM_Frame_Current_Unit'"));
		if (SC.Succeeded())
		{
			DefaultSelectCursor = SC.Object;
		}
	}
}
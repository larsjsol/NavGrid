// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "NavGrid.h"
#include "Tile.h"

#include "Editor.h"

#include <limits>

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
			if (DefaultMovableHighlight) { Tile->MovableHighlight->SetStaticMesh(DefaultMovableHighlight); }
			if (DefaultDangerousHighlight) { Tile->DangerousHighlight->SetStaticMesh(DefaultDangerousHighlight); }
			if (DefaultSpecialHighlight) { Tile->SpecialHighlight->SetStaticMesh(DefaultSpecialHighlight); }
			if (DefaultBackpointerArrow) { Tile->BackpointerMesh->SetStaticMesh(DefaultBackpointerArrow); }
		}
	}
}

void ANavGrid::Destroyed()
{
	Super::Destroyed();

	for (ATile *Tile : Tiles)
	{
		if (Tile && Tile->IsValidLowLevel()) { Tile->Destroy(); }
	}
	Tiles.Empty();
}


FVector ANavGrid::LocalPosition(int32 X, int32 Y)
{
	return FVector(X * TileWidth, Y * TileHeight, 0);
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

ATile *ANavGrid::GetTile(const FVector &WorldLocation)
{
	FVector LocalCoord = WorldLocation - GetActorLocation();
	return GetTile((int) (LocalCoord.X / TileWidth), (int) (LocalCoord.Y / TileHeight));
}

void ANavGrid::GetTiles(TArray<ATile *> &OutTiles)
{
	OutTiles.Empty();

	for (ATile *T : Tiles)
	{
		if (T) { OutTiles.Add(T); }
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

void ANavGrid::TileClicked(ATile &Tile)
{
	OnTileClickedEvent.Broadcast(Tile);
}

void ANavGrid::TileCursorOver(ATile &Tile)
{
	OnTileCursorOverEvent.Broadcast(Tile);
}

void ANavGrid::Neighbours(ATile *Tile, TArray<ATile*> &OutArray)
{
	OutArray.Empty();
	if (Tile)
	{
		for (int X = Tile->X - 1; X <= Tile->X + 1; X++)
		{
			for (int Y = Tile->Y - 1; Y <= Tile->Y + 1; Y++)
			{
				// skip the starting tile
				if (X != Tile->X || Y != Tile->Y)
				{
					ATile *N = GetTile(X, Y);
					if (N) { OutArray.Add(N); }
				}
			}
		}
	}
}

void ANavGrid::TilesInRange(ATile *Tile, TArray<ATile *> &OutArray, float Range)
{
	OutArray.Empty();
	for (ATile *T : Tiles)
	{
		if (T) { T->ResetPath(); }
	}

	ATile *Current = Tile;
	Current->Distance = 0;

	TArray<ATile *> NeighbouringTiles;
	Neighbours(Current, NeighbouringTiles);
	TArray<ATile *> TentativeSet(NeighbouringTiles);

	while (Current)
	{
		Neighbours(Current, NeighbouringTiles);
		for (ATile *N : NeighbouringTiles)
		{
			if (!N->Visited)
			{
				float TentativeDistance = N->Cost + Current->Distance;
				if (TentativeDistance <= N->Distance)
				{
					/*
						Prioritize straight paths by using the world distance as a tiebreaker
						when TentativeDistance is equal N->Dinstance
					*/
					float OldDistance = std::numeric_limits<float>::infinity();
					float NewDistance = 0;
					if (TentativeDistance == N->Distance)
					{
						NewDistance = (Current->GetActorLocation() - N->GetActorLocation()).Size();
						if (N->Backpointer)
						{
							OldDistance = (N->Backpointer->GetActorLocation() - N->GetActorLocation()).Size();
						}
					}

					if (NewDistance < OldDistance) // Always true if TentativeDistance < N->Distance
					{
						N->Distance = TentativeDistance;
						N->Backpointer = Current;

						if (TentativeDistance <= Range)
						{
							TentativeSet.AddUnique(N);
						}
					}
				}
			}
		}
		Current->Visited = true;
		TentativeSet.Remove(Current);
		if (Current != Tile) { OutArray.Add(Current); } // dont include the starting tile
		if (TentativeSet.Num())
		{
			Current = TentativeSet[0];
		}
		else
		{
			Current = NULL;
		}
	}
}

void ANavGrid::AssignDefaultAssets()
{
	SetSM(&DefaultTileMesh, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Tile_Square.SM_Tile_Square'"));
	SetSM(&DefaultHoverCursor, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Frame_Hover.SM_Frame_Hover'"));
	SetSM(&DefaultSelectCursor, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Frame_Current_Unit.SM_Frame_Current_Unit'"));
	SetSM(&DefaultMovableHighlight, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Tile_In_Move_Range_Square.SM_Tile_In_Move_Range_Square'"));
	SetSM(&DefaultDangerousHighlight, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Tile_In_Sight_Range_Square.SM_Tile_In_Sight_Range_Square'"));
	SetSM(&DefaultSpecialHighlight, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Tile_In_Long_Move_Range.SM_Tile_In_Long_Move_Range'"));
	SetSM(&DefaultBackpointerArrow, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Arrow.SM_Arrow'"));
}

void ANavGrid::SetSM(UStaticMesh **Meshptr, const TCHAR* AssetReference)
{	
	auto OF = ConstructorHelpers::FObjectFinder<UStaticMesh>(AssetReference);
	if (OF.Succeeded()) { *Meshptr = OF.Object;  }
}
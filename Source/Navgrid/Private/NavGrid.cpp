// Fill out your copyright notice in the Description page of Project Settings.

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

	SetSM(&DefaultTileMesh, TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Tile.NavGrid_Tile'"));
	SetSM(&DefaultHoverCursor, TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'"));
	SetSM(&DefaultSelectCursor, TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'"));
	SetSM(&DefaultMovableHighlight, TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Movable.NavGrid_Movable'"));
	SetSM(&DefaultDangerousHighlight, TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Dangerous.NavGrid_Dangerous'"));
	SetSM(&DefaultSpecialHighlight, TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Special.NavGrid_Special'"));
	//SetSM(&DefaultBackpointerArrow, TEXT("StaticMesh'/Game/NavGrid/Meshes/SM_Arrow.SM_Arrow'"));
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

	// Ensure that dimentions are > 0
	XSize = FPlatformMath::Max<int32>(1, XSize);
	YSize = FPlatformMath::Max<int32>(1, YSize);

	if (XSize != PrevXSize || YSize != PrevYSize)
	{
		// create or destroy tiles to the fill the grid
		AdjustNumberOfTiles();
		PrevXSize = XSize;
		PrevYSize = YSize;
	}

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
	// We're flooring the coord below so add half of a tile to ensure we get the right one 
	LocalCoord.X += TileWidth / 2; 
	LocalCoord.Y += TileHeight / 2;
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
	int32 NewSize = XSize * YSize;
	int32 OldSize = PrevXSize * PrevYSize;

	UPROPERTY() TArray<ATile *> NewTiles;
	NewTiles.SetNum(NewSize, true);
	
	// create the new grid
	for (int32 X = 0; X < XSize; X++)
	{
		for (int32 Y = 0; Y < YSize; Y++)
		{
			if (X < PrevXSize && Y < PrevYSize)
			{
				NewTiles[Y * XSize + X] = Tiles[Y * PrevXSize + X];
				Tiles[Y * PrevXSize + X] = NULL; // all remaining pointers are destroy()ed later 
			}
			else
			{
				FActorSpawnParameters fp;
				fp.bAllowDuringConstructionScript = true;
				UPROPERTY() ATile *Tile = GetWorld()->SpawnActor<ATile>(ATile::StaticClass(), fp);
				Tile->SetOwner(this);
				GEditor->ParentActors(this, Tile, NAME_None);

				//set position
				Tile->X = X;
				Tile->Y = Y;
				FVector Position = GetActorLocation() + LocalPosition(Tile->X, Tile->Y);
				Tile->SetActorLocation(Position);

				NewTiles[Y * XSize + X] = Tile;
			}
		}
	}
	// clean up the old grid, 
	for (ATile *Tile : Tiles)
	{
		if (Tile) Tile->Destroy();
	}

	Tiles = NewTiles;
}

void ANavGrid::TileClicked(ATile &Tile)
{
	OnTileClickedEvent.Broadcast(Tile);
}

void ANavGrid::TileCursorOver(ATile &Tile)
{
	OnTileCursorOverEvent.Broadcast(Tile);
}

void ANavGrid::EndTileCursorOver(ATile &Tile)
{
	OnEndTileCursorOverEvent.Broadcast(Tile);
}

void ANavGrid::Neighbours(ATile *Tile, TArray<ATile*> &OutArray, bool DoCollisionTests, UCapsuleComponent *Capsule)
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
					if (N)
					{ 
						if (!DoCollisionTests || !Obstructed(Tile, N, Capsule))
						{
							OutArray.Add(N);
						}
					}
				}
			}
		}
	}
}

void ANavGrid::TilesInRange(ATile *Tile, TArray<ATile *> &OutArray, float Range, bool DoCollisionTests, UCapsuleComponent *Capsule)
{
	OutArray.Empty();
	for (ATile *T : Tiles)
	{
		if (T) { T->ResetPath(); }
	}

	ATile *Current = Tile;
	Current->Distance = 0;

	TArray<ATile *> NeighbouringTiles;
	Neighbours(Current, NeighbouringTiles, DoCollisionTests, Capsule);
	TArray<ATile *> TentativeSet(NeighbouringTiles);

	while (Current)
	{
		Neighbours(Current, NeighbouringTiles, DoCollisionTests, Capsule);
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

bool ANavGrid::Obstructed(ATile *From, ATile *To, UCapsuleComponent *CollisionCapsule)
{
	if (!From || !To || !CollisionCapsule)
	{
		UE_LOG(NavGrid, Error, TEXT("ANavGrid::Obstructed() called with NULL argument: From:%p, To:%p, CollisionCapsule:%p"), (void *)From, (void *)To, (void *)CollisionCapsule);
		return false; //I guess...
	}
	else
	{
		FHitResult OutHit;
		FVector Start = From->GetActorLocation() + CollisionCapsule->RelativeLocation;
		FVector End = To->GetActorLocation() + CollisionCapsule->RelativeLocation;
		FQuat Rot = FQuat::Identity;
		FCollisionShape CollisionShape = CollisionCapsule->GetCollisionShape();
		FCollisionQueryParams CQP;
		CQP.AddIgnoredActor(CollisionCapsule->GetOwner());
		FCollisionResponseParams CRP;
		bool HitSomething =  GetWorld()->SweepSingleByChannel(OutHit, Start, End, Rot, 
                                                              ECollisionChannel::ECC_Pawn, CollisionShape, CQP, CRP);
		return HitSomething;
	}

}

void ANavGrid::SetSM(UStaticMesh **Meshptr, const TCHAR* AssetReference)
{	
	auto OF = ConstructorHelpers::FObjectFinder<UStaticMesh>(AssetReference);
	if (OF.Succeeded()) { *Meshptr = OF.Object; }
}

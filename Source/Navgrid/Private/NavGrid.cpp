// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"

#include "Editor.h"
#include <limits>

DEFINE_LOG_CATEGORY(NavGrid);

ECollisionChannel ANavGrid::ECC_Walkable = ECollisionChannel::ECC_GameTraceChannel1; //Ugh... Lets hope this isn't used anywhere else


// Sets default values
ANavGrid::ANavGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;
}

void ANavGrid::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

}


UNavTileComponent *ANavGrid::GetTile(const FVector &WorldLocation)
{
	FHitResult HitResult;
	FVector TraceEnd = WorldLocation - FVector(0, 0, 500);
	FCollisionQueryParams CQP;
	CQP.bTraceComplex = true;
	GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, TraceEnd, ANavGrid::ECC_Walkable, CQP);

	// check if component of any of its parents are is a tile
	UPrimitiveComponent *Comp = HitResult.GetComponent();
	UNavTileComponent *Tile = Cast<UNavTileComponent>(Comp);
	if (Tile) { return Tile; }
	if (Comp)
	{
		TArray<USceneComponent *> Components;
		Comp->GetParentComponents(Components);
		for (auto *C : Components)
		{
			UNavTileComponent *Tile = Cast<UNavTileComponent>(C);
			if (Tile) { return Tile; }
		}
	}
	return NULL;
}

void ANavGrid::TileClicked(UNavTileComponent &Tile)
{
	OnTileClickedEvent.Broadcast(Tile);
}

void ANavGrid::TileCursorOver(UNavTileComponent &Tile)
{
	OnTileCursorOverEvent.Broadcast(Tile);
}

void ANavGrid::EndTileCursorOver(UNavTileComponent &Tile)
{
	OnEndTileCursorOverEvent.Broadcast(Tile);
}

void ANavGrid::TilesInRange(UNavTileComponent * Tile, TArray<UNavTileComponent*>& OutArray, float Range, bool DoCollisionTests, UCapsuleComponent * Capsule)
{
	OutArray.Empty();
	if (!Capsule)
	{
		UE_LOG(NavGrid, Error, TEXT("TilesInRange called with NULL param: Tile: %p, Capsule: %p"), (void *)Tile, (void *)Capsule);
		return;
	}

	TArray<UNavTileComponent *> AllTiles;
	GetEveryTile(AllTiles, GetWorld());
	for (auto *T : AllTiles)
	{
		T->ResetPath();
	}

	UNavTileComponent *Current = Tile;
	Current->Distance = 0;
	TArray<UNavTileComponent *> NeighbouringTiles;
	Current->GetUnobstructedNeighbours(*Capsule, NeighbouringTiles);
	TArray<UNavTileComponent *> TentativeSet(NeighbouringTiles);

	while (Current)
	{
		Current->GetUnobstructedNeighbours(*Capsule, NeighbouringTiles);
		for (UNavTileComponent *N : NeighbouringTiles)
		{
			if (!N->Visited)
			{
				float TentativeDistance = N->Cost + Current->Distance;
				if (TentativeDistance <= N->Distance)
				{

					//	Prioritize straight paths by using the world distance as a tiebreaker
					//	when TentativeDistance is equal N->Dinstance
					float OldDistance = std::numeric_limits<float>::infinity();
					float NewDistance = 0;
					if (TentativeDistance == N->Distance)
					{
						NewDistance = (Current->GetComponentLocation() - N->GetComponentLocation()).Size();
						if (N->Backpointer)
						{
							OldDistance = (N->Backpointer->GetComponentLocation() - N->GetComponentLocation()).Size();
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


void ANavGrid::GetEveryTile(TArray<UNavTileComponent *> &OutTiles, UWorld * World)
{
	for (TObjectIterator<UNavTileComponent> Itr; Itr; ++Itr)
	{
		if (Itr->GetWorld() == World)
		{
			OutTiles.Add(*Itr);
		}
	}
}

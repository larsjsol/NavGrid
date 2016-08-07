// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "GridPawn.h"

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

UNavTileComponent *ANavGrid::GetTile(const FVector &WorldLocation, bool FindFloor/*= true*/)
{
	if (FindFloor)
	{
		return LineTraceTile(WorldLocation + FVector(0, 0, 50), WorldLocation - FVector(0, 0, 500));
	}
	else
	{
		/* Do a bunch of horizontal line traces and pick the closest tile component*/
		UNavTileComponent *Closest = NULL;
		static FVector EndPoints[8] = {
				FVector(0, 200, 0),
				FVector(200, 200, 0),
				FVector(200, 0, 0),
				FVector(200, -200, 0),
				FVector(0, -200, 0),
				FVector(-200, -200, 0),
				FVector(-200, 0, 0),
				FVector(-200, 200, 0)
		};
		for (FVector EndPoint : EndPoints)
		{
			UNavTileComponent *Tile = LineTraceTile(WorldLocation - EndPoint, WorldLocation + EndPoint);
			if (Tile)
			{
				if (!Closest)
				{
					Closest = Tile;
				}
				else if (FVector::Dist(Tile->GetComponentLocation(), WorldLocation) < FVector::Dist(Closest->GetComponentLocation(), WorldLocation))
				{
					Closest = Tile;
				}
			}
		}
		return Closest;
	}
}

UNavTileComponent *ANavGrid::LineTraceTile(const FVector &Start, const FVector &End)
{
	FHitResult HitResult;
	TArray<FHitResult> HitResults;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ANavGrid::ECC_Walkable);
	UPrimitiveComponent *Comp = HitResult.GetComponent();
	UNavTileComponent *Tile = Cast<UNavTileComponent>(Comp);
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

void ANavGrid::TilesInRange(UNavTileComponent * Tile, TArray<UNavTileComponent*>& OutArray, AGridPawn *Pawn, bool DoCollisionTests)
{
	OutArray.Empty();

	TArray<UNavTileComponent *> AllTiles;
	GetEveryTile(AllTiles, GetWorld());
	for (auto *T : AllTiles)
	{
		T->ResetPath();
	}

	UNavTileComponent *Current = Tile;
	Current->Distance = 0;
	TArray<UNavTileComponent *> NeighbouringTiles;
	Current->GetUnobstructedNeighbours(*Pawn->CapsuleComponent, NeighbouringTiles);
	TArray<UNavTileComponent *> TentativeSet(NeighbouringTiles);

	while (Current)
	{
		Current->GetUnobstructedNeighbours(*Pawn->CapsuleComponent, NeighbouringTiles);
		for (UNavTileComponent *N : NeighbouringTiles)
		{
			if (!N->Traversable(Pawn->MovementComponent->MaxWalkAngle, Pawn->MovementComponent->AvailableMovementModes))
			{
				continue;
			}

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

						if (TentativeDistance <= Pawn->MovementComponent->MovementRange)
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

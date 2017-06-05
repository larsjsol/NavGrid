// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

#include "Editor.h"
#include <limits>

DEFINE_LOG_CATEGORY(NavGrid);

// Sets default values
ANavGrid::ANavGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;

	Cursor = CreateDefaultSubobject<UStaticMeshComponent>(FName("Cursor"));
	Cursor->SetupAttachment(GetRootComponent());
	Cursor->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Cursor->ToggleVisibility(false);
	auto HCRef = TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'");
	auto HCFinder = ConstructorHelpers::FObjectFinder<UStaticMesh>(HCRef);
	if (HCFinder.Succeeded())
	{
		Cursor->SetStaticMesh(HCFinder.Object);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("Error loading %s"), HCRef);
	}
}

ANavGrid * ANavGrid::GetNavGrid(UWorld *World)
{
	if (World)
	{
		TActorIterator<ANavGrid> Itr(World, ANavGrid::StaticClass());
		if (Itr)
		{
			return *Itr;
		}
	}
	return NULL;
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

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_NavGridWalkable);
	UPrimitiveComponent *Comp = HitResult.GetComponent();
	return Cast<UNavTileComponent>(Comp);
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

void ANavGrid::CalculateTilesInRange(UNavTileComponent * Tile, AGridPawn *Pawn, bool DoCollisionTests)
{
	TilesInRange.Empty();

	if (!Tile || !Pawn)
	{
		UE_LOG(NavGrid, Error, TEXT("%s::CalculateTilesInRange(): Passed NULL-pointer"), *GetName());
		return;
	}

	TArray<UNavTileComponent *> AllTiles;
	GetEveryTile(AllTiles, GetWorld());
	for (auto *T : AllTiles)
	{
		T->ResetPath();
	}
	NumPersistentTiles = AllTiles.Num() - NumVirtualTiles;

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
		if (Current != Tile) { TilesInRange.Add(Current); } // dont include the starting tile
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

void ANavGrid::GetTilesInRange(TArray<UNavTileComponent*>& OutTiles)
{
	OutTiles = TilesInRange;
}

bool ANavGrid::TraceTileLocation(const FVector & TraceStart, const FVector & TraceEnd, FVector & OutTilePos)
{
	FCollisionQueryParams CQP;
	CQP.bFindInitialOverlaps = true;
	CQP.bTraceComplex = true;
	FHitResult HitResult;

	bool BlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Pawn, CQP);
	if (BlockingHit)
	{
		// dont place tiles on top of pawns or inside of things
		if (Cast<AGridPawn>(HitResult.Actor.Get()) || HitResult.bStartPenetrating)
		{
			return false;
		}
		// dont place a new tile if there already is one there
		if (GetTile(HitResult.ImpactPoint))
		{
			return false;
		}

		OutTilePos = HitResult.ImpactPoint;
		return true;
	}
	
	return false;
}

UNavTileComponent * ANavGrid::PlaceTile(const FVector & Location, AActor * TileOwner)
{
	if (!TileOwner)
	{
		TileOwner = this;
	}

	UPROPERTY() UNavTileComponent *TileComp = NewObject<UNavTileComponent>(this);
	TileComp->SetupAttachment(TileOwner->GetRootComponent());
	TileComp->SetWorldTransform(FTransform::Identity);
	TileComp->SetWorldLocation(Location);
	TileComp->SetGrid(this);
	TileComp->SetBoxExtent(FVector(TileSize / 2, TileSize / 2, 5));
	TileComp->RegisterComponentWithWorld(TileOwner->GetWorld());

	return TileComp;
}

UNavTileComponent * ANavGrid::ConsiderPlaceTile(const FVector &TraceStart, const FVector &TraceEnd, AActor * TileOwner /*= NULL*/)
{
	if (!TileOwner)
	{
		TileOwner = this;
	}

	FVector TileLocation;
	bool CanPlaceTile = TraceTileLocation(TraceStart, TraceEnd, TileLocation);
	if (CanPlaceTile)
	{
		return PlaceTile(TileLocation, TileOwner);
	}

	return NULL;
}

void ANavGrid::GenerateVirtualTiles(const AGridPawn *Pawn)
{
	// only keep a reasonable number
	if (VirtualTiles.Num() > 10000)
	{
		DestroyVirtualTiles();
	}

	// place a tile under the actor if it is not already one there
	if (!GetTile(Pawn->GetActorLocation()))
	{
		VirtualTiles.Add(PlaceTile(Pawn->GetActorLocation()));
	}

	FVector Min = Pawn->GetActorLocation() - FVector(Pawn->MovementComponent->MovementRange * TileSpacing);
	FVector Max = Pawn->GetActorLocation() + FVector(Pawn->MovementComponent->MovementRange * TileSpacing);
	for (float X = Min.X; X <= Max.X; X += TileSpacing)
	{
		for (float Y = Min.Y; Y <= Max.Y; Y += TileSpacing)
		{
			for (float Z = Max.Z; Z >= Min.Z; Z -= TileSpacing)
			{
				UPROPERTY() UNavTileComponent *TileComp = ConsiderPlaceTile(FVector(X, Y, Z + TileSpacing + 25), FVector(X, Y, Z - 25));
				if (TileComp)
				{
					VirtualTiles.Add(TileComp);
				}
			}
		}
	}
	NumVirtualTiles = VirtualTiles.Num();
}

void ANavGrid::DestroyVirtualTiles()
{
	for (UNavTileComponent *T : VirtualTiles)
	{
		if (T && T->IsValidLowLevel())
		{
			T->DestroyComponent();
		}
	}
	VirtualTiles.Empty();
}

void ANavGrid::Destroyed()
{
	Super::Destroyed();
	DestroyVirtualTiles();
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

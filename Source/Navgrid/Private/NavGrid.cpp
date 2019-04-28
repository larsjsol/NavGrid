// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"
#include "AssetRegistryModule.h"

#include <limits>

DEFINE_LOG_CATEGORY(NavGrid);

// Sets default values
ANavGrid::ANavGrid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	TileClass = UNavTileComponent::StaticClass();

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

	AddHighlightType("Movable", TEXT("Material'/NavGrid/Materials/Movable_Mat.Movable_Mat'"));
	AddHighlightType("Dangerous", TEXT("Material'/NavGrid/Materials/Dangerous_Mat.Dangerous_Mat'"));
	AddHighlightType("Special", TEXT("Material'/NavGrid/Materials/Special_Mat.Special_Mat'"));

	CurrentPawn = NULL;
	CurrentTile = NULL;
	bCurrentDoCollisionTests = false;
}

void ANavGrid::SetTileHighlight(UNavTileComponent & Tile, FName Type)
{
	Tile.SetHighlight(Type);
}

void ANavGrid::ClearTileHighlights()
{
	for (auto &H : TileHighlights)
	{
		H.Value->ClearInstances();
	}
}

void ANavGrid::AddHighlightType(const FName &Type, const TCHAR *FileName)
{
	TileHighLightPaths.Add(Type, FileName);
}

UInstancedStaticMeshComponent * ANavGrid::GetHighlightComponent(FName Type)
{
	/* build the instanced mesh component if we have not already done so */
	if (!TileHighlights.Contains(Type) && TileHighLightPaths.Contains(Type))
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		UStaticMesh *Mesh = LoadObject<UStaticMesh>(this, TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_TileHighlight.NavGrid_TileHighlight'"));
		check(Mesh);
		UMaterial *Material = LoadObject<UMaterial>(this, TileHighLightPaths[Type]);
		check(Material);
		auto *Comp = NewObject<UInstancedStaticMeshComponent>(this);
		Comp->SetupAttachment(GetRootComponent());
		Comp->SetStaticMesh(Mesh);
		Comp->SetMaterial(0, Material);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->RegisterComponent();
		TileHighlights.Add(Type, Comp);
	}
	/* we *should* now have the object we need*/
	if (TileHighlights.Contains(Type))
	{
		return TileHighlights[Type];
	}
	else
	{
		return NULL;
	}
}

ANavGrid *ANavGrid::GetNavGrid(AActor *ActorInWorld)
{
	return GetNavGrid(ActorInWorld->GetWorld());
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

UNavTileComponent *ANavGrid::GetTile(const FVector &WorldLocation, bool FindFloor/*= true*/, float UpwardTraceLength/* = 100*/, float DownwardTraceLength/* = 100*/)
{
	if (FindFloor)
	{
		return LineTraceTile(WorldLocation + FVector(0, 0, UpwardTraceLength), WorldLocation - FVector(0, 0, DownwardTraceLength));
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
	TArray<FHitResult> HitResults;
	FCollisionQueryParams CQP;
	CQP.TraceTag = "NavGridTile";

	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, ECC_NavGridWalkable, CQP);
	if (HitResults.Num())
	{
		UPrimitiveComponent *Comp = HitResults[0].GetComponent();
		return Cast<UNavTileComponent>(Comp);
	}
	else
	{
		return nullptr;
	}
}

void ANavGrid::TileClicked(const UNavTileComponent *Tile)
{
	OnTileClicked.Broadcast(Tile);
}

void ANavGrid::TileCursorOver(const UNavTileComponent *Tile)
{
	OnTileCursorOver.Broadcast(Tile);
}

void ANavGrid::EndTileCursorOver(const UNavTileComponent *Tile)
{
	OnEndTileCursorOver.Broadcast(Tile);
}

void ANavGrid::CalculateTilesInRange(AGridPawn *Pawn, bool DoCollisionTests)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ANavGrid_CalculateTilesInRange);

	check(Pawn);
	ClearTiles();
	UNavTileComponent *Current = Pawn->GetTile();
	/* if we're not on the grid, the number of tiles in range is zero */
	if (!Current)
	{
		return;
	}

	Current->Distance = 0;
	TArray<UNavTileComponent *> NeighbouringTiles;
	Current->GetUnobstructedNeighbours(*Pawn->MovementCollisionCapsule, NeighbouringTiles);
	TArray<UNavTileComponent *> TentativeSet(NeighbouringTiles);

	while (Current)
	{
		Current->GetUnobstructedNeighbours(*Pawn->MovementCollisionCapsule, NeighbouringTiles);
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
		if (Current != Pawn->GetTile()) { TilesInRange.Add(Current); } // dont include the starting tile
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

void ANavGrid::GetTilesInRange(AGridPawn *Pawn, bool DoCollisionTests, TArray<UNavTileComponent*>& OutTiles)
{
	if (Pawn != CurrentPawn || DoCollisionTests != bCurrentDoCollisionTests || Pawn->GetTile() != CurrentTile)
	{
		CurrentPawn = Pawn;
		bCurrentDoCollisionTests = DoCollisionTests;
		CurrentTile = Pawn->GetTile();
		CalculateTilesInRange(CurrentPawn, bCurrentDoCollisionTests);
	}
	OutTiles = TilesInRange;
}

void ANavGrid::ClearTiles()
{
	TilesInRange.Empty();
	TArray<UNavTileComponent *> AllTiles;
	GetEveryTile(AllTiles, GetWorld());
	for (auto *T : AllTiles)
	{
		T->ResetPath();
	}

	ClearTileHighlights();
	NumPersistentTiles = AllTiles.Num() - VirtualTiles.Num();
}

bool ANavGrid::TraceTileLocation(const FVector & TraceStart, const FVector & TraceEnd, FVector & OutTilePos)
{
	FCollisionQueryParams CQP;
	CQP.bFindInitialOverlaps = true;
	CQP.TraceTag = "NavGridTilePlacement";
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

	UNavTileComponent *TileComp = NewObject<UNavTileComponent>(TileOwner, TileClass);
	TileComp->SetupAttachment(TileOwner->GetRootComponent());
	TileComp->SetWorldTransform(FTransform::Identity);
	TileComp->SetWorldLocation(Location);
	TileComp->SetBoxExtent(FVector(TileSize / 2, TileSize / 2, 5));
	TileComp->RegisterComponentWithWorld(TileOwner->GetWorld());
	TileComp->SetGrid(this);

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

FVector ANavGrid::AdjustToTileLocation(const FVector &Location)
{
	UNavTileComponent *SnapTile = GetTile(Location);
	if (SnapTile)
	{
		return SnapTile->GetComponentLocation();
	}

	// try to position the pawn so that it matches a regular grid
	// we do not change the vertical location
	FVector Offset = Location - GetActorLocation();
	int32 XRest = (int32)Offset.X % (int32)TileSize;
	int32 YRest = (int32)Offset.Y % (int32)TileSize;
	FVector AdjustedLocation = Location;
	AdjustedLocation.X += (TileSize / 2) - XRest;
	AdjustedLocation.Y += (TileSize / 2) - YRest;
	return AdjustedLocation;
}

void ANavGrid::GenerateVirtualTiles(const AGridPawn *Pawn)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_ANavGrid_GenerateVirtualTiles);

	// invalidate all existing paths
	CurrentPawn = nullptr;

	// only keep a reasonable number
	if (VirtualTiles.Num() > MaxVirtualTiles)
	{
		UE_LOG(NavGrid, Log, TEXT("Limit reached (%i), removing all virtual tiles"), MaxVirtualTiles);
		DestroyVirtualTiles();
	}

	FVector Center = AdjustToTileLocation(Pawn->GetActorLocation());

	FVector Min = Center - FVector(Pawn->MovementComponent->MovementRange * TileSize);
	FVector Max = Center + FVector(Pawn->MovementComponent->MovementRange * TileSize);
	for (float X = Min.X; X <= Max.X; X += TileSize)
	{
		for (float Y = Min.Y; Y <= Max.Y; Y += TileSize)
		{
			for (float Z = Max.Z; Z >= Min.Z; Z -= TileSize)
			{
				UNavTileComponent *TileComp = ConsiderPlaceTile(FVector(X, Y, Z + TileSize), FVector(X, Y, Z - 0.1));
				if (TileComp)
				{
					VirtualTiles.Add(TileComp);
				}
			}
		}
	}
}

void ANavGrid::GenerateVirtualTile(const AGridPawn * Pawn)
{
	FVector Location = AdjustToTileLocation(Pawn->GetActorLocation());
	UNavTileComponent *TileComp = ConsiderPlaceTile(Location + FVector(0, 0, TileSize), Location - FVector(0, 0, 0.1));
	if (TileComp)
	{
		VirtualTiles.Add(TileComp);
	}
}

void ANavGrid::DestroyVirtualTiles()
{
	for (UNavTileComponent *T : VirtualTiles)
	{
		if (IsValid(T))
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

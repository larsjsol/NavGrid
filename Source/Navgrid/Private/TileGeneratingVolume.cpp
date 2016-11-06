#include "NavGridPrivatePCH.h"

void ATileGeneratingVolume::GenerateTiles()
{
	if (Tiles.Num() == 0)
	{
		FBoxSphereBounds Bounds = GetBrushComponent()->CalcBounds(GetTransform());
		FVector Center = GetActorLocation();

		float TileSize = 200; //FIXME we should fetch default tile properties from somewhere
		int32 NumX = Bounds.BoxExtent.X / TileSize;
		int32 NumY = Bounds.BoxExtent.Y / TileSize;

		float MinX = Center.X - (NumX * TileSize);
		float MaxX = Center.X + (NumX * TileSize);
		float MinY = Center.Y - (NumY * TileSize);
		float MaxY = Center.Y + (NumY * TileSize);

		for (float X = MinX; X <= MaxX; X += TileSize)
		{
			for (float Y = MinY; Y <= MaxY; Y += TileSize)
			{
				FVector TileLocation;
				bool CanPlaceTile = TraceTileLocation(FVector(X, Y, Center.Z + Bounds.BoxExtent.Z), FVector(X, Y, Center.Z - Bounds.BoxExtent.Z), TileLocation);

				if (CanPlaceTile)
				{
					UPROPERTY() UNavTileComponent *TileComp = NewObject<UNavTileComponent>(this);
					TileComp->SetupAttachment(GetRootComponent());
					TileComp->SetWorldTransform(FTransform::Identity);
					TileComp->SetWorldRotation(FQuat::Identity);
					TileComp->SetWorldLocation(TileLocation);
					TileComp->RegisterComponentWithWorld(GetWorld());
					Tiles.Add(TileComp);

					if (Tiles.Num() == MaxNumberOfTiles)
					{
						UE_LOG(NavGrid, Warning, TEXT("%s: MaxNumberOfTiles (%i) reached."), *GetName(), MaxNumberOfTiles);
						return;
					}
				}
			}
		}
	}
}

void ATileGeneratingVolume::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	if (RegenerateTiles)
	{
		for (auto *T : Tiles)
		{
			if (T && T->IsValidLowLevel())
			{
				T->DestroyComponent();
			}
		}
		Tiles.Empty();
		RegenerateTiles = false;
	}
	GenerateTiles();
}

bool ATileGeneratingVolume::TraceTileLocation(const FVector & TraceStart, const FVector & TraceEnd, FVector & OutTilePos)
{
	FCollisionQueryParams CQP;
	CQP.bFindInitialOverlaps = true;
	CQP.bTraceComplex = true;
	FHitResult HitResult;
	bool BlockingHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Pawn, CQP);
	OutTilePos = HitResult.ImpactPoint;

	return !HitResult.bStartPenetrating && BlockingHit;
}


#include "NavGridPrivatePCH.h"

void ATileGeneratingVolume::GenerateTiles()
{
	if (Tiles.Num() == 0)
	{
		FBoxSphereBounds Bounds = GetBrushComponent()->CalcBounds(GetTransform());
		FVector Center = GetActorLocation();

		float Spacing = ANavGrid::DefaultTileSpacing;
		int32 NumX = Bounds.BoxExtent.X / Spacing;
		int32 NumY = Bounds.BoxExtent.Y / Spacing;

		float MinX = Center.X - (NumX * Spacing);
		float MaxX = Center.X + (NumX * Spacing);
		float MinY = Center.Y - (NumY * Spacing);
		float MaxY = Center.Y + (NumY * Spacing);

		for (float X = MinX; X <= MaxX; X += Spacing)
		{
			for (float Y = MinY; Y <= MaxY; Y += Spacing)
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


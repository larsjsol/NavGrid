#include "NavGridPrivatePCH.h"

#include "Builders/EditorBrushBuilder.h"
#include "Builders/CubeBuilder.h"

void ATileGeneratingVolume::GenerateTiles()
{
	const UCubeBuilder *Builder = Cast<UCubeBuilder>(GetBrushBuilder());

	if (Tiles.Num() == 0 && Builder)
	{
		float Spacing = ANavGrid::DefaultTileSpacing;
		float XHalfSize = Builder->X * GetActorScale().X / 2;
		float YHalfSize = Builder->Y * GetActorScale().Y / 2;
		float ZHalfSize = Builder->Z * GetActorScale().Z / 2;

		FRotator Rotation = GetActorRotation();
		for (float X = -XHalfSize; X <= XHalfSize; X += Spacing)
		{
			for (float Y = -YHalfSize; Y <= YHalfSize; Y += Spacing)
			{
				/* rotate our local point and add the actor location to get the world position*/
				FVector WorldPos = Rotation.RotateVector(FVector(X, Y, 0)) + GetActorLocation();

				FVector TileLocation;
				bool CanPlaceTile = TraceTileLocation(FVector(WorldPos.X, WorldPos.Y, WorldPos.Z + ZHalfSize),
				                                      FVector(WorldPos.X, WorldPos.Y, WorldPos.Z - ZHalfSize), TileLocation);
				if (CanPlaceTile)
				{
					UPROPERTY() UNavTileComponent *TileComp = NewObject<UNavTileComponent>(this);
					TileComp->SetupAttachment(GetRootComponent());
					TileComp->SetWorldTransform(FTransform::Identity);
					TileComp->SetWorldRotation(Rotation.Quaternion());
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


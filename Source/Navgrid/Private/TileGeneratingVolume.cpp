#include "NavGridPrivatePCH.h"

#include "Builders/EditorBrushBuilder.h"
#include "Builders/CubeBuilder.h"

ATileGeneratingVolume::ATileGeneratingVolume()
{
	UBrushComponent *MyBrush = GetBrushComponent();
	MyBrush->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATileGeneratingVolume::GenerateTiles()
{
	const UCubeBuilder *Builder = Cast<UCubeBuilder>(GetBrushBuilder());
	ANavGrid *Grid = ANavGrid::GetNavGrid(GetWorld());
	if (!Grid)
	{
		UE_LOG(NavGrid, Error, TEXT("%s: Unable to find NavGrid"), *GetName());
		return;
	}

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
				UPROPERTY() UNavTileComponent *TileComp = Grid->ConsiderPlaceTile(
					FVector(WorldPos.X, WorldPos.Y, WorldPos.Z + ZHalfSize), 
					FVector(WorldPos.X, WorldPos.Y, WorldPos.Z - ZHalfSize), this);
				if (TileComp)
				{
					TileComp->SetWorldRotation(Rotation.Quaternion());
					Tiles.Add(TileComp);
				}
				if (Tiles.Num() == MaxNumberOfTiles)
				{
					UE_LOG(NavGrid, Warning, TEXT("%s: MaxNumberOfTiles (%i) reached."), *GetName(), MaxNumberOfTiles);
					return;
				}
			}
		}
	}
}

void ATileGeneratingVolume::DestroyTiles()
{
	for (auto *T : Tiles)
	{
		if (T && T->IsValidLowLevel())
		{
			T->DestroyComponent();
		}
	}
	Tiles.Empty();
}

void ATileGeneratingVolume::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	if (RegenerateTiles)
	{
		DestroyTiles();
		RegenerateTiles = false;
	}
	GenerateTiles();
}

void ATileGeneratingVolume::Destroyed()
{
	Super::Destroyed();
	DestroyTiles();
}

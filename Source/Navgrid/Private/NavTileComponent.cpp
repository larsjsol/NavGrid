// Fill out your copyright notice in the Description page of Project Settings.

#include "NavTileComponent.h"
#include <limits>
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UNavTileComponent::UNavTileComponent()
	:Super()
{
	PawnLocationOffset = FVector::ZeroVector;
	SetComponentTickEnabled(false);

	/* Bind mouse events */
	OnBeginCursorOver.AddDynamic(this, &UNavTileComponent::CursorOver);
	OnEndCursorOver.AddDynamic(this, &UNavTileComponent::EndCursorOver);
	OnClicked.AddDynamic(this, &UNavTileComponent::Clicked);
	/* Bind touch events */
	OnInputTouchEnter.AddDynamic(this, &UNavTileComponent::TouchEnter);
	OnInputTouchLeave.AddDynamic(this, &UNavTileComponent::TouchLeave);
	OnInputTouchEnd.AddDynamic(this, &UNavTileComponent::TouchEnd);

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // So we get mouse over events
	SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Block); // So we get mouse over events
	SetCollisionResponseToChannel(ANavGrid::ECC_NavGridWalkable, ECollisionResponse::ECR_Overlap); // So we can find the floor with a line trace

	MovementModes.Add(EGridMovementMode::Stationary);
	MovementModes.Add(EGridMovementMode::Walking);
	MovementModes.Add(EGridMovementMode::InPlaceTurn);

	ShapeColor = FColor::Magenta;

	Reset();
}

bool UNavTileComponent::Traversable(const TSet<EGridMovementMode>& PawnMovementModes) const
{
	return MovementModes.Intersect(PawnMovementModes).Num() > 0;
}

bool UNavTileComponent::LegalPositionAtEndOfTurn(const TSet<EGridMovementMode> &PawnMovementModes) const
{
	return MovementModes.Contains(EGridMovementMode::Stationary);
}

FVector UNavTileComponent::GetPawnLocation() const
{
	return GetComponentLocation() + GetComponentRotation().RotateVector(PawnLocationOffset);
}

void UNavTileComponent::SetPawnLocationOffset(const FVector &Offset)
{
	PawnLocationOffset = Offset;
}

void UNavTileComponent::SetGrid(ANavGrid * InGrid)
{
	Grid = InGrid;
}

ANavGrid * UNavTileComponent::GetGrid() const
{
	return Grid;
}

void UNavTileComponent::Reset()
{
	Distance = std::numeric_limits<float>::infinity();
	Backpointer = NULL;
	Visited = false;
}

bool UNavTileComponent::Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule) const
{
	return Obstructed(FromPos + CollisionCapsule.GetRelativeLocation(), GetPawnLocation() + CollisionCapsule.GetRelativeLocation(), CollisionCapsule);
}

bool UNavTileComponent::Obstructed(const FVector &From, const FVector &To, const UCapsuleComponent &CollisionCapsule) const
{
	FHitResult OutHit;
	FQuat Rot = FQuat::Identity;
	FCollisionShape CollisionShape = CollisionCapsule.GetCollisionShape();
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(CollisionCapsule.GetOwner());
	CQP.TraceTag = "NavGridMovement";
	return CollisionCapsule.GetWorld()->SweepSingleByChannel(OutHit, From, To, Rot, ECollisionChannel::ECC_Pawn, CollisionShape, CQP);
}

void UNavTileComponent::GetNeighbours(const UCapsuleComponent & CollisionCapsule, TArray<UNavTileComponent*>& OutUnObstructed, TArray<UNavTileComponent*>& OutObstructed)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UNavTileComponent_GetNeighbours);

	OutUnObstructed.Empty();
	OutObstructed.Empty();

	if (IsValid(Grid))
	{
		FVector MyExtent = BoxExtent + FVector(Grid->TileSize * 0.75);
		TArray<FHitResult> HitResults;
		Grid->GetWorld()->SweepMultiByChannel(HitResults, GetComponentLocation(), GetComponentLocation() + FVector(0, 0, 1), GetComponentQuat(), Grid->ECC_NavGridWalkable, FCollisionShape::MakeBox(MyExtent));
		for (FHitResult &Hit : HitResults)
		{
			UNavTileComponent *HitTile = Cast<UNavTileComponent>(Hit.GetComponent());
			if (IsValid(HitTile))
			{
				if (HitTile != this && !HitTile->Obstructed(GetPawnLocation(), CollisionCapsule))
				{
					OutUnObstructed.AddUnique(HitTile);
				}
				else
				{
					OutObstructed.AddUnique(HitTile);
				}
			}
		}
	}
}

void UNavTileComponent::GetUnobstructedNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutNeighbours)
{
	TArray<UNavTileComponent *> Dummy;
	GetNeighbours(CollisionCapsule, OutNeighbours, Dummy);
}

void UNavTileComponent::Clicked(UPrimitiveComponent* TouchedComponent, FKey Key)
{
	Grid->TileClicked(this);
}

void UNavTileComponent::CursorOver(UPrimitiveComponent* TouchedComponent)
{
	Grid->TileCursorOver(this);
}

void UNavTileComponent::EndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	Grid->EndTileCursorOver(this);
}

void UNavTileComponent::TouchEnter(ETouchIndex::Type Type, UPrimitiveComponent* TouchedComponent)
{
	CursorOver(TouchedComponent);
}

void UNavTileComponent::TouchLeave(ETouchIndex::Type Type, UPrimitiveComponent* TouchedComponent)
{
	EndCursorOver(TouchedComponent);
}

void UNavTileComponent::TouchEnd(ETouchIndex::Type Type, UPrimitiveComponent* TouchedComponent)
{
	Grid->TileClicked(this);
}

void UNavTileComponent::AddPathSegments(USplineComponent &OutSpline, TArray<FPathSegment> &OutPathSegments, bool EndTile) const
{
	FVector EntryPoint = OutSpline.GetLocationAtSplinePoint(OutSpline.GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local);
	float SegmentStart = OutSpline.GetSplineLength();
	OutSpline.AddSplinePoint(GetComponentLocation() + PawnLocationOffset, ESplineCoordinateSpace::Local);
	OutPathSegments.Add(FPathSegment(MovementModes, SegmentStart, OutSpline.GetSplineLength()));
}

FVector UNavTileComponent::GetSplineMeshUpVector()
{
	return FVector(0, 0, 1);
}

void UNavTileComponent::SetHighlight(FName NewHighlightType)
{
	auto *HighlightComponent = Grid->GetHighlightComponent(NewHighlightType);
	if (HighlightComponent)
	{
		FVector MeshSize = HighlightComponent->GetStaticMesh()->GetBoundingBox().GetSize();
		FVector TileSize = GetScaledBoxExtent() * 2;
		FTransform Transform = GetComponentTransform();
		Transform.SetScale3D(FVector(TileSize.X / MeshSize.X, TileSize.Y / MeshSize.Y, 1));
		HighlightComponent->AddInstanceWorldSpace(Transform);
	}
}

void UNavTileComponent::DrawDebug(UCapsuleComponent *CollisionCapsule, bool bPersistentLines, float LifeTime, float Thickness)
{
	DrawDebugCapsule(GetWorld(), GetPawnLocation() + CollisionCapsule->GetRelativeLocation(), CollisionCapsule->GetScaledCapsuleHalfHeight(), CollisionCapsule->GetScaledCapsuleRadius(),
		CollisionCapsule->GetComponentQuat(), FColor::Cyan, bPersistentLines, LifeTime, 0, Thickness);
	DrawDebugBox(GetWorld(), GetComponentLocation(), BoxExtent, GetComponentQuat(), FColor::Cyan, bPersistentLines, LifeTime, 0, Thickness);
	if (IsValid(Grid))
	{
		DrawDebugBox(GetWorld(), GetComponentLocation(), BoxExtent + FVector(Grid->TileSize * 0.75), GetComponentQuat(), FColor::Blue, bPersistentLines, LifeTime, 0, Thickness);
	}
	TArray<UNavTileComponent *> UnObstructed, Obstructed;
	GetNeighbours(*CollisionCapsule, UnObstructed, Obstructed);
	for (UNavTileComponent *Tile : UnObstructed)
	{
		DrawDebugLine(GetWorld(), GetPawnLocation() + CollisionCapsule->GetRelativeLocation(), CollisionCapsule->GetRelativeLocation() + ((GetPawnLocation() + Tile->GetPawnLocation()) / 2), FColor::Green, bPersistentLines, LifeTime, 0, Thickness);
	}
	for (UNavTileComponent *Tile : Obstructed)
	{
		DrawDebugLine(GetWorld(), GetPawnLocation() + CollisionCapsule->GetRelativeLocation(), CollisionCapsule->GetRelativeLocation() + ((GetPawnLocation() + Tile->GetPawnLocation()) / 2), FColor::Red, bPersistentLines, LifeTime, 0, Thickness);
	}
}

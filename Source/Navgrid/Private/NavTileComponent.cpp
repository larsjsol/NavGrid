// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"
#include <limits>

UNavTileComponent::UNavTileComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
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

	ShapeColor = FColor::Magenta;

	ResetPath();
}

bool UNavTileComponent::Traversable(float MaxWalkAngle, const TArray<EGridMovementMode>& AvailableMovementModes) const
{
	FRotator TileRot = GetComponentRotation();
	float MaxAngle = FMath::Max3<float>(TileRot.Pitch, TileRot.Yaw, TileRot.Roll);
	float MinAngle = FMath::Min3<float>(TileRot.Pitch, TileRot.Yaw, TileRot.Roll);
	if (AvailableMovementModes.Contains(EGridMovementMode::Walking) &&
		(MaxAngle < MaxWalkAngle && MinAngle > -MaxWalkAngle))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool UNavTileComponent::LegalPositionAtEndOfTurn(float MaxWalkAngle, const TArray<EGridMovementMode> &AvailableMovementModes) const
{
	return Traversable(MaxWalkAngle, AvailableMovementModes);
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
	SetCollisionResponseToChannel(Grid->ECC_NavGridWalkable, ECollisionResponse::ECR_Overlap); // So we can find the floor with a line trace
}

ANavGrid * UNavTileComponent::GetGrid() const
{
	return Grid;
}

void UNavTileComponent::ResetPath()
{
	Distance = std::numeric_limits<float>::infinity();
	Backpointer = NULL;
	Visited = false;
}

TArray<FVector>* UNavTileComponent::GetContactPoints()
{
	if (!ContactPoints.Num())
	{
		int32 XExtent = GetScaledBoxExtent().X;
		int32 YExtent = GetScaledBoxExtent().Y;
		for (int32 X = -XExtent; X <= XExtent; X += XExtent)
		{
			for (int32 Y = -YExtent; Y <= YExtent; Y += YExtent)
			{
				FVector PointLocation = GetComponentRotation().RotateVector(FVector(X, Y, 0));
				FVector WorldLocation = GetComponentLocation() + PointLocation;
				ContactPoints.Add(WorldLocation);
			}
		}
	}
	return &ContactPoints;
}

TArray<UNavTileComponent*>* UNavTileComponent::GetNeighbours()
{
	float MaxDistance = Grid->TileSize * 0.75;

	Neighbours.Empty();
	for (TObjectIterator<UNavTileComponent> Itr; Itr; ++Itr)
	{
		if (Itr->GetWorld() == GetWorld() && *Itr != this)
		{
			bool Added = false; // stop comparing CPs when we know a tile is a neighbour
			for (const FVector &OtherCP : *Itr->GetContactPoints())
			{
				for (const FVector &MyCP : *GetContactPoints())
				{
					if ((OtherCP - MyCP).Size() < MaxDistance)
					{
						Neighbours.Add(*Itr);
						Added = true;
						break;
					}
				}
				if (Added) { break; }
			}
		}
	}
	return &Neighbours;
}

bool UNavTileComponent::Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule) const
{
	return Obstructed(FromPos, PawnLocationOffset + GetComponentLocation(), CollisionCapsule);
}

bool UNavTileComponent::Obstructed(const FVector & From, const FVector & To, const UCapsuleComponent & CollisionCapsule)
{
	FHitResult OutHit;
	FVector Start = From + CollisionCapsule.RelativeLocation;
	FVector End = To + CollisionCapsule.RelativeLocation;
	FQuat Rot = FQuat::Identity;
	FCollisionShape CollisionShape = CollisionCapsule.GetCollisionShape();
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(CollisionCapsule.GetOwner());
	CQP.TraceTag = "NavGridMovement";

	return CollisionCapsule.GetWorld()->SweepSingleByChannel(OutHit, Start, End, Rot, ECollisionChannel::ECC_Pawn, CollisionShape, CQP);
}

void UNavTileComponent::GetUnobstructedNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutNeighbours)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UNavTileComponent_GetUnobstructedNeighbours);

	OutNeighbours.Empty();
	for (auto N : *GetNeighbours())
	{
		if (!N->Obstructed(PawnLocationOffset + GetComponentLocation(), CollisionCapsule))
		{
			OutNeighbours.Add(N);
		}
	}
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

void UNavTileComponent::AddSplinePoints(const FVector &FromPos, USplineComponent &OutSpline, bool EndTile) const
{
	OutSpline.AddSplinePoint(GetComponentLocation() + PawnLocationOffset, ESplineCoordinateSpace::Local, false);
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
		FTransform HighlightTransform = GetComponentTransform();
		FVector HighlightLocation = HighlightTransform.GetLocation();
		HighlightLocation.Z += Grid->UIOffset;
		HighlightTransform.SetLocation(HighlightLocation);
		HighlightComponent->AddInstanceWorldSpace(HighlightTransform);
	}
}

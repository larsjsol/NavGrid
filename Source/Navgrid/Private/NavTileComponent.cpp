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
	FindNeighbours();
}

ANavGrid * UNavTileComponent::GetGrid() const
{
	return Grid;
}

void UNavTileComponent::DestroyComponent(bool bPromoteChildren)
{
	for (UNavTileComponent *Neighbour : Neighbours)
	{
		Neighbour->RemoveNeighbour(this);
	}
	Super::DestroyComponent(bPromoteChildren);
}

void UNavTileComponent::UpdateBodySetup()
{
	Super::UpdateBodySetup();

	FVector NeighbourhoodExtent = BoxExtent;
	/* make the tile taller so it will be included in neighbourhoods that spread over slopes */
	if (IsValid(Grid))
	{
		NeighbourhoodExtent.Z = FMath::Max<float>(NeighbourhoodExtent.Z, Grid->TileSize / 2);
	}
	/* Make the shape slightly larger than the actual tile so it will intersect its neighbours */
	NeighbourhoodExtent += FVector(15);
	NeighbourhoodShape = FCollisionShape::MakeBox(NeighbourhoodExtent);
}

void UNavTileComponent::ResetPath()
{
	Distance = std::numeric_limits<float>::infinity();
	Backpointer = NULL;
	Visited = false;
}

void UNavTileComponent::FindNeighbours()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UNavTileComponent_FindNeighbours);
	check(IsRegistered());
	UpdateBodySetup(); //make sure that Extent and NeighbourhoodShape is updated
	Neighbours.Empty();
	for (TObjectIterator<UNavTileComponent> Itr; Itr; ++Itr)
	{
		if (Itr->GetWorld() == GetWorld() && *Itr != this)
		{
			if (OverlapComponent(Itr->GetComponentLocation(), Itr->GetComponentRotation().Quaternion(), Itr->NeighbourhoodShape) ||
				Itr->OverlapComponent(GetComponentLocation(), GetComponentRotation().Quaternion(), NeighbourhoodShape))
			{
				AddNeighbour(*Itr);
				Itr->AddNeighbour(this);
			}
		}
	}
}

bool UNavTileComponent::Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule) const
{
	return Obstructed(FromPos + CollisionCapsule.RelativeLocation, GetComponentLocation() + PawnLocationOffset + CollisionCapsule.RelativeLocation, CollisionCapsule);
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
		FVector MeshSize = HighlightComponent->GetStaticMesh()->GetBoundingBox().GetSize();
		FTransform Transform = GetComponentTransform();
		Transform.SetScale3D(FVector(Grid->TileSize / MeshSize.X, Grid->TileSize / MeshSize.Y, 1));
		HighlightComponent->AddInstanceWorldSpace(Transform);
	}
}

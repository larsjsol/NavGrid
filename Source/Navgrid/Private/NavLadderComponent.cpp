// Fill out your copyright notice in the Description page of Project Settings.

#include "NavLadderComponent.h"

UNavLadderComponent::UNavLadderComponent()
	:Super()
{
	MovementModes.Empty();
	MovementModes.Add(EGridMovementMode::ClimbingUp);
	MovementModes.Add(EGridMovementMode::ClimbingDown);
}

void UNavLadderComponent::SetGrid(ANavGrid *InGrid)
{
	Super::SetGrid(InGrid);
	TileSize = InGrid->TileSize;
}

void UNavLadderComponent::GetNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutUnObstructed, TArray<UNavTileComponent *> &OutObstructed)
{
	OutUnObstructed.Empty();
	OutObstructed.Empty();
	if (IsValid(Grid))
	{
		FCollisionShape Shape = FCollisionShape::MakeBox(BoxExtent + FVector(Grid->TileSize / 2));

		TArray<FHitResult> HitResults;
		TArray<UNavTileComponent *> AllNeighbours;
		Grid->GetWorld()->SweepMultiByChannel(HitResults, GetComponentLocation(), GetComponentLocation() + FVector(0, 0, 1), GetComponentQuat(), Grid->ECC_NavGridWalkable, Shape);
		for (FHitResult &Hit : HitResults)
		{
			UNavTileComponent *HitTile = Cast<UNavTileComponent>(Hit.GetComponent());
			if (IsValid(HitTile) && HitTile != this)
			{
				AllNeighbours.AddUnique(HitTile);
			}
		}

		for (UNavTileComponent *N : AllNeighbours)
		{
			//Determine if we should trace from the top or bottom point
			float TopDistance = (GetTopPathPoint() - N->GetPawnLocation()).Size();
			float BottomDistance = (GetBottomPathPoint() - N->GetPawnLocation()).Size();
			FVector TracePoint = TopDistance < BottomDistance ? GetTopPathPoint() : GetBottomPathPoint();
			if (N->Obstructed(TracePoint, CollisionCapsule))
			{
				OutObstructed.Add(N);
			}
			else
			{
				OutUnObstructed.Add(N);
			}
		}
	}
}

bool UNavLadderComponent::Obstructed(const FVector & FromPos, const UCapsuleComponent & CollisionCapsule) const
{
	//Determine if we should trace to the top or bottom point
	float TopDistance = (GetTopPathPoint() - FromPos).Size();
	float BottomDistance = (GetBottomPathPoint() - FromPos).Size();
	FVector TracePoint = TopDistance < BottomDistance ? GetTopPathPoint() : GetBottomPathPoint();

	FHitResult OutHit;
	FCollisionShape CollisionShape = CollisionCapsule.GetCollisionShape();
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(CollisionCapsule.GetOwner());
	CQP.TraceTag = "NavGridMovement";
	return CollisionCapsule.GetWorld()->SweepSingleByChannel(OutHit, FromPos + CollisionCapsule.GetRelativeLocation(), TracePoint + CollisionCapsule.GetRelativeLocation(),
		GetComponentQuat(), ECollisionChannel::ECC_Pawn, CollisionShape, CQP);
}

void UNavLadderComponent::AddPathSegments(USplineComponent &OutSpline, TArray<FPathSegment> &OutPathSegments, bool EndTile) const
{
	FVector EntryPoint = OutSpline.GetLocationAtSplinePoint(OutSpline.GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local);
	float TopDistance = (GetTopPathPoint() - EntryPoint).Size();
	float BottomDistance = (GetBottomPathPoint() - EntryPoint).Size();

	FPathSegment NewSegment;
	NewSegment.MovementModes = MovementModes;
	NewSegment.PawnRotationHint = GetComponentRotation();
	NewSegment.PawnRotationHint.Yaw -= 180;

	// add spline points and segments
	if (TopDistance > BottomDistance)
	{
		OutSpline.AddSplinePoint(GetBottomPathPoint(), ESplineCoordinateSpace::Local);
		NewSegment.Start = OutSpline.GetSplineLength();
		OutSpline.AddSplinePoint(GetTopPathPoint(), ESplineCoordinateSpace::Local);
		NewSegment.End = OutSpline.GetSplineLength();
	}
	else
	{
		OutSpline.AddSplinePoint(GetTopPathPoint(), ESplineCoordinateSpace::Local);
		NewSegment.Start = OutSpline.GetSplineLength();
		OutSpline.AddSplinePoint(GetBottomPathPoint(), ESplineCoordinateSpace::Local);
		NewSegment.End = OutSpline.GetSplineLength();
	}

	// unlike regular tiles, we do not want the pawn to change movement mode untill it reaches the first path point
	// we therefore extend the previous segment to that point
	if (OutPathSegments.Num())
	{
		OutPathSegments.Last().End = NewSegment.Start;
	}

	// add the new segment
	OutPathSegments.Add(NewSegment);

	if (EndTile)
	{
		OutSpline.RemoveSplinePoint(OutSpline.GetNumberOfSplinePoints() - 1);
		OutSpline.AddSplinePoint(PawnLocationOffset + GetComponentLocation(), ESplineCoordinateSpace::Local);
	}
}

FVector UNavLadderComponent::GetSplineMeshUpVector()
{
	FRotator Rot = GetComponentRotation();
	FVector UpVector = Rot.RotateVector(FVector(0, -1, 0));
	return UpVector;
}

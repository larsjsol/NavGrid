// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

UNavLadderComponent::UNavLadderComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	BottomPathPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BottomPathPoint"));
	BottomPathPoint->SetupAttachment(this);

	TopPathPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopPathPoint"));
	TopPathPoint->SetupAttachment(this);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(this);

	MovementModes.Empty();
	MovementModes.Add(EGridMovementMode::ClimbingUp);
	MovementModes.Add(EGridMovementMode::ClimbingDown);
}

void UNavLadderComponent::SetGrid(ANavGrid *InGrid)
{
	Super::SetGrid(InGrid);

	PawnLocationOffset = GetComponentRotation().RotateVector(FVector(90, 0, 0));
	BottomPathPoint->SetRelativeLocation(FVector(Grid->TileSize * 0.75, 0, 50 - BoxExtent.Z));
	TopPathPoint->SetRelativeLocation(FVector(Grid->TileSize * 0.75, 0, BoxExtent.Z - 25));
}

FVector UNavLadderComponent::GetPawnLocation() const
{
	return (BottomPathPoint->GetComponentLocation() + TopPathPoint->GetComponentLocation()) / 2;
}

void UNavLadderComponent::GetUnobstructedNeighbours(const UCapsuleComponent & CollisionCapsule, TArray<UNavTileComponent*>& OutNeighbours)
{
	OutNeighbours.Empty();
	if (IsValid(Grid))
	{
		FCollisionShape Shape = FCollisionShape::MakeBox(BoxExtent + FVector(Grid->TileSize / 2));

		TArray<FHitResult> HitResults;
		Grid->GetWorld()->SweepMultiByChannel(HitResults, GetComponentLocation(), GetComponentLocation() + FVector(0, 0, 1), FQuat(), Grid->ECC_NavGridWalkable, Shape);
		for (FHitResult &Hit : HitResults)
		{
			UNavTileComponent *HitTile = Cast<UNavTileComponent>(Hit.GetComponent());
			if (IsValid(HitTile) && HitTile != this)
			{
				OutNeighbours.AddUnique(HitTile);
			}
		}
	}

	for (int32 Idx = OutNeighbours.Num() - 1; Idx >= 0; Idx--)
	{
		//Determine if we should trace from the top or bottom point
		float TopDistance = (TopPathPoint->GetComponentLocation() - OutNeighbours[Idx]->GetPawnLocation()).Size();
		float BottomDistance = (BottomPathPoint->GetComponentLocation() - OutNeighbours[Idx]->GetPawnLocation()).Size();
		FVector TracePoint = TopDistance < BottomDistance ? TopPathPoint->GetComponentLocation() : BottomPathPoint->GetComponentLocation();

		if (OutNeighbours[Idx]->Obstructed(TracePoint, CollisionCapsule))
		{
			OutNeighbours.RemoveAt(Idx);
		}
	}
}

bool UNavLadderComponent::Obstructed(const FVector & FromPos, const UCapsuleComponent & CollisionCapsule) const
{
	//Determine if we should trace to the top or bottom point
	float TopDistance = (TopPathPoint->GetComponentLocation() - FromPos).Size();
	float BottomDistance = (BottomPathPoint->GetComponentLocation() - FromPos).Size();
	FVector TracePoint = TopDistance < BottomDistance ? TopPathPoint->GetComponentLocation() : BottomPathPoint->GetComponentLocation();

	FHitResult OutHit;
	FCollisionShape CollisionShape = CollisionCapsule.GetCollisionShape();
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(CollisionCapsule.GetOwner());
	CQP.TraceTag = "NavGridMovement";
	return CollisionCapsule.GetWorld()->SweepSingleByChannel(OutHit, FromPos + CollisionCapsule.RelativeLocation, TracePoint + CollisionCapsule.RelativeLocation,
		GetComponentQuat(), ECollisionChannel::ECC_Pawn, CollisionShape, CQP);
}

bool UNavLadderComponent::Traversable(float MaxWalkAngle, const TSet<EGridMovementMode>& PawnMovementModes) const
{
	return MovementModes.Intersect(PawnMovementModes).Num() > 0;
}

void UNavLadderComponent::AddPathSegments(USplineComponent &OutSpline, TArray<FPathSegment> &OutPathSegments, bool EndTile) const
{
	FVector EntryPoint = OutSpline.GetLocationAtSplinePoint(OutSpline.GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local);
	float TopDistance = (TopPathPoint->GetComponentLocation() - EntryPoint).Size();
	float BottomDistance = (BottomPathPoint->GetComponentLocation() - EntryPoint).Size();

	FPathSegment NewSegment;
	NewSegment.MovementModes = MovementModes;
	NewSegment.PawnRotationHint = GetComponentRotation();
	NewSegment.PawnRotationHint.Yaw -= 180;

	// add spline points and segments
	if (TopDistance > BottomDistance)
	{
		OutSpline.AddSplinePoint(BottomPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
		NewSegment.Start = OutSpline.GetSplineLength();
		OutSpline.AddSplinePoint(TopPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
		NewSegment.End = OutSpline.GetSplineLength();
	}
	else
	{
		OutSpline.AddSplinePoint(TopPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
		NewSegment.Start = OutSpline.GetSplineLength();
		OutSpline.AddSplinePoint(BottomPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
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

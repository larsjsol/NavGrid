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
}

void UNavLadderComponent::BeginPlay()
{
	Super::BeginPlay();

	// it seems like 'BoxExtent' still has the default values in PostInitProperties() 
	PawnLocationOffset = GetComponentRotation().RotateVector(FVector(90, 0, 0));
	BottomPathPoint->SetRelativeLocation(FVector(60, 0, -BoxExtent.Z + 50));
	TopPathPoint->SetRelativeLocation(FVector(60, 0, BoxExtent.Z - 25));
}

TArray<FVector>* UNavLadderComponent::GetContactPoints()
{
	if (!ContactPoints.Num())
	{
		float ZExtent = GetScaledBoxExtent().Z;
		FVector RelativeTop = GetComponentRotation().RotateVector(FVector(0, 0, ZExtent));
		ContactPoints.Add(GetComponentLocation() + RelativeTop);
		FVector RelativeBottom = GetComponentRotation().RotateVector(FVector(0, 0, -ZExtent));
		ContactPoints.Add(GetComponentLocation() + RelativeBottom);
	}
	return &ContactPoints;
}

FVector UNavLadderComponent::GetPawnLocation()
{
	return (BottomPathPoint->GetComponentLocation() + TopPathPoint->GetComponentLocation()) / 2;
}

void UNavLadderComponent::GetUnobstructedNeighbours(const UCapsuleComponent & CollisionCapsule, TArray<UNavTileComponent*>& OutNeighbours)
{
	OutNeighbours.Empty();
	for (auto N : *GetNeighbours())
	{
		//Determine if we should trace from the top or bottom point
		float TopDistance = (TopPathPoint->GetComponentLocation() - N->GetPawnLocation()).Size();
		float BottomDistance = (BottomPathPoint->GetComponentLocation() - N->GetPawnLocation()).Size();
		FVector TracePoint = TopDistance < BottomDistance ? TopPathPoint->GetComponentLocation() : BottomPathPoint->GetComponentLocation();

		if (!UNavTileComponent::Obstructed(TracePoint, N->GetPawnLocation(), CollisionCapsule))
		{
			OutNeighbours.Add(N);
		}
	}
}

bool UNavLadderComponent::Obstructed(const FVector & FromPos, const UCapsuleComponent & CollisionCapsule)
{
	//Determine if we should trace to the top or bottom point
	float TopDistance = (TopPathPoint->GetComponentLocation() - FromPos).Size();
	float BottomDistance = (BottomPathPoint->GetComponentLocation() - FromPos).Size();
	FVector TracePoint = TopDistance < BottomDistance ? TopPathPoint->GetComponentLocation() : BottomPathPoint->GetComponentLocation();

	return UNavTileComponent::Obstructed(FromPos, TracePoint, CollisionCapsule);
}

bool UNavLadderComponent::Traversable(float MaxWalkAngle, const TArray<EGridMovementMode>& AvailableMovementModes) const
{
	return AvailableMovementModes.Contains(EGridMovementMode::ClimbingDown) || AvailableMovementModes.Contains(EGridMovementMode::ClimbingUp);
}

bool UNavLadderComponent::LegalPositionAtEndOfTurn(float MaxWalkAngle, const TArray<EGridMovementMode>& AvailableMovementModes) const
{
	return false;
}


void UNavLadderComponent::AddSplinePoints(const FVector &FromPos, USplineComponent &OutSpline, bool LastTile)
{
	float TopDistance = (TopPathPoint->GetComponentLocation() - FromPos).Size();
	float BottomDistance = (BottomPathPoint->GetComponentLocation() - FromPos).Size();
	if (TopDistance > BottomDistance)
	{
		OutSpline.AddSplinePoint(BottomPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
		OutSpline.AddSplinePoint(TopPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
	}
	else
	{
		OutSpline.AddSplinePoint(TopPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
		OutSpline.AddSplinePoint(BottomPathPoint->GetComponentLocation(), ESplineCoordinateSpace::Local);
	}

	if (LastTile)
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

// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "NavLadderComponent.h"

UNavLadderComponent::UNavLadderComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	Extent->SetBoxExtent(FVector(50, 5, 150));
	Extent->SetRelativeLocation(FVector(0, 0, 150));

	PawnLocationOffset->SetRelativeLocation(FVector(0, -50, 150));

	BottomPathPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BottomPathPoint"));
	BottomPathPoint->SetRelativeLocation(FVector(0, -50, 0));
	BottomPathPoint->SetupAttachment(this);

	TopPathPoint = CreateDefaultSubobject<USceneComponent>(TEXT("TopPathPoint"));
	TopPathPoint->SetRelativeLocation(FVector(0, -50, 270));
	TopPathPoint->SetupAttachment(this);
}

TArray<FVector>* UNavLadderComponent::GetContactPoints()
{
	if (!ContactPoints.Num())
	{
		int32 ZExtent = Extent->GetScaledBoxExtent().Z;
		FVector RelativeTop = GetComponentRotation().RotateVector(FVector(0, 0, 2 * ZExtent));
		ContactPoints.Add(GetComponentLocation() + RelativeTop);
		FVector RelativeBottom = GetComponentRotation().RotateVector(FVector(0, 0, 0));
		ContactPoints.Add(GetComponentLocation() + RelativeBottom);
	}
	return &ContactPoints;
}

void UNavLadderComponent::GetUnobstructedNeighbours(const UCapsuleComponent & CollisionCapsule, TArray<UNavTileComponent*>& OutNeighbours)
{
	OutNeighbours.Empty();
	for (auto N : *GetNeighbours())
	{
		//Determine if we should trace from the top or bottom point
		float TopDistance = (TopPathPoint->GetComponentLocation() - N->PawnLocationOffset->GetComponentLocation()).Size();
		float BottomDistance = (BottomPathPoint->GetComponentLocation() - N->PawnLocationOffset->GetComponentLocation()).Size();
		FVector TracePoint = TopDistance < BottomDistance ? TopPathPoint->GetComponentLocation() : BottomPathPoint->GetComponentLocation();

		if (!UNavTileComponent::Obstructed(PawnLocationOffset->GetComponentLocation(), TracePoint, CollisionCapsule) &&
			!UNavTileComponent::Obstructed(TracePoint, N->PawnLocationOffset->GetComponentLocation(), CollisionCapsule))
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

	return UNavTileComponent::Obstructed(FromPos, TracePoint, CollisionCapsule) || 
		UNavTileComponent::Obstructed(TracePoint, PawnLocationOffset->GetComponentLocation(), CollisionCapsule);
}

void UNavLadderComponent::GetPathPoints(const FVector &FromPos, TArray<FVector>& OutPathPoints, TArray<FVector> &OutUpVectors)
{
	OutPathPoints.Empty();

	// points suitable if the pawn enters from the top
	OutPathPoints.Add(TopPathPoint->GetComponentLocation());
	OutPathPoints.Add(PawnLocationOffset->GetComponentLocation());
	OutPathPoints.Add(BottomPathPoint->GetComponentLocation());

	// reverse the list if the pawn entered from the bottom
	float TopDistance = (TopPathPoint->GetComponentLocation() - FromPos).Size();
	float BottomDistance = (BottomPathPoint->GetComponentLocation() - FromPos).Size();
	if (TopDistance > BottomDistance)
	{
		Algo::Reverse(OutPathPoints);
	}

	OutUpVectors.Empty();
	FVector Edge = GetComponentRotation().RotateVector(FVector(0, -1, 1)).GetSafeNormal();
	FVector Middle = GetComponentRotation().RotateVector(FVector(0, -1, 0));
	OutUpVectors.Add(Edge);
	OutUpVectors.Add(Middle);
	OutUpVectors.Add(Edge);
}
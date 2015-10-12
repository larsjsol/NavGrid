// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "NavGrid.h"
#include "GridMovementComponent.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

UGridMovementComponent::UGridMovementComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	Spline = ObjectInitializer.CreateDefaultSubobject<USplineComponent>(this, "PathSpline");
}

void UGridMovementComponent::BeginPlay()
{
	/* Grab a reference to the NavGrid */
	TActorIterator<ANavGrid> NavGridItr(GetWorld());
	Grid = *NavGridItr;
	if (!Grid) { UE_LOG(NavGrid, Error, TEXT("%st: Unable to get reference to Navgrid."), *GetName()); }
}

bool UGridMovementComponent::CreatePath(const ATile &Target)
{
	Spline->ClearSplinePoints();

	ATile *Location = Grid->GetTile(GetOwner()->GetActorLocation());
	if (!Location) { UE_LOG(NavGrid, Error, TEXT("%s: Not on grid"), *GetOwner()->GetName()); return false; }

	TArray<ATile *> InRange;
	Grid->TilesInRange(Location, InRange, MovementRange);
	if (InRange.Contains(&Target))
	{
		TArray<ATile *> Path;
		ATile *Current = (ATile *) &Target;
		while (Current)
		{
			Path.Add(Current);
			Current = Current->Backpointer;
		}

		for (int32 Idx = Path.Num() - 1; Idx >= 0; Idx--)
		{
			Spline->AddSplineWorldPoint(Path[Idx]->GetActorLocation());
		}
		return true;
	}
	else { return false; }
}

void UGridMovementComponent::FollowPath()
{

}

bool UGridMovementComponent::MoveTo(const ATile &Target)
{
	bool PathExists = CreatePath(Target);
	if (PathExists) { FollowPath(); }
	return PathExists;
}

void UGridMovementComponent::ShowPath()
{
	if (PathMesh)
	{
		float Distance = 0;
		FBoxSphereBounds Bounds = PathMesh->GetBounds();
		float MeshLength = FMath::Abs(Bounds.BoxExtent.X);
		float SplineLength = Spline->GetSplineLength();

		Spline->SetRelativeLocation(FVector::ZeroVector);

		while (Distance < SplineLength)
		{
			AddSplineMesh(Distance, FMath::Min(Distance + MeshLength, SplineLength));
			Distance += FMath::Min(MeshLength, SplineLength - Distance);
		}
	}
}

void UGridMovementComponent::HidePath()
{
	for (USplineMeshComponent *SMesh : SplineMeshes)
	{
		SMesh->DestroyComponent();
	}
	SplineMeshes.Empty();
}

void UGridMovementComponent::AddSplineMesh(float From, float To)
{
	float TanScale = 25;
	FVector StartPos = Spline->GetWorldLocationAtDistanceAlongSpline(From);
	StartPos.Z += VerticalOffest;
	FVector StartTan = Spline->GetWorldDirectionAtDistanceAlongSpline(From) * TanScale;
	FVector EndPos = Spline->GetWorldLocationAtDistanceAlongSpline(To);
	EndPos.Z += VerticalOffest;
	FVector EndTan = Spline->GetWorldDirectionAtDistanceAlongSpline(To) * TanScale;

	UPROPERTY() USplineMeshComponent *SplineMesh = NewObject<USplineMeshComponent>(this);
	SplineMesh->RegisterComponentWithWorld(GetWorld());
	SplineMesh->SetMobility(EComponentMobility::Movable);

	SplineMesh->SetStaticMesh(PathMesh);
	SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);

	SplineMeshes.Add(SplineMesh);
}
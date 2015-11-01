// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "GridPawn.h"
#include "GridMovementComponent.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

UGridMovementComponent::UGridMovementComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	Spline = ObjectInitializer.CreateDefaultSubobject<USplineComponent>(this, "PathSpline");
	PathMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Path.NavGrid_Path'")).Object;
}

void UGridMovementComponent::BeginPlay()
{
	/* Grab a reference to the NavGrid */
	TActorIterator<ANavGrid> NavGridItr(GetWorld());
	Grid = *NavGridItr;
	if (!Grid) { UE_LOG(NavGrid, Error, TEXT("%st: Unable to get reference to Navgrid."), *GetName()); }
}

void UGridMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Moving)
	{
		/* Find the next location */
		float RemainingDistance = Spline->GetSplineLength();
		Distance = FMath::Min(Spline->GetSplineLength(), Distance + (MaxSpeed * DeltaTime));
		
		/* Move and rotate the actor */
		AActor *Owner = GetOwner();
		FTransform OldTransform = Owner->GetTransform();
		FTransform NewTransform = Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
		FVector OldLocation = Owner->GetActorLocation();

		/* Restrain rotation axis */
		FRotator Rotation = NewTransform.Rotator();
		Rotation.Roll = LockRoll ? 0 : Rotation.Roll;
		Rotation.Pitch = LockPitch ? 0 : Rotation.Pitch; 
		Rotation.Yaw = LockYaw ? 0 : Rotation.Yaw;
		NewTransform.SetRotation(Rotation.Quaternion());

		Owner->SetActorTransform(NewTransform);

		/* Check if we're reached our destination*/
		if (Distance >= Spline->GetSplineLength())
		{
			Moving = false;
			Distance = 0;
			Velocity = FVector::ZeroVector;
			OnMovementEndEvent.Broadcast();
		}
		else
		{
			Velocity = (NewTransform.GetLocation() - OldTransform.GetLocation()) * (1 / DeltaTime);
		}

		// update velocity so it can be fetched by the pawn 
		UpdateComponentVelocity();
	}
}

bool UGridMovementComponent::CreatePath(const ATile &Target)
{
	Spline->ClearSplinePoints();

	AActor *Owner = GetOwner();
	AGridPawn *GridPawnOwner = Cast<AGridPawn>(Owner);

	ATile *Location = Grid->GetTile(Owner->GetActorLocation());
	if (!Location) { UE_LOG(NavGrid, Error, TEXT("%s: Not on grid"), *Owner->GetName()); return false; }

	TArray<ATile *> InRange;
	if (GridPawnOwner) {
		Grid->TilesInRange(Location, InRange, MovementRange, true, GridPawnOwner->CapsuleComponent);
	}
	else
	{
		Grid->TilesInRange(Location, InRange, MovementRange);
	}

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
			Spline->AddSplinePoint(Path[Idx]->GetActorLocation(), ESplineCoordinateSpace::Local);
		}
		return true;
	}
	else { return false; }
}

void UGridMovementComponent::FollowPath()
{
	/* Set the Moving flag, the real work is done in TickComponent() */
	Moving = true;
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
		float Distance = HorizontalOffset; // Get some distance between the actor and the path
		FBoxSphereBounds Bounds = PathMesh->GetBounds();
		float MeshLength = FMath::Abs(Bounds.BoxExtent.X);
		float SplineLength = Spline->GetSplineLength();
		SplineLength -= HorizontalOffset; // Get some distance between the cursor and the path

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
	FVector StartPos = Spline->GetLocationAtDistanceAlongSpline(From, ESplineCoordinateSpace::Local);
	StartPos.Z += VerticalOffest;
	FVector StartTan = Spline->GetDirectionAtDistanceAlongSpline(From, ESplineCoordinateSpace::Local) * TanScale;
	FVector EndPos = Spline->GetLocationAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local);
	EndPos.Z += VerticalOffest;
	FVector EndTan = Spline->GetDirectionAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local) * TanScale;

	UPROPERTY() USplineMeshComponent *SplineMesh = NewObject<USplineMeshComponent>(this);
	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
	SplineMesh->SetStaticMesh(PathMesh);
	SplineMesh->RegisterComponentWithWorld(GetWorld());

	SplineMeshes.Add(SplineMesh);
}

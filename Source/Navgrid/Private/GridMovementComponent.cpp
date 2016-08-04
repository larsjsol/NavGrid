// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "GridPawn.h"
#include "GridMovementComponent.h"
#include "NavTileComponent.h"
#include "NavLadderComponent.h"

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
	if (NavGridItr)
	{
		Grid = *NavGridItr;
	}
	else
	{
		UE_LOG(NavGrid, Fatal, TEXT("%st: Unable to get reference to Navgrid."), *GetName());
	}
}

void UGridMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Moving)
	{
		/* Find the next location */
		Distance = FMath::Min(Spline->GetSplineLength(), Distance + (MaxSpeed * DeltaTime));
		
		/* Grab our current transform so we can find the velocity if we need it later */
		AActor *Owner = GetOwner();
		FTransform OldTransform = Owner->GetTransform();

		/* Find the next loaction from the spline*/
		FTransform NewTransform = Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);

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

bool UGridMovementComponent::CreatePath(UNavTileComponent &Target)
{
	Spline->ClearSplinePoints();
	SplineDistanceToTile.Empty();

	AGridPawn *Owner = Cast<AGridPawn>(GetOwner());
	Tile = Grid->GetTile(Owner->GetActorLocation());
	if (!Tile) 
	{ 
		UE_LOG(NavGrid, Error, TEXT("%s: Not on grid"), *Owner->GetName());
		return false;
	}

	TArray<UNavTileComponent *> InRange;
	Grid->TilesInRange(Tile, InRange, MovementRange, true, Owner->CapsuleComponent);
	if (InRange.Contains(&Target))
	{
		// create a list of tiles from the destination to the starting point and reverse it
		TArray<UNavTileComponent *> Path;
		UNavTileComponent *Current = &Target;
		while (Current)
		{
			Path.Add(Current);
			Current = Current->Backpointer;
		}
		Algo::Reverse(Path);

		// first add a spline point for the pawn starting location
		Spline->AddSplinePoint(GetOwner()->GetActorLocation(), ESplineCoordinateSpace::Local);
		SetTileAtInterval(Tile, 0, Spline->GetSplineLength());

		// Add spline points for the tiles in the path
		for (int32 Idx = 1; Idx < Path.Num(); Idx++)
		{
			float From = Spline->GetSplineLength();
			Path[Idx]->AddSplinePoints(Path[Idx - 1]->GetComponentLocation(), *Spline, Idx == Path.Num() - 1);
			SetTileAtInterval(Path[Idx], From, Spline->GetSplineLength());
		}

		/*	remove spline points that are closer together that 50uu
			this seems to prevent som wierd artifacts
		*/
		float PrevDistance = -100;
		int32 SplinePoint = 0;
		while (SplinePoint < Spline->GetNumberOfSplinePoints())
		{
			float Distance = Spline->GetDistanceAlongSplineAtSplinePoint(SplinePoint);
			if (Distance - PrevDistance < 50)
			{
				Spline->RemoveSplinePoint(SplinePoint);
			}
			else
			{
				SplinePoint++;
			}

			PrevDistance = Distance;
		}

		return true; // success!
	}
	else
	{
		return false; // no path to Target
	}
}

void UGridMovementComponent::FollowPath()
{
	/* Set the Moving flag, the real work is done in TickComponent() */
	Moving = true;
}

bool UGridMovementComponent::MoveTo(UNavTileComponent &Target)
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

		// Set up direction
		FVector PrevUp = FVector::ZeroVector;
		for (int32 Idx = 0; Idx < SplineMeshes.Num(); Idx++)
		{
			USplineMeshComponent *SplineMesh = SplineMeshes[Idx];
			UNavTileComponent *Tile = GetTileAtDistance(MeshLength * Idx);
			FVector UpDir = Tile->GetSplineMeshUpVector();
			/* try to average it out in order to avoid issues in sharp vertical turns */
			SplineMesh->SetSplineUpDir((UpDir + PrevUp) / 2);
			PrevUp = UpDir;
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
	StartPos.Z += VerticalOffset;
	FVector StartTan = Spline->GetDirectionAtDistanceAlongSpline(From, ESplineCoordinateSpace::Local) * TanScale;
	FVector EndPos = Spline->GetLocationAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local);
	EndPos.Z += VerticalOffset;
	FVector EndTan = Spline->GetDirectionAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local) * TanScale;

	UPROPERTY() USplineMeshComponent *SplineMesh = NewObject<USplineMeshComponent>(this);

	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
	SplineMesh->SetStaticMesh(PathMesh);
	SplineMesh->RegisterComponentWithWorld(GetWorld());

	//UNavTileComponent *Tile = GetTileAtDistance((From + To) / 2);
	//SplineMesh->SetSplineUpDir(Tile->GetSplineMeshUpVector());
	SplineMeshes.Add(SplineMesh);
}

UNavTileComponent *UGridMovementComponent::GetTileAtDistance(float Distance)
{
	if (Distance >= 0 && (int32) Distance < SplineDistanceToTile.Num())
	{
		return SplineDistanceToTile[Distance];
	}
	else
	{
		UE_LOG(NavGrid, Warning, TEXT("GetTileAtDistance(): No tile at distance %f"), Distance);
		return NULL;
	}
}

void UGridMovementComponent::SetTileAtInterval(UNavTileComponent *Tile, float From, float To)
{
	
	/* Make sure to fill earlier slots with this tile if they are not already set */
	From = FMath::Min(SplineDistanceToTile.Num(), (int32) From);

	/* Make sure our array is large enough */
	if (To + 1 >= SplineDistanceToTile.Num())
	{ 
		SplineDistanceToTile.SetNum(To + 1);
	}

	/* Set all tiles in range To - From inclusive so we later can look them up by saying SplineDistanceToTile[Distance]*/
	for (int32 Idx = From; Idx <= To; Idx++)
	{
		SplineDistanceToTile[Idx] = Tile;
	}
}

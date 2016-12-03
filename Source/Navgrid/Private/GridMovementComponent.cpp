// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Animation/AnimInstance.h"

UGridMovementComponent::UGridMovementComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	Spline = ObjectInitializer.CreateDefaultSubobject<USplineComponent>(this, "PathSpline");
	PathMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Path.NavGrid_Path'")).Object;
	
	AvailableMovementModes.Add(EGridMovementMode::ClimbingDown);
	AvailableMovementModes.Add(EGridMovementMode::ClimbingUp);
	AvailableMovementModes.Add(EGridMovementMode::Stationary);
	AvailableMovementModes.Add(EGridMovementMode::Walking);
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
		UE_LOG(NavGrid, Fatal, TEXT("%s: Unable to get reference to Navgrid."), *GetName());
	}

	/* Grab a reference to (a) AnimInstace */
	for (UActorComponent *Comp : GetOwner()->GetComponentsByClass(USkeletalMeshComponent::StaticClass()))
	{
		USkeletalMeshComponent *Mesh = Cast<USkeletalMeshComponent>(Comp);	
		if (Mesh)
		{
			AnimInstance = Mesh->GetAnimInstance();
			if (AnimInstance)
			{
				break;
			}
		}
	}
	if (!AnimInstance)
	{
		UE_LOG(NavGrid, Error, TEXT("%s: Unable to get reference to AnimInstance"), *GetName());
	}
}

void UGridMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/* Update movement mode and make a broadcast if it has changed */
	EGridMovementMode NewMovementMode = GetMovementMode();
	if (NewMovementMode != MovementMode)
	{
		OnMovementModeChangedEvent.Broadcast(MovementMode, NewMovementMode);
		MovementMode = NewMovementMode;
	}

	if (Moving)
	{

		/* Check if we can get the speed from root motion */
		float CurrentSpeed = 0;
		if (bUseRootMotion)
		{
			CurrentSpeed = ConsumeRootMotion().GetLocation().Size();
		}
		/* Root motion is either not available or there is really little movement, use manually set values */
		if (CurrentSpeed < 25 * DeltaTime)
		{
			if (MovementMode == EGridMovementMode::ClimbingDown || MovementMode == EGridMovementMode::ClimbingUp)
			{
				CurrentSpeed = MaxClimbSpeed * DeltaTime;
			}
			else
			{
				CurrentSpeed = MaxWalkSpeed * DeltaTime;
			}
		}
		Distance = FMath::Min(Spline->GetSplineLength(), Distance + CurrentSpeed);
		
		/* Grab our current transform so we can find the velocity if we need it later */
		AActor *Owner = GetOwner();
		FTransform OldTransform = Owner->GetTransform();

		/* Find the next location and rotation from the spline*/
		FTransform NewTransform = Spline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::Local);
		FRotator DesiredRotation;

		/* Restrain rotation axis if we're walking */
		if (MovementMode == EGridMovementMode::Walking)
		{
			DesiredRotation = NewTransform.Rotator();
			DesiredRotation.Roll = LockRoll ? 0 : DesiredRotation.Roll;
			DesiredRotation.Pitch = LockPitch ? 0 : DesiredRotation.Pitch;
			DesiredRotation.Yaw = LockYaw ? 0 : DesiredRotation.Yaw;
		}
		/* Use the rotation from the ladder if we're climbing */
		else if (MovementMode == EGridMovementMode::ClimbingUp || MovementMode == EGridMovementMode::ClimbingDown)
		{
			UNavTileComponent *CurrentTile = Grid->GetTile(PawnOwner->GetActorLocation(), false);
			if (CurrentTile)
			{
				DesiredRotation = CurrentTile->GetComponentRotation();
				DesiredRotation.Yaw -= 180;
			}
			else 
			// Dont update the rotation if we have no idea of what it should be
			// This to prevent the occational stuttering at the start of a descent
			{
				DesiredRotation = GetOwner()->GetActorRotation();
			}
		}

		/* Find the new rotation by limiting DesiredRotation by MaxRotationSpeed */
		FRotator NewRotation = LimitRotation(OldTransform.GetRotation().Rotator(), DesiredRotation, DeltaTime);

		NewTransform.SetRotation(NewRotation.Quaternion());
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

	AGridPawn *Owner = Cast<AGridPawn>(GetOwner());
	Tile = Grid->GetTile(Owner->GetActorLocation());
	if (!Tile) 
	{ 
		UE_LOG(NavGrid, Error, TEXT("%s: Not on grid"), *Owner->GetName());
		return false;
	}

	TArray<UNavTileComponent *> InRange;
	Grid->TilesInRange(Tile, InRange, Owner, true);
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

		// Estiamte how much of the spline that covers the first tile
		FVector Extent = Path[0]->GetScaledBoxExtent();
		float LengthOffset = FMath::Max3<float>(Extent.X, Extent.Y, Extent.Z);

		// first add a spline point for the pawn starting location
		Spline->AddSplinePoint(GetOwner()->GetActorLocation(), ESplineCoordinateSpace::Local);

		// Add spline points for the tiles in the path
		for (int32 Idx = 1; Idx < Path.Num(); Idx++)
		{
			float From = Spline->GetSplineLength();
			Path[Idx]->AddSplinePoints(Path[Idx - 1]->GetComponentLocation(), *Spline, Idx == Path.Num() - 1);
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

void UGridMovementComponent::PauseMoving()
{
	Moving = false;
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
		float PathDistance = HorizontalOffset; // Get some distance between the actor and the path
		FBoxSphereBounds Bounds = PathMesh->GetBounds();
		float MeshLength = FMath::Abs(Bounds.BoxExtent.X) * 2;
		float SplineLength = Spline->GetSplineLength();
		SplineLength -= HorizontalOffset; // Get some distance between the cursor and the path

		while (PathDistance < SplineLength)
		{
			AddSplineMesh(PathDistance, FMath::Min(PathDistance + MeshLength, SplineLength));
			PathDistance += FMath::Min(MeshLength, SplineLength - PathDistance);
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

FTransform UGridMovementComponent::ConsumeRootMotion()
{
	if (!AnimInstance)
	{
		return FTransform();
	}
	
	FRootMotionMovementParams RootMotionParams = AnimInstance->ConsumeExtractedRootMotion(1);
	return RootMotionParams.GetRootMotionTransform();
}

EGridMovementMode UGridMovementComponent::GetMovementMode()
{
	if (!Moving)
	{
		return EGridMovementMode::Stationary;
	}

	FVector ForwardLoc = GetForwardLocation(50);
	ForwardLoc -= GetOwner()->GetActorLocation();
	ForwardLoc = ForwardLoc.GetSafeNormal();
	float UpAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(ForwardLoc, FVector(0, 0, 1))));

	if (UpAngle < 90 - MaxWalkAngle)
	{
		return EGridMovementMode::ClimbingUp;
	}
	else if (UpAngle > 90 + MaxWalkAngle)
	{
		return EGridMovementMode::ClimbingDown;
	}
	else
	{
		return EGridMovementMode::Walking;
	}
}

FVector UGridMovementComponent::GetForwardLocation(float ForwardDistance)
{
	float D = FMath::Min(Spline->GetSplineLength(), Distance + ForwardDistance);
	return Spline->GetLocationAtDistanceAlongSpline(D, ESplineCoordinateSpace::Local);
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
	FVector UpVector = EndPos - StartPos;
	UpVector = FVector(UpVector.Y, UpVector.Z, UpVector.X);

	UPROPERTY() USplineMeshComponent *SplineMesh = NewObject<USplineMeshComponent>(this);
	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
	SplineMesh->SetStaticMesh(PathMesh);
	SplineMesh->RegisterComponentWithWorld(GetWorld());
	SplineMesh->SetSplineUpDir(UpVector);
	SplineMeshes.Add(SplineMesh);
}

FRotator UGridMovementComponent::LimitRotation(const FRotator &OldRotation, const FRotator &NewRotation, float DeltaTime)
{
	FRotator Result = OldRotation.GetNormalized();
	FRotator DeltaRotation = NewRotation - OldRotation;
	DeltaRotation.Normalize();
	Result.Pitch += DeltaRotation.Pitch > 0 ? FMath::Min<float>(DeltaRotation.Pitch, MaxRotationSpeed * DeltaTime) :
		FMath::Max<float>(DeltaRotation.Pitch, MaxRotationSpeed * -DeltaTime);
	Result.Roll += DeltaRotation.Roll > 0 ? FMath::Min<float>(DeltaRotation.Roll, MaxRotationSpeed * DeltaTime) :
		FMath::Max<float>(DeltaRotation.Roll, MaxRotationSpeed * -DeltaTime);
	Result.Yaw += DeltaRotation.Yaw > 0 ? FMath::Min<float>(DeltaRotation.Yaw, MaxRotationSpeed * DeltaTime) :
		FMath::Max<float>(DeltaRotation.Yaw, MaxRotationSpeed * -DeltaTime);

	return Result;
}

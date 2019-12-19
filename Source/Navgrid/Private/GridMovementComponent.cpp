// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Animation/AnimInstance.h"

FPathSegment::FPathSegment(TSet<EGridMovementMode> InMovementModes, float InStart, float InEnd)
{
	MovementModes = InMovementModes;
	Start = InStart;
	End = InEnd;
}

UGridMovementComponent::UGridMovementComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	PathMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Path.NavGrid_Path'")).Object;

	AvailableMovementModes.Add(EGridMovementMode::ClimbingDown);
	AvailableMovementModes.Add(EGridMovementMode::ClimbingUp);
	AvailableMovementModes.Add(EGridMovementMode::Stationary);
	AvailableMovementModes.Add(EGridMovementMode::Walking);
	AvailableMovementModes.Add(EGridMovementMode::InPlaceTurn);

	Distance = 0;
	MovementMode = EGridMovementMode::Stationary;
	MovementPhase = EGridMovementPhase::Done;
}

void UGridMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	/* I dont know why, but if we use createdefaultsubobject in the constructor this is sometimes reset to NULL*/
	if (!IsValid(Spline))
	{
		Spline = NewObject<USplineComponent>(this, "PathSpline");
		check(Spline);
	}

	Grid = ANavGrid::GetNavGrid(GetWorld());
	if (!Grid)
	{
		UE_LOG(NavGrid, Error, TEXT("%s was unable to find a NavGrid in level"), *this->GetName());
	}

	/* Grab a reference to (a) AnimInstace */
	for (UActorComponent *Comp : GetOwner()->GetComponentsByClass(USkeletalMeshComponent::StaticClass()))
	{
		USkeletalMeshComponent *Mesh = Cast<USkeletalMeshComponent>(Comp);
		if (Mesh)
		{
			MeshRotation = Mesh->GetRelativeTransform().Rotator();
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
		bUseRootMotion = false;
		bAlwaysUseRootMotion = false;
	}
}

void UGridMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// if we are moving, find the current path segment
	if (MovementMode == EGridMovementMode::Walking ||
		MovementMode == EGridMovementMode::ClimbingDown ||
		MovementMode == EGridMovementMode::ClimbingUp)
	{
		CurrentPathSegment = FPathSegment({ EGridMovementMode::Walking }, 0, Spline->GetSplineLength());
		for (FPathSegment &Segment : PathSegments)
		{
			if (Distance >= Segment.Start && Distance <= Segment.End)
			{
				CurrentPathSegment = Segment;
				break;
			}
		}
	}

	AActor *Owner = GetOwner();
	FTransform NewTransform = Owner->GetActorTransform();
	FRotator ActorRotation = Owner->GetActorRotation();

	ConsiderUpdateMovementMode();
	switch (MovementMode)
	{
	default:
	case EGridMovementMode::Stationary:
		if (bAlwaysUseRootMotion && MovementPhase != EGridMovementPhase::Ending)
		{
			FTransform RootMotion = ConsumeRootMotion();
			NewTransform.SetRotation(NewTransform.GetRotation() * RootMotion.GetRotation());
			FRotator AnimToWorld = Owner->GetActorRotation() + MeshRotation;
			NewTransform.SetLocation(NewTransform.GetLocation() + AnimToWorld.RotateVector(RootMotion.GetLocation()));

		}
		break; //nothing to do
	case EGridMovementMode::InPlaceTurn:
		NewTransform = TransformFromRotation(DeltaTime);
		break;
	case EGridMovementMode::Walking:
	case EGridMovementMode::ClimbingDown:
	case EGridMovementMode::ClimbingUp:
		NewTransform = TransformFromPath(DeltaTime);
		ConsiderUpdateCurrentTile();
		break;
	}

	/* reset rotation if we have any rotation locks */
	FRotator NewRotation = ApplyRotationLocks(NewTransform.GetRotation().Rotator());
	NewTransform.SetRotation(NewRotation.Quaternion());
	/* never change the scale */
	NewTransform.SetScale3D(Owner->GetActorScale3D());

	// update velocity so it can be fetched by the pawn
	Velocity = (NewTransform.GetLocation() - Owner->GetActorLocation()) * (1 / DeltaTime);
	UpdateComponentVelocity();
	// actually move the the actor
	Owner->SetActorTransform(NewTransform);

	if (MovementPhase == EGridMovementPhase::Beginning)
	{
		MovementPhase = EGridMovementPhase::Middle;
	}
	else if (MovementPhase == EGridMovementPhase::Ending && Velocity.Size() < 25)
	{
		OnMovementEndEvent.Broadcast();
		MovementPhase = EGridMovementPhase::Done;
	}
}

void UGridMovementComponent::StopMovementImmediately()
{
	ChangeMovementMode(EGridMovementMode::Stationary);
	MovementPhase = EGridMovementPhase::Done;
	Distance = 0;
	if (IsValid(Spline))
	{
		Spline->ClearSplinePoints();
	}
}

FTransform UGridMovementComponent::TransformFromPath(float DeltaTime)
{
	/* Check if we can get the speed from root motion */
	float CurrentSpeed = 0;
	if (bUseRootMotion)
	{
		FTransform RootMotion = ConsumeRootMotion();
		CurrentSpeed = RootMotion.GetLocation().Size();

		/* adjust the speed if we know the length and root motion in the ending animation */
		if (StoppingDistance > 0 && StoppingTime > 0 && EGridMovementPhase::Ending == MovementPhase)
		{
			CurrentSpeed = FMath::Max(DeltaTime * StoppingDistance / StoppingTime, CurrentSpeed);
		}
	}

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
	}
	/* Use the rotation from the ladder if we're climbing */
	else if (MovementMode == EGridMovementMode::ClimbingUp || MovementMode == EGridMovementMode::ClimbingDown)
	{
		DesiredRotation = CurrentPathSegment.PawnRotationHint;
	}

	/* Find the new rotation by limiting DesiredRotation by MaxRotationSpeed */
	FRotator NewRotation = LimitRotation(OldTransform.GetRotation().Rotator(), DesiredRotation, DeltaTime);
	NewTransform.SetRotation(NewRotation.Quaternion());

	/* Check if we are in the end phase of this movement mode */
	if (Distance + StoppingDistance >= Spline->GetSplineLength())
	{
		MovementPhase = EGridMovementPhase::Ending;
	}
	/* clear path if we have traversed it all (we might not get here if StoppingDistance is set) */
	if (CurrentSpeed == 0 || Distance >= Spline->GetSplineLength())
	{
		ChangeMovementMode(EGridMovementMode::Stationary);
		MovementPhase = EGridMovementPhase::Done;
		Distance = 0;
		Spline->ClearSplinePoints();
	}

	return NewTransform;
}

FTransform UGridMovementComponent::TransformFromRotation(float DeltaTime)
{
	AActor *Owner = GetOwner();
	FTransform NewTransform = Owner->GetActorTransform();
	if (Owner->GetActorRotation().Equals(DesiredForwardRotation))
	{
		ChangeMovementMode(EGridMovementMode::Stationary);
	}
	else
	{
		FRotator NewRotation = LimitRotation(Owner->GetActorRotation(), DesiredForwardRotation, DeltaTime);
		NewTransform.SetRotation(NewRotation.Quaternion());
	}
	return NewTransform;
}

void UGridMovementComponent::ConsiderUpdateCurrentTile()
{
	// try to grab the tile we're on and store it in CurrentTile. Take care to not overwrite a pointer to an
	// actual tile with NULL as that would mean we have moved off the grid
	if (IsValid(Grid))
	{

		UNavTileComponent *Tile = Grid->GetTile(GetOwner()->GetActorLocation(), true);
		if (!Tile &&
			(AvailableMovementModes.Contains(EGridMovementMode::ClimbingDown) ||
				AvailableMovementModes.Contains(EGridMovementMode::ClimbingUp)))
		{
			Tile = Grid->GetTile(GetOwner()->GetActorLocation(), false);
		}

		if (IsValid(Tile) && Tile != CurrentTile)
		{
			CurrentTile = Tile;

			ANavGridGameState *GameState = Cast<ANavGridGameState>(UGameplayStatics::GetGameState(GetOwner()));
			if (IsValid(GameState))
			{
				AGridPawn *GridPawn = Cast<AGridPawn>(GetOwner());
				GameState->OnPawnEnterTile().Broadcast(GridPawn, CurrentTile);
			}
		}
	}
}

void UGridMovementComponent::GetTilesInRange(TArray<UNavTileComponent *> &OutTiles)
{
	if (IsValid(Grid))
	{
		ConsiderUpdateCurrentTile();
		Grid->GetTilesInRange(Cast<AGridPawn>(GetOwner()), OutTiles);
	}
}

UNavTileComponent *UGridMovementComponent::GetTile()
{
	return CurrentTile;
}

ANavGrid * UGridMovementComponent::GetNavGrid()
{
	if (!IsValid(Grid))
	{
		Grid = ANavGrid::GetNavGrid(GetOwner());
	}
	check(Grid);
	return Grid;
}

void UGridMovementComponent::StringPull(TArray<const UNavTileComponent*>& InPath, TArray<const UNavTileComponent*>& OutPath)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UGridMovementComponent_StringPull);

	if (InPath.Num() > 2)
	{
		AGridPawn *GridPawnOwner = Cast<AGridPawn>(GetOwner());
		OutPath.Empty();
		const UCapsuleComponent &Capsule = *GridPawnOwner->MovementCollisionCapsule;
		int32 CurrentIdx = 0;
		OutPath.Add(InPath[0]);
		for (int32 Idx = 1; Idx < InPath.Num() - 1; Idx++)
		{
			// keep points needed to get around chasms and obstacles
			FVector Delta = InPath[Idx]->GetPawnLocation() - InPath[CurrentIdx]->GetPawnLocation();
			if (FMath::Abs(Delta.Rotation().Pitch) > MaxWalkAngle ||
				FMath::Abs(Delta.Z) > Capsule.RelativeLocation.Z - Capsule.GetScaledCapsuleHalfHeight() ||
				InPath[Idx]->Obstructed(InPath[CurrentIdx]->GetPawnLocation(), Capsule))
			{
				OutPath.AddUnique(InPath[Idx - 1]);
				CurrentIdx = Idx - 1;
			}
			// dont stringpull ladders
			else if (Cast<const UNavLadderComponent>(InPath[Idx]))
			{
				OutPath.AddUnique(InPath[Idx - 1]);
				OutPath.AddUnique(InPath[Idx]);
				if (Idx + 1 < InPath.Num())
				{
					OutPath.AddUnique(InPath[Idx + 1]);
				}
				CurrentIdx = Idx + 1;
				Idx = Idx + 1;
			}
		}
		OutPath.AddUnique(InPath[InPath.Num() - 2]);
		OutPath.AddUnique(InPath[InPath.Num() - 1]);
	}
	else
	{
		OutPath = InPath;
	}
}

bool UGridMovementComponent::CreatePath(const UNavTileComponent &Target)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UGridMovementComponent_CreatePath);
	AGridPawn *Owner = Cast<AGridPawn>(GetOwner());

	if (!IsValid(CurrentTile))
	{
		UE_LOG(NavGrid, Error, TEXT("%s: Not on grid"), *Owner->GetName());
		return false;
	}

	TArray<UNavTileComponent *> InRange;
	Grid->GetTilesInRange(Cast<AGridPawn>(GetOwner()), InRange);
	if (InRange.Contains(&Target))
	{
		// create a list of tiles from the destination to the starting point and reverse it
		TArray<const UNavTileComponent *> Path;
		const UNavTileComponent *Current = &Target;
		while (Current)
		{
			Path.Add(Current);
			Current = Current->Backpointer;
		}
		if (bStringPullPath)
		{
			TArray<const UNavTileComponent *> StringPulledPath;
			StringPull(Path, StringPulledPath);
			Path = StringPulledPath;
		}
		Algo::Reverse(Path);

		// Build the path spline and path segments
		Spline->ClearSplinePoints();
		PathSegments.Empty();
		if (Path.Num() > 1)
		{
			FVector ActorLocation = GetOwner()->GetActorLocation();
			// use the actor location inststead of the tile location for the first spline point
			Spline->AddSplinePoint(ActorLocation, ESplineCoordinateSpace::Local);
			Spline->SetSplinePointType(0, ESplinePointType::Linear, false);

			for (int32 Idx = 1; Idx < Path.Num(); Idx++)
			{
				if (Grid->GetTile(ActorLocation) != Path[Idx] && Owner->GetTile() != Path[Idx])
				{
					Path[Idx]->AddPathSegments(*Spline, PathSegments, Idx == Path.Num() - 1);
				}
			}
			return true; // success!
		}
	}

	return false; // no path to TargetTile
}

bool UGridMovementComponent::MoveTo(const UNavTileComponent &Target)
{
	bool PathExists = CreatePath(Target);
	if (PathExists)
	{
		ChangeMovementMode(EGridMovementMode::Walking);
	}
	return PathExists;
}

void UGridMovementComponent::TurnTo(const FRotator & Forward)
{
	if (AvailableMovementModes.Contains(EGridMovementMode::InPlaceTurn))
	{
		DesiredForwardRotation = Forward;
		ChangeMovementMode(EGridMovementMode::InPlaceTurn);
	}
}

void UGridMovementComponent::SnapToGrid()
{
	AGridPawn *GridPawnOwner = Cast<AGridPawn>(GetOwner());
	check(GridPawnOwner);

	ConsiderUpdateCurrentTile();

	// move owner to the tile's pawn location
	if (IsValid(CurrentTile))
	{
		GridPawnOwner->SetActorLocation(CurrentTile->GetPawnLocation());
	}
}

float UGridMovementComponent::GetRemainingDistance()
{
	return FMath::Max(Spline->GetSplineLength() - Distance, 0.0f);
}

FRotator UGridMovementComponent::ApplyRotationLocks(const FRotator & InRotation)
{
	FRotator OwnerRot = GetOwner()->GetActorRotation();
	FRotator Ret;
	Ret.Pitch = LockPitch ? OwnerRot.Pitch : InRotation.Pitch;
	Ret.Roll = LockRoll ? OwnerRot.Roll : InRotation.Roll;
	Ret.Yaw = LockYaw ? OwnerRot.Yaw : InRotation.Yaw;
	return Ret;
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


void UGridMovementComponent::ConsiderUpdateMovementMode()
{
	// only consider changing mode when we are moving
	if (MovementMode == EGridMovementMode::Walking ||
		MovementMode == EGridMovementMode::ClimbingDown ||
		MovementMode == EGridMovementMode::ClimbingUp)
	{
		// if the maching segment is not walkable, transtion to the a climbing mode
		if (!CurrentPathSegment.MovementModes.Contains(EGridMovementMode::Walking))
		{
			// try to get the tile the pawn will occupy when it moves a bit further
			FVector ForwardPoint = GetForwardLocation(50);
			FVector ActorLocation = GetOwner()->GetActorLocation();
			if (ForwardPoint.Z > ActorLocation.Z)
			{
				ChangeMovementMode(EGridMovementMode::ClimbingUp);
			}
			else
			{
				ChangeMovementMode(EGridMovementMode::ClimbingDown);
			}
		}
		// default movement mode is walking
		else
		{
			ChangeMovementMode(EGridMovementMode::Walking);
		}
	}
}

void UGridMovementComponent::ChangeMovementMode(EGridMovementMode NewMode)
{
	if (NewMode != MovementMode)
	{
		OnMovementModeChangedEvent.Broadcast(MovementMode, NewMode);
		MovementMode = NewMode;
		if (MovementMode != EGridMovementMode::Stationary)
		{
			MovementPhase = EGridMovementPhase::Beginning;
		}
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
	StartPos.Z += Grid->UIOffset;
	FVector StartTan = Spline->GetDirectionAtDistanceAlongSpline(From, ESplineCoordinateSpace::Local) * TanScale;
	FVector EndPos = Spline->GetLocationAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local);
	EndPos.Z += Grid->UIOffset;
	FVector EndTan = Spline->GetDirectionAtDistanceAlongSpline(To, ESplineCoordinateSpace::Local) * TanScale;
	FVector UpVector = EndPos - StartPos;
	UpVector = FVector(UpVector.Y, UpVector.Z, UpVector.X);

	UPROPERTY() USplineMeshComponent *SplineMesh = NewObject<USplineMeshComponent>(this);
	SplineMesh->SetMobility(EComponentMobility::Movable);
	SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
	SplineMesh->SetStaticMesh(PathMesh);
	SplineMesh->RegisterComponentWithWorld(GetWorld());
	SplineMesh->SetSplineUpDir(UpVector);
	SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

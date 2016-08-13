// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "GridMovementComponent.generated.h"

class ANavGrid;
class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;
class UNavTileComponent;

UENUM(BlueprintType)
enum class EGridMovementMode : uint8
{
	Stationary		UMETA(DisplayName = "Stationary"),
	Walking			UMETA(DisplayName = "Walking"),
	ClimbingUp 		UMETA(DisplayName = "Climbing up"),
	ClimbingDown	UMETA(DisplayName = "Climbing down")
};


/**
 * A movement component that operates on a NavGrid
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class NAVGRID_API UGridMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
public:
	UGridMovementComponent(const FObjectInitializer &ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	/* bound to the first NavGrid found in the level */
	UPROPERTY(BlueprintReadOnly, Category = "Movement") ANavGrid *Grid = NULL;
	/* How far (in tile cost) the actor can move in one go */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement") float MovementRange = 4;
	/* How fast can the actor move when walking*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement") float MaxWalkSpeed = 450;
	/* How fast can the actor move when climbing */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement") float MaxClimbSpeed = 200;
	/* How fast can the actor turn */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement") float MaxRotationSpeed = 1080;
	/* Steepest slope the actor can walk up or down */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement") float MaxWalkAngle = 45;
	/* MovementModes usable for this Pawn */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement") TArray<EGridMovementMode> AvailableMovementModes;
	/* Should we ignore rotation over the X axis */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement") bool LockRoll = true;
	/* Should we ignore rotation over the Y axis */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement") bool LockPitch = true;
	/* Should we ignore rotation over the Z axis */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement") bool LockYaw = false;
	/* Should we use root motion for speed */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Movement") bool bUseRootMotion = true;
	/*
	Spline that is used as a path. The points are in word coords.
	
	We use ESplineCoordinateSpace::Local in the getters and setters to avoid any extra coord translation
	*/
	UPROPERTY(BlueprintReadOnly, Category = "Visualization") USplineComponent *Spline = NULL;
	/* Mesh used to visualize the path */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization") UStaticMesh *PathMesh = NULL;
	/* Vertical offset between Path and visualization */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization") float VerticalOffset = 50;
	/* Distance between actor and where we start showing the path */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization") float HorizontalOffset = 87.5;

	/* Create a path to Target, return false if no path is found */
	bool CreatePath(UNavTileComponent &Target);
	/* Follow an existing path */
	void FollowPath();
	/* Temporarily stop moving, call FollowPath() to resume */
	void PauseMoving();
	/* Create a path and follow it if it exists */
	bool MoveTo(UNavTileComponent &Target);
	/* Visualize path */
	void ShowPath();
	/* Hide path */
	void HidePath();

	FTransform ConsumeRootMotion();

	EGridMovementMode GetMovementMode();
protected:
	EGridMovementMode MovementMode = EGridMovementMode::Stationary;
public:
	/* Return the point the the pawn will reach if it continues moving for ForwardDistance */
	FVector GetForwardLocation(float ForwardDistance);

	DECLARE_EVENT(UGridMovementComponent, FOnMovementDone);
	/* Triggered when movement ends */
	FOnMovementDone& OnMovementEnd() { return OnMovementEndEvent; }
private:
	FOnMovementDone OnMovementEndEvent;

public:
	DECLARE_EVENT_TwoParams(UGridMovementComponent, FOnMovementModeChanged, EGridMovementMode, EGridMovementMode);
	/* Triggered when the movement mode changes */
	FOnMovementModeChanged& OnMovementModeChanged() { return OnMovementModeChangedEvent; }
private:
	FOnMovementModeChanged OnMovementModeChangedEvent;

public:
	/* Should the actor be moved along the spline on the next Tick()? */
	bool Moving = false;

protected:
	UPROPERTY() TArray<USplineMeshComponent *> SplineMeshes;

	/* Helper: Puts a spline mesh in the range along the spline */
	void AddSplineMesh(float From, float To);

	/* How far along the spline are we */
	float Distance = 0;

	/* The tile we're currently on*/
	UNavTileComponent *Tile;

	UPROPERTY() UAnimInstance *AnimInstance;

	/* Return a delta FRotater that is within MaxRotationSpeed */
	FRotator LimitRotation(const FRotator &OldRotation, const FRotator &NewRotation, float DeltaTime);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/MovementComponent.h"
#include "GridMovementComponent.generated.h"

class ATile;
class ANavGrid;
class USplineComponent;
class USplineMeshComponent;
class UStaticMesh;

/**
 * 
 */
UCLASS(ClassGroup = Movement, meta = (BlueprintSpawnableComponent))
class BOARDGAME_API UGridMovementComponent : public UMovementComponent
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
	/* How fast can the actor move */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Movement") float MaxSpeed = 450;

	/* Spline used to visualize the path */
	UPROPERTY(BlueprintReadOnly, Category = "Visualization") USplineComponent *Spline = NULL;
	/* Mesh used to visualize the path */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization") UStaticMesh *PathMesh = NULL;
	/* Vertical offset between Path and visualization */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization") float VerticalOffest = 30;
	/* Distance between actor and where we start showing the path */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Visualization") float HorizontalOffset = 30;

	/* Create a path to Target, return false if no path is found */
	bool CreatePath(const ATile &Target);
	/* Follow an existing path */
	void FollowPath();
	/* Create a path and follow it if it exists */
	bool MoveTo(const ATile &Target);
	/* Visualize path */
	void ShowPath();
	/* Hide path */
	void HidePath();

protected:
	UPROPERTY() TArray<USplineMeshComponent *> SplineMeshes;

	/* Helper: Puts a spline mesh in the range along the spline */
	void AddSplineMesh(float From, float To);

	/* Should the actor be moved along the spline on the next Tick()? */
	bool Moving = false;
	/* How far along the spline are we */
	float Distance = 0;
};

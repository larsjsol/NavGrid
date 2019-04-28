// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"

#include "GridMovementComponent.h"
#include "GridPawn.generated.h"

class UTurnComponent;
class UCapsuleComponent;
class UArrowComponent;
class UNavTileComponent;
class ANavGrid;

UENUM()
enum class EGridPawnState : uint8
{
	/* it is not this pawns turn */
	WaitingForTurn	UMETA(DisplayName = "Waiting for turn"),
	/* Ready for player input */
	Ready			UMETA(DisplayName = "Ready"),
	/* Currently performing some sort of action and is not ready for player input */
	Busy			UMETA(DisplayName = "Busy"),
	/* Dead */
	Dead			UMETA(DisplayName = "Dead")
};

/** A pawn that can move around on a NavGrid. */
UCLASS()
class NAVGRID_API AGridPawn : public APawn, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AGridPawn();
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform &Transform) override;

	// IGenericTeamAgentInterface start
	virtual void SetGenericTeamId(const FGenericTeamId& InTeamId) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	// IGenericTeamAgentInterface end
protected:
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "NavGrid")
	FGenericTeamId TeamId;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	USceneComponent *SceneRoot;
	/** Bounding capsule.
	Used to check for collisions when spawning and for mouse over events.
	It should be adjusted so it envelops the entire mesh of the pawn.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCapsuleComponent *BoundsCapsule;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UGridMovementComponent *MovementComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UTurnComponent *TurnComponent;
	/** Collision capsule.
	Used to check if a pawn will collide with the environment if it moves into a tile.
	It should be slightly thinner than the pawn and its location along the z-axis determines the height
	of the obstacles the pawn can step over.
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UCapsuleComponent *MovementCollisionCapsule;
	/* Shown when the pawn is selected/has its turn */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UStaticMeshComponent *SelectedHighlight;
	/* An arrow pointing forward */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	UArrowComponent *Arrow;

	/* Should this pawn snap to grid at the start of each turn */
	UPROPERTY(EditAnyWhere)
	bool SnapToGrid = true;

	/* Callend on round start */
	UFUNCTION()
	virtual void OnRoundStart() {}
	/* Called on turn start for any component */
	UFUNCTION()
	virtual void OnAnyTurnStart(UTurnComponent *InTurnComponent);
	/* Called on turn start for this pawn */
	virtual void OnTurnStart();
	/* Called on turn end for any component */
	UFUNCTION()
	virtual void OnAnyTurnEnd(UTurnComponent *InTurnComponent);
	/* Called on turn end for this pawn */
	virtual void OnTurnEnd();
	/* Called when done moving */
	virtual void OnMoveEnd();
	/* Called when any component owner is ready for player or ai input */
	UFUNCTION()
	virtual void OnAnyPawnReadyForInput(UTurnComponent *InTurnComponent);
	/* Called when this pawn is ready for player or ai input */
	virtual void OnPawnReadyForInput() {}

	/* override this class and implement your own AI here. The default implementation just ends the turn */
	virtual void PlayAITurn();
	/* Get the current state for this pawn */
	UFUNCTION(BlueprintCallable)
	virtual EGridPawnState GetState() const;
	/* is this pawn controlled by a human player? */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	bool bHumanControlled;
	/* can the we request to start our turn now? The turn manager may still deny our request even if this returns true */
	virtual bool CanBeSelected();

	virtual bool CanMoveTo(const UNavTileComponent & Tile);
	virtual void MoveTo(const UNavTileComponent & Tile);

	/* get the tile occupied at the start of this pawns turn */
	UFUNCTION(BlueprintCallable)
	UNavTileComponent *GetTile() const { return MovementComponent->GetTile(); }
	template <class T>
	T *GetTile() const { return Cast<T>(GetTile()); }

	/* Place a tile under our feet if there is not alrady one there. Returne the tile under our feet */
	UNavTileComponent *ConsiderGenerateVirtualTile();
	template <class T>
	T *ConsiderGenerateVirtualTile() { return Cast<T>(ConsiderGenerateVirtualTile()); }

	void GenerateVirtualTiles();

	/* Called when the user clicks on this actor, default implementation is to change the the current turn taker to this */
	UFUNCTION()
	virtual void Clicked(AActor *ClickedActor, FKey PressedKey);

#if WITH_EDITORONLY_DATA
	void OnObjectSelectedInEditor(UObject *SelectedObject);

protected:
	UPROPERTY(EditAnyWhere)
	bool bPreviewTiles = false;
	UPROPERTY(EditAnyWhere)
	float PreviewTileSize = 200;
	FTimerHandle PreviewTimerHandle;
	void UpdatePreviewTiles();
	UPROPERTY(Transient)
	ANavGrid *PreviewGrid;
#endif // WITH_EDITORONLY_DATA
};

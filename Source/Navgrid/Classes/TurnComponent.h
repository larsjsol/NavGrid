// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "TurnManager.h"

#include "TurnComponent.generated.h"

/**
* Actors with a turn component can be managed by a turn manager
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class NAVGRID_API UTurnComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UTurnComponent();
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	ATurnManager *GetTurnManager() { return TurnManager; }
protected:
	UPROPERTY()
	ATurnManager *TurnManager;
	UFUNCTION()
	void OnTurnTimeout();
	FTimerHandle TurnTimeoutHandle;
public:
	/* The number of actions this pawn can perform in a single round */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	int32 StartingActionPoints;
	/* Remaining actions that this pawn can perform this round */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 RemainingActionPoints;

	/* end this turn after the given amount of time has passed. Set to 0 to disable */
	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite)
	float TurnTimeout;

	/* Tell the manager to end the turn for this component */
	void EndTurn();
	void EndTeamTurn();
	/* request the turn manager to start a turn for this component */
	void RequestStartTurn();
	/* request that the turn manager starts the turn for the next component on our team */
	void RequestStartNextComponent();

	/* Used be the owning actor to notify that it is ready to receive input from a player or ai */
	void OwnerReadyForInput();

	/* is it this components turn? */
	UFUNCTION(BlueprintPure)
	bool MyTurn() { return IsValid(TurnManager) && TurnManager->GetCurrentComponent() == this; }

	/* which team this component is a part of */
	UFUNCTION(BlueprintPure)
	FGenericTeamId TeamId() const { return FGenericTeamId::GetTeamIdentifier(GetOwner()); }

	UFUNCTION(BlueprintPure)
	AActor *GetCurrentActor() const;
	template <class T>
	T *GetCurrentActor() const { return Cast<T>(GetCurrentActor()); }

	// register with the turn manager in order to get to take turns
	void RegisterWithTurnManager();
	// unregister, this compnent will no longer get to take turns
	void UnregisterWithTurnManager();

	// called by the turn manager
	void OnTurnStart();
	void OnTurnEnd();
};
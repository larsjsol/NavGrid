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
public:
	/* The number of actions this pawn can perform in a single round */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	int32 StartingActionPoints;
	/* Remaining actions that this pawn can perform this round */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 RemainingActionPoints;

	/* Tell the manager that we are done acting for this round*/
	void EndTurn() { TurnManager->EndTurn(this); }
	void EndTeamTurn() { TurnManager->EndTeamTurn(FGenericTeamId::GetTeamIdentifier(GetOwner())); }
	/* request the turn manager to start a turn for this component */
	bool RequestStartTurn() { return TurnManager->RequestStartTurn(this); }
	/* request that the turn manager starts the turn for the next component on our team */
	bool RequestStartNextComponent() { return TurnManager->RequestStartNextComponent(this); }

	/* Used be the owning actor to notify that it is ready to receive input from a player or ai */
	void OwnerReadyForInput() { TurnManager->OnReadyForInput().Broadcast(this); }

	/* is it this components turn? */
	UFUNCTION(BlueprintPure)
	bool MyTurn() { return IsValid(TurnManager) && TurnManager->GetCurrentComponent() == this; }

	/* which team this component is a part of */
	UFUNCTION(BlueprintPure)
	FGenericTeamId TeamId() const { return FGenericTeamId::GetTeamIdentifier(GetOwner()); }
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "TurnManager.h"
#include "TurnComponent.generated.h"

class APlayerController;

/**
* Actors with a turn component can be managed by a turn manager
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class NAVGRID_API UTurnComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UTurnComponent();

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	//Event delegates
	DECLARE_MULTICAST_DELEGATE(FOnTurnStart);
	DECLARE_MULTICAST_DELEGATE(FOnTurnEnd);
	DECLARE_MULTICAST_DELEGATE(FOnRoundStart);
	DECLARE_MULTICAST_DELEGATE(FOnPawnReady);

	/* The Owners turn have started */
	FOnTurnStart& OnTurnStart() { return OnTurnStartDelegate; }
	/* The Owners turn have ended */
	FOnTurnEnd& OnTurnEnd() { return OnTurnEndDelegate; }
	/* All actors managed by the turn manager have had their turn and a new round begins */
	FOnRoundStart& OnRoundStart() { return OnRoundStartDelegate; }
	/* The pawn is ready for player input */
	FOnPawnReady& OnPawnReady() { return OnPawnReadyDelegate; }

	void SetTurnManager(ATurnManager *InTurnManager);
protected:
	UPROPERTY()
	ATurnManager *TurnManager;
public:
	/* The number of actions this pawn can perform in a single round */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Turn Component")
	int32 ActionPoints;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turn Component")
	int32 RemainingActionPoints;

	/* Tell the manager that we are done acting for this round*/
	void EndTurn();
	/* Tell the manager that the turn should pass to the next component*/
	void StartTurnNext();
	/* request the turn manager to start a turn for this component */
	bool RequestStartTurn() { return TurnManager->RequestStartTurn(this); }
	/* Callend when a turn starts */
	void TurnStart();
	/* Called when a turn ends */
	void TurnEnd();
	/* Called when a new round starts (i.e. everyone has acted and its time to start over) */
	void RoundStart();
	/* Broadcast that this component is ready for player input */
	void BroadcastReadyForPlayerInput();

	/* Remove the entry for this component in the turn manager */
	void UnRegister();

	/* is it this components turn? */
	UFUNCTION(BLueprintPure, Category = "Turn Component")
	inline bool MyTurn() { return bMyTurn; }

	/* which team this component is a part of */
	UFUNCTION(BLueprintPure, Category = "Turn Component")
	FGenericTeamId TeamId() const { return FGenericTeamId::GetTeamIdentifier(GetOwner()); }

protected:
	bool bMyTurn;

private:
	FOnTurnStart OnTurnStartDelegate;
	FOnTurnEnd OnTurnEndDelegate;
	FOnRoundStart OnRoundStartDelegate;
	FOnPawnReady OnPawnReadyDelegate;
};
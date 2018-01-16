// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "TurnComponent.generated.h"

class ATurnManager;

/**
* Actors with a turn component can be managed by a turn manager 
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class NAVGRID_API UTurnComponent : public UActorComponent
{
	GENERATED_BODY()
public:

	//Event delegates
	DECLARE_DELEGATE(FOnTurnStart);
	DECLARE_DELEGATE(FOnTurnEnd);
	DECLARE_DELEGATE(FOnRoundStart);

	/* The Owners turn have started */
	FOnTurnStart& OnTurnStart() { return OnTurnStartDelegate; }
	/* The Owners turn have ended */
	FOnTurnEnd& OnTurnEnd() { return OnTurnEndDelegate; }
	/* All actors managed by the turn manager have had their turn and a new round begins */
	FOnRoundStart& OnRoundStart() { return OnRoundStartDelegate; }

	void SetTurnManager(ATurnManager *InTurnManager);
protected:
	UPROPERTY()
	ATurnManager *TurnManager;
public:
	UPROPERTY(VisibleAnywhere, Category = "Turn Component")
	bool bCanStillActThisRound = true;

	/* Tell the manager that we are done acting for this round*/
	void EndTurn();
	/* Tell the manager that the turn should pass to the next component*/
	void StartTurnNext();
	/* Callend when a turn starts */
	void TurnStart();
	/* Called when a turn ends */
	void TurnEnd();
	/* Called when a new round starts (i.e. everyone has acted and its time to start over) */
	void RoundStart();

private:
	FOnTurnStart OnTurnStartDelegate;
	FOnTurnEnd OnTurnEndDelegate;
	FOnRoundStart OnRoundStartDelegate;
};
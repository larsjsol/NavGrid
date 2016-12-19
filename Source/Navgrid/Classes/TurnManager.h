// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TurnManager.generated.h"

class UTurnComponent;

/**
* Coordinates a set of turn components
*/
UCLASS()
class NAVGRID_API ATurnManager : public AActor
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	/* Start the first round */
	void StartFirstRound();
	/* Add a turn component to be managed */
	void Register(UTurnComponent *TurnComponent);
	/* End the turn for the current turn component */
	void EndTurn(UTurnComponent *Ender);
	/* Start turn for a component */
	void StartTurn(UTurnComponent *TurnComponent);
	/* Move on to the next component in line */
	void StartTurnNext();
	/* Return the component whos turn it is */
	UTurnComponent *GetCurrentComponent();

	//Event delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, UTurnComponent *, TurnComponent);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, UTurnComponent *, TurnComponent);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStart);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTurnStart OnTurnStart;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTurnEnd OnTurnEnd;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRoundStart OnRoundStart;

protected:
	UPROPERTY() TArray<UTurnComponent *> TurnComponents;
	UPROPERTY(VisibleAnyWhere, Category = "Turn Manager", meta = (AllowPrivateAccess = "true"))
	int32 ComponentIndex = 0;
	UPROPERTY(VisibleAnyWhere, Category = "Turn Manager", meta = (AllowPrivateAccess = "true"))
	int32 Round = 0;
	void ChangeCurrent(int32 NewIndex);
	// returns -1 if no components can act this round
	int32 GetNextIndexThatCanAct();
	void StartNewRound();
};
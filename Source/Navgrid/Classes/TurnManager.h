// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "TurnManager.generated.h"

class UTurnComponent;

//Declare delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyForPlayerInput, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundEnd);

/**
* Coordinates a set of turn components.
*
* Terms:
*  'Turn' is used for a singe pawn doing a single action.
*  'Round' is used for all pawns managed by this turn manager having their Turn.
*/
UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class NAVGRID_API ATurnManager : public AActor
{
	GENERATED_BODY()
public:
	ATurnManager();

	/* Add a turn component to be managed */
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	virtual void Register(UTurnComponent *TurnComponent);
	/* remove a component from this manager */
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	virtual void UnRegister(UTurnComponent *TurnComponent);
	/* End the turn for the current turn component */
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	virtual void EndTurn(UTurnComponent *Ender);
	/* Move on to the next component in line */
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	virtual void StartTurnNext();
	/* request to immediatly start the turn for the supplied component. Return false if the request is denied */
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	virtual bool RequestStartTurn(UTurnComponent *TurnComponent);
	/* Return the component whos turn it is */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Manager")
	UTurnComponent *GetCurrentComponent();
	/* Start the first round */
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	virtual void StartFirstRound();
	/* End the current round */
	UFUNCTION(BlueprintCallable, Category = "Turn Manager")
	virtual void EndRound();

	/* Get the turn manager for a given team, the default implementation just returns this */
	UFUNCTION(BlueprintPure, Category = "Turn Manager")
	virtual ATurnManager *GetTurnManager(const FGenericTeamId& TeamID) { return this; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Turn Manager")
	int32 GetRound() const { return Round; }

	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnTurnStart OnTurnStart;
	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnTurnEnd OnTurnEnd;
	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnReadyForPlayerInput OnReadyForPlayerInput;
	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnRoundStart OnRoundStart;
	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnRoundStart OnRoundEnd;

	/* wait for this long between each turn */
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Turn Manager")
	float TurnDelay;

protected:
	UPROPERTY()
	TArray<UTurnComponent *> TurnComponents;
	UPROPERTY(VisibleAnyWhere, Category = "Turn Manager", meta = (AllowPrivateAccess = "true"))
	int32 ComponentIndex = 0;
	UPROPERTY(VisibleAnyWhere, Category = "Turn Manager", meta = (AllowPrivateAccess = "true"))
	int32 Round = 0;
	void ChangeCurrent(int32 NewIndex);
	// returns -1 if no components can act this round
	int32 GetNextIndexThatCanAct();
	void StartNewRound();

	// start the turn for the current component
	void StartTurnCurrent();
	FTimerHandle TurnDelayHandle;
};
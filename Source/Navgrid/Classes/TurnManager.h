// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TurnManager.generated.h"

class UTurnComponent;

//Declare delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStart);

/**
* Coordinates a set of turn components.
*
* Terms:
*  'Turn' is used for a singe pawn doing one or more actions.
*  'Round' is used for all active pawns having their Turn. Note that a pawn may have several Turns in a Round, 
*  say if the player selects another pawn to go first.
*/
UCLASS(BlueprintType, Blueprintable, NotPlaceable)
class NAVGRID_API ATurnManager : public AActor
{
	GENERATED_BODY()
public:
	virtual void BeginPlay() override;

	/* Start the first round */
	UFUNCTION(BlueprintCallable, Category = "TurnManager")
	void StartFirstRound();
	/* Add a turn component to be managed */
	UFUNCTION(BlueprintCallable, Category = "TurnManager")
	void Register(UTurnComponent *TurnComponent);
	/* End the turn for the current turn component */
	UFUNCTION(BlueprintCallable, Category = "TurnManager")
	void EndTurn(UTurnComponent *Ender);
	/* Move on to the next component in line */
	UFUNCTION(BlueprintCallable, Category = "TurnManager")
	void StartTurnNext();
	/* Return the component whos turn it is */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TurnManager")
	UTurnComponent *GetCurrentComponent();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TurnManager")
	int32 GetRound() const { return Round; }

	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnTurnStart OnTurnStart;
	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnTurnEnd OnTurnEnd;
	UPROPERTY(BlueprintAssignable, Category = "Turn Manager")
	FOnRoundStart OnRoundStart;

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
};
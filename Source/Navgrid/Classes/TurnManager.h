// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TurnManager.generated.h"

class UTurnComponent;

/**
* Coordinates a set of turn components.
*
* Terms:
*  'Turn' is used for a singe pawn doing one or more actions.
*  'Round' is used for all active pawns having their Turn. Note that a pawn may have several Turns in a Round, 
*  say if the player selects another pawn to go first.
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
	/* Move on to the next component in line */
	void StartTurnNext();
	/* Return the component whos turn it is */
	UTurnComponent *GetCurrentComponent();

	//Declare events
	DECLARE_EVENT_OneParam(UTurnComponent, FOnTurnStart, UTurnComponent *);
	DECLARE_EVENT_OneParam(UTurnComponent, FOnTurnEnd, UTurnComponent *);
	DECLARE_EVENT(UTurnComponent, FOnRoundStart);

	FOnTurnStart& OnTurnStart() { return OnTurnStartEvent; }
	FOnTurnEnd& OnTurnEnd() { return OnTurnEndEvent; }
	FOnRoundStart& OnRoundStart() { return OnRoundStartEvent; }
private:
	FOnTurnStart OnTurnStartEvent;
	FOnTurnEnd OnTurnEndEvent;
	FOnRoundStart OnRoundStartEvent;

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
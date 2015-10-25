// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "TurnComponent.generated.h"


/**
* Actors with a turn component can be managed by a turn manager 
*/
UCLASS(meta = (BlueprintSpawnableComponent))
class NAVGRID_API UTurnComponent : public UActorComponent
{
	GENERATED_BODY()
public:

	//Event delegates
	DECLARE_EVENT(UTurnComponent, FOnTurnStart);
	DECLARE_EVENT(UTurnComponent, FOnTurnEnd);
	DECLARE_EVENT(UTurnComponent, FOnRoundStart);

	/* The Owners turn have started */
	FOnTurnStart& OnTurnStart() { return OnTurnStartEvent; }
	/* The Owners turn have ended */
	FOnTurnEnd& OnTurnEnd() { return OnTurnEndEvent; }
	/* All actors managed by the turn manager have had their turn and a new round begins */
	FOnRoundStart& OnRoundStart() { return OnRoundStartEvent; }

	void TurnStart();
	void TurnEnd();
	void RoundStart();
private:
	FOnTurnStart OnTurnStartEvent;
	FOnTurnEnd OnTurnEndEvent;
	FOnRoundStart OnRoundStartEvent;
};
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
	void Register(UTurnComponent *TurnComponent);
	void EndTurn(UTurnComponent *Ender);
	/* Return the component whos turn it is */
	UTurnComponent *GetCurrentComponent();

	//Event delegates
	DECLARE_EVENT_OneParam(ATurnManager, FOnTurnStart, const UTurnComponent&);
	DECLARE_EVENT_OneParam(ATurnManager, FOnTurnEnd, const UTurnComponent&);
	DECLARE_EVENT(ATurnManager, FOnRoundStart);
	
	/* Triggered on turn start for a component */
	FOnTurnStart& OnTurnStart() { return OnTurnStartEvent; }
	/* Triggered on turn end for a component */
	FOnTurnEnd& OnTurnEnd() { return OnTurnEndEvent; }
	/* Triggered when all components are done and a new round begins */
	FOnRoundStart& OnRoundEnd() { return OnRoundStartEvent; }

private:
	FOnTurnStart OnTurnStartEvent;
	FOnTurnEnd OnTurnEndEvent;
	FOnRoundStart OnRoundStartEvent;

protected:
	UPROPERTY() TArray<UTurnComponent *> TurnComponents;
	int32 ComponentIndex = 0;
};
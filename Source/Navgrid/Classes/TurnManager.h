// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "TurnManager.generated.h"

class UTurnComponent;

//Declare delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyForInput, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundEnd);

USTRUCT()
struct FTeam
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<UTurnComponent *> Components;

	bool HasComponentWaitingForTurn();
};

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
	virtual void BeginPlay() override;

	/* Add a turn component to be managed */
	UFUNCTION(BlueprintCallable)
	void Register(UTurnComponent *TurnComponent);

protected:
	void StartTurnForNextComponent();
	void StartTurn();

public:
	/* End the turn for the current turn component */
	UFUNCTION(BlueprintCallable)
	bool EndTurn(UTurnComponent *Ender);
	/* End the turn for all components of the current team */
	UFUNCTION(BlueprintCallable)
	bool EndTeamTurn(FGenericTeamId InTeamId);
	/* request to immediatly start the turn for the supplied component. Return false if the request is denied */
	UFUNCTION(BlueprintCallable)
	bool RequestStartTurn(UTurnComponent *CallingComponent);
	UFUNCTION(BlueprintCallable)
	bool RequestStartNextComponent(UTurnComponent *CallingComponent);
	/* Return the component whos turn it is */
	UFUNCTION(BlueprintPure)
	UTurnComponent *GetCurrentComponent();
	UFUNCTION(BlueprintPure)
	FGenericTeamId GetCurrentTeam() const { return CurrentTeam; }

	UFUNCTION(BlueprintPure)
	int32 GetRound() const { return Round; }

private:
	UPROPERTY(BlueprintAssignable)
	FOnTurnStart OnTurnStartDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnTurnEnd OnTurnEndDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnReadyForInput OnReadyForInputDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRoundStart OnRoundStartDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRoundEnd OnRoundEndDelegate;

public:
	virtual FOnTurnStart& OnTurnStart() { return OnTurnStartDelegate; }
	virtual FOnTurnEnd& OnTurnEnd() { return OnTurnEndDelegate; }
	virtual FOnReadyForInput& OnReadyForInput() { return OnReadyForInputDelegate; }
	virtual FOnRoundStart& OnRoundStart() { return OnRoundStartDelegate; }
	virtual FOnRoundEnd& OnRoundEnd() { return OnRoundEndDelegate; }

	/* wait for this long between each turn */
	UPROPERTY(BlueprintReadWrite, EditAnyWhere, Category = "Turn Manager")
	float TurnDelay;

protected:
	UPROPERTY()
	TMap<FGenericTeamId, FTeam> Teams;
	int32 CurrentComponent;
	FGenericTeamId CurrentTeam;
	int32 Round;

	FTimerHandle TurnDelayHandle;
};
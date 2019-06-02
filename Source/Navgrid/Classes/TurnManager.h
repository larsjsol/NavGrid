// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "TurnManager.generated.h"

class UTurnComponent;

//Declare delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnStart, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnEnd, UTurnComponent *, TurnComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamTurnStart, const FGenericTeamId &, TeamId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamTurnEnd, const FGenericTeamId &, TeamId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyForInput, UTurnComponent *, TurnComponent);
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
	virtual void Tick(float DeltaTime) override;

	/* Add a turn component to be managed */
	UFUNCTION(BlueprintCallable)
	void RegisterTurnComponent(UTurnComponent *TurnComponent);
	UFUNCTION(BlueprintCallable)
	void UnregisterTurnComponent(UTurnComponent *TurnComponent);

	/* minumuim number of teams needed to start a new turn */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	int32 MinNumberOfTeams;

	/* End the turn for the current turn component */
	UFUNCTION(BlueprintCallable)
	void EndTurn(UTurnComponent *Ender);
	/* End the turn for all components of the current team */
	UFUNCTION(BlueprintCallable)
	void EndTeamTurn(FGenericTeamId InTeamId);
	/* request to immediatly start the turn for the supplied component. Return false if the request is denied */
	UFUNCTION(BlueprintCallable)
	void RequestStartTurn(UTurnComponent *CallingComponent);
	UFUNCTION(BlueprintCallable)
	void RequestStartNextComponent(UTurnComponent *CallingComponent);
	UFUNCTION(BlueprintPure)
	UTurnComponent *GetCurrentComponent() const { return CurrentComponent; }
	UFUNCTION(BlueprintPure)
	FGenericTeamId GetCurrentTeam() const;

	AActor *GetCurrentActor() const;
	template <class T>
	T* GetCurrentActor() const { return Cast<T>(GetCurrentActor()); }

	UFUNCTION(BlueprintPure)
	int32 GetRound() const { return Round; }

protected:
	// find the next team member that can act this turn
	UTurnComponent *FindNextTeamMember(const FGenericTeamId &TeamId);
	UTurnComponent *FindNextComponent();
	bool HasComponentsThatCanAct();

private:
	UPROPERTY(BlueprintAssignable)
	FOnTurnStart OnTurnStartDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnTurnEnd OnTurnEndDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnTeamTurnStart OnTeamTurnStartDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnTeamTurnEnd OnTeamTurnEndDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnReadyForInput OnReadyForInputDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRoundStart OnRoundStartDelegate;
	UPROPERTY(BlueprintAssignable)
	FOnRoundEnd OnRoundEndDelegate;

public:
	virtual FOnTurnStart& OnTurnStart() { return OnTurnStartDelegate; }
	virtual FOnTurnEnd& OnTurnEnd() { return OnTurnEndDelegate; }
	virtual FOnTeamTurnStart& OnTeamTurnStart() { return OnTeamTurnStartDelegate; }
	virtual FOnTeamTurnEnd& OnTeamTurnEnd() { return OnTeamTurnEndDelegate; }
	virtual FOnReadyForInput& OnReadyForInput() { return OnReadyForInputDelegate; }
	virtual FOnRoundStart& OnRoundStart() { return OnRoundStartDelegate; }
	virtual FOnRoundEnd& OnRoundEnd() { return OnRoundEndDelegate; }

protected:
	UPROPERTY(VisibleAnyWhere)
	UTurnComponent *CurrentComponent;
	UPROPERTY()
	UTurnComponent *NextComponent;
	TMultiMap<FGenericTeamId, UTurnComponent *> Teams;
	UPROPERTY(VisibleAnyWhere)
	int32 Round;
	bool bStartNewTurn;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurnManager.h"
#include "TurnComponent.h"
#include "TeamTurnManager.generated.h"


/**
 *
 */
UCLASS()
class NAVGRID_API ATeamTurnManager : public ATurnManager
{
	GENERATED_BODY()

public:
	ATeamTurnManager();
	virtual void BeginPlay() override;

	virtual void Register(UTurnComponent *TurnComponent) override;
	virtual void StartTurnNext() override;
	virtual void EndRound() override;
	virtual bool RequestStartTurn(UTurnComponent *TurnComponent) override;

	virtual ATurnManager *GetTurnManager(const FGenericTeamId& TeamID) override;
	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category = "Turn Manager")
	bool Master;

	virtual bool MyTurn() override { return TurnComponent->MyTurn(); }
protected:
	UPROPERTY(VisibleAnyWhere, Category = "Turn Manager", meta = (AllowPrivateAccess = "true"))
	UTurnComponent *TurnComponent;

	UPROPERTY()
	TMap<uint8, ATeamTurnManager *> Teams;
	FGenericTeamId TeamId;
};

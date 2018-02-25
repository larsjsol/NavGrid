// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurnManager.h"
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
	virtual void Register(UTurnComponent *TurnComponent) override;
	virtual void StartTurnNext() override;
	virtual ATurnManager *GetTurnManager(int32 TeamId = 0) override;

	UPROPERTY(VisibleAnyWhere, BlueprintReadWrite, Category = "Turn Manager")
	bool Master;
	
protected:
	UPROPERTY(VisibleAnyWhere, Category = "Turn Manager", meta = (AllowPrivateAccess = "true"))
	UTurnComponent *TurnComponent;

	UPROPERTY()
	TMap<int32, ATeamTurnManager *> Teams;
};

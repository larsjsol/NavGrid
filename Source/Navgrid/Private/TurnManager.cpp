#include "NavGridPrivatePCH.h"

void ATurnManager::BeginPlay()
{
	Super::BeginPlay();
	StartNewRound();
}

void ATurnManager::Register(UTurnComponent *TurnComponent)
{
	TurnComponents.AddUnique(TurnComponent);
	TurnComponent->TurnManager = this;
}

void ATurnManager::EndTurn(UTurnComponent *Ender)
{
	if (Ender == TurnComponents[ComponentIndex])
	{
		StartTurnNext();
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("EndTurn() called unexpectedly by %s"), *Ender->GetName());
	}
}

void ATurnManager::StartTurn(UTurnComponent * TurnComponent)
{
	for (int32 Counter = 0; Counter < TurnComponents.Num(); Counter++)
	{
		int32 Idx = (ComponentIndex + Counter) % TurnComponents.Num();
		if (TurnComponents[Idx] == TurnComponent)
		{
			ChangeCurrent(Idx);
			return;
		}
	}
	UE_LOG(NavGrid, Error, TEXT("ATurnManager::StartTurn() called with invalid TurnComponent %s"), *TurnComponent->GetName());
}

void ATurnManager::StartTurnNext()
{
	if (!ConsiderEndRound())
	{
		int32 NewIndex = (ComponentIndex + 1) % TurnComponents.Num();
		ChangeCurrent(NewIndex);
	}
}

UTurnComponent *ATurnManager::GetCurrentComponent()
{
	return TurnComponents[ComponentIndex];
}

void ATurnManager::ChangeCurrent(int32 NewIndex)
{
	TurnComponents[ComponentIndex]->TurnEnd();
	OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
	ComponentIndex = NewIndex;
	TurnComponents[ComponentIndex]->TurnStart();
	OnTurnStart.Broadcast(TurnComponents[ComponentIndex]);
}

bool ATurnManager::ConsiderEndRound()
{
	bool AllDone = true;
	for (UTurnComponent * TC : TurnComponents)
	{
		if (TC->bCanStillActThisRound)
		{
			AllDone = false;
			break;
		}
	}

	if (AllDone)
	{
		StartNewRound();
	}
	return AllDone;
}

void ATurnManager::StartNewRound()
{
	for (UTurnComponent *TC : TurnComponents)
	{
		TC->RoundStart();
	}
	OnRoundStart.Broadcast();
	ComponentIndex = 0;
	if (TurnComponents.Num())
	{
		TurnComponents[ComponentIndex]->TurnStart();
		OnTurnStart.Broadcast(TurnComponents[ComponentIndex]);
	}
}

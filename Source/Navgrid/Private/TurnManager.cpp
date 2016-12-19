#include "NavGridPrivatePCH.h"

void ATurnManager::BeginPlay()
{
	Super::BeginPlay();
}

void ATurnManager::StartFirstRound()
{
	for (UTurnComponent *TC : TurnComponents)
	{
		TC->RoundStart();
	}
	OnRoundStart.Broadcast();
	ComponentIndex = 0;
	TurnComponents[ComponentIndex]->TurnStart();
	OnTurnStart.Broadcast(TurnComponents[ComponentIndex]);
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
		if (Ender->bCanStillActThisRound)
		{
			// tell the will get to keep going if it still can act this round
			ChangeCurrent(ComponentIndex);
		}
		else
		{
			StartTurnNext();
		}
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
	int32 NewIndex = GetNextIndexThatCanAct();
	if (NewIndex >= 0)
	{
		ChangeCurrent(NewIndex);
	}
	else
	{
		StartNewRound();
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

int32 ATurnManager::GetNextIndexThatCanAct()
{
	for (int32 Count = 0; Count < TurnComponents.Num(); Count++)
	{
		int32 CandidateIndex = (ComponentIndex + Count) % TurnComponents.Num();
		if (TurnComponents[CandidateIndex]->bCanStillActThisRound)
		{
			return CandidateIndex;
		}
	}
	return -1;
}

void ATurnManager::StartNewRound()
{
	for (UTurnComponent *TC : TurnComponents)
	{
		TC->RoundStart();
	}
	OnRoundStart.Broadcast();
	ComponentIndex = 0;
	int32 NextIndex = GetNextIndexThatCanAct();
	if (NextIndex >= 0)
	{
		ChangeCurrent(NextIndex);
	}
}

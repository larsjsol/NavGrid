#include "NavGridPrivatePCH.h"


void ATurnManager::Register(UTurnComponent *TurnComponent)
{
	TurnComponents.AddUnique(TurnComponent);
	TurnComponent->SetTurnManager(this);
}

void ATurnManager::EndTurn(UTurnComponent *Ender)
{
	check(Ender == TurnComponents[ComponentIndex])
	if (Ender->bCanStillActThisRound)
	{
		// Ender get to keep going if it still can act this round
		ChangeCurrent(ComponentIndex);
	}
	else
	{
		StartTurnNext();
	}
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
		if (ComponentIndex >= 0 && ComponentIndex < TurnComponents.Num())
		{
			TurnComponents[ComponentIndex]->TurnEnd();
			OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
		}
		StartNewRound();
	}
}

UTurnComponent *ATurnManager::GetCurrentComponent()
{
	return TurnComponents[ComponentIndex];
}

void ATurnManager::StartFirstRound()
{
	check(Round == 0);
	StartNewRound();
}

void ATurnManager::ChangeCurrent(int32 NewIndex)
{
	check(NewIndex >= 0 && NewIndex < TurnComponents.Num());
	TurnComponents[ComponentIndex]->TurnEnd();
	OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
	ComponentIndex = NewIndex;
	TurnComponents[ComponentIndex]->TurnStart();
	OnTurnStart.Broadcast(TurnComponents[ComponentIndex]);
}

int32 ATurnManager::GetNextIndexThatCanAct()
{
	for (int32 Candidate = 0; Candidate < TurnComponents.Num(); Candidate++)
	{
		if (TurnComponents[Candidate]->bCanStillActThisRound)
		{
			return Candidate;
		}
	}
	return -1;
}

void ATurnManager::StartNewRound()
{
	Round++;
	for (UTurnComponent *TC : TurnComponents)
	{
		TC->RoundStart();
	}
	OnRoundStart.Broadcast();
	int32 First = GetNextIndexThatCanAct();
	if (First >= 0)
	{
		ComponentIndex = First;
		TurnComponents[ComponentIndex]->TurnStart();
		OnTurnStart.Broadcast(TurnComponents[ComponentIndex]);
	}
}

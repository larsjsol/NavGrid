#include "NavGridPrivatePCH.h"


ATurnManager::ATurnManager()
	:Super()
{
	TurnDelay = 0.5;
}

void ATurnManager::Register(UTurnComponent *TurnComponent)
{
	TurnComponents.AddUnique(TurnComponent);
	TurnComponent->SetTurnManager(this);
}

void ATurnManager::EndTurn(UTurnComponent *Ender)
{
	check(Ender == TurnComponents[ComponentIndex])
	if (Ender->RemainingActionPoints > 0)
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
		EndRound();
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

void ATurnManager::EndRound()
{
	if (ComponentIndex >= 0 && ComponentIndex < TurnComponents.Num())
	{
		TurnComponents[ComponentIndex]->TurnEnd();
		OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
	}
	StartNewRound();
}

void ATurnManager::ChangeCurrent(int32 NewIndex)
{
	check(NewIndex >= 0 && NewIndex < TurnComponents.Num());
	TurnComponents[ComponentIndex]->TurnEnd();
	OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
	ComponentIndex = NewIndex;
	GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnManager::StartTurnCurrent, FMath::Max(0.01f, TurnDelay));
}

int32 ATurnManager::GetNextIndexThatCanAct()
{
	// start searching from the next component in the list
	for (int32 Count = 1; Count <= TurnComponents.Num(); Count++)
	{
		int32 Candidate = (ComponentIndex + Count) % TurnComponents.Num();
		if (TurnComponents[Candidate]->RemainingActionPoints > 0)
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
	ComponentIndex = 0; // start searching for eligible components from the start of the array 
	int32 First = GetNextIndexThatCanAct();
	if (First >= 0)
	{
		ComponentIndex = First;
		GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnManager::StartTurnCurrent, FMath::Max(0.01f, TurnDelay));
	}
}

void ATurnManager::StartTurnCurrent()
{
	TurnComponents[ComponentIndex]->TurnStart();
	OnTurnStart.Broadcast(TurnComponents[ComponentIndex]);
}

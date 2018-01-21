#include "NavGridPrivatePCH.h"

void ATurnManager::BeginPlay()
{
	Super::BeginPlay();
}

void ATurnManager::StartFirstRound()
{
	StartNewRound();
}

void ATurnManager::Register(UTurnComponent *TurnComponent)
{
	TurnComponents.AddUnique(TurnComponent);
	TurnComponent->SetTurnManager(this);
}

void ATurnManager::EndTurn(UTurnComponent *Ender)
{
	if (Ender == TurnComponents[ComponentIndex])
	{
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
	else
	{
		UE_LOG(NavGrid, Error, TEXT("EndTurn() called unexpectedly by %s"), *Ender->GetName());
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
		StartNewRound();

	}
}

UTurnComponent *ATurnManager::GetCurrentComponent()
{
	return TurnComponents[ComponentIndex];
}

void ATurnManager::ChangeCurrent(int32 NewIndex)
{
	if (NewIndex >= 0 && NewIndex < TurnComponents.Num())
	{
		TurnComponents[ComponentIndex]->TurnEnd();
		OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
		ComponentIndex = NewIndex;
		TurnComponents[ComponentIndex]->TurnStart();
		OnTurnStart.Broadcast(TurnComponents[ComponentIndex]);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("ChangeCurrent: Illegal TurnComponent index %i"), NewIndex);
	}
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

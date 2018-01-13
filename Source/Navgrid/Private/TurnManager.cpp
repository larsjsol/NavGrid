#include "NavGridPrivatePCH.h"

void ATurnManager::BeginPlay()
{
	Super::BeginPlay();
}

void ATurnManager::StartFirstRound()
{
	Round++;

	for (UTurnComponent *TC : TurnComponents)
	{
		TC->RoundStart();
	}
	OnRoundStartEvent.Broadcast();
	ComponentIndex = GetNextIndexThatCanAct();
	if (ComponentIndex > -1 && ComponentIndex < TurnComponents.Num())
	{
		TurnComponents[ComponentIndex]->TurnStart();
		OnTurnStartEvent.Broadcast(TurnComponents[ComponentIndex]);
	}
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
		// broadcast the end turn event to the Ender
		Ender->TurnEnd();
		OnTurnEndEvent.Broadcast(Ender);

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
	OnTurnEndEvent.Broadcast(TurnComponents[ComponentIndex]);
	ComponentIndex = NewIndex;
	TurnComponents[ComponentIndex]->TurnStart();
	OnTurnStartEvent.Broadcast(TurnComponents[ComponentIndex]);
}

int32 ATurnManager::GetNextIndexThatCanAct()
{
	// add +1 so we only reselt the current pawn if it is the only one left that can act
	for (int32 Count = 1; Count <= TurnComponents.Num(); Count++)
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
	Round++;

	for (UTurnComponent *TC : TurnComponents)
	{
		TC->RoundStart();
	}
	OnRoundStartEvent.Broadcast();
	ComponentIndex = 0;
	int32 NextIndex = GetNextIndexThatCanAct();
	if (NextIndex >= 0)
	{
		ChangeCurrent(NextIndex);
	}
}

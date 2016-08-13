#include "NavGridPrivatePCH.h"

void ATurnManager::BeginPlay()
{
	Super::BeginPlay();

	for (UTurnComponent *TC : TurnComponents)
	{
		TC->RoundStart();
	}
	OnRoundStartEvent.Broadcast();
	if (TurnComponents.Num())
	{
		TurnComponents[0]->TurnStart();
		OnTurnStartEvent.Broadcast(*TurnComponents[0]);
	}
}

void ATurnManager::Register(UTurnComponent *TurnComponent)
{
	TurnComponents.AddUnique(TurnComponent);
}

void ATurnManager::EndTurn(UTurnComponent *Ender)
{
	if (Ender == TurnComponents[ComponentIndex])
	{
		Ender->TurnEnd();
		OnTurnEndEvent.Broadcast(*TurnComponents[ComponentIndex]);

		ComponentIndex = (ComponentIndex + 1) % TurnComponents.Num();
		if (ComponentIndex == 0)
		{
			for (UTurnComponent *TC : TurnComponents)
			{
				TC->RoundStart();
			}
			OnRoundStartEvent.Broadcast();
		}

		TurnComponents[ComponentIndex]->TurnStart();
		OnTurnStartEvent.Broadcast(*TurnComponents[ComponentIndex]);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("EndTurn() called unexpectedly by %s"), *Ender->GetName());
	}
}

UTurnComponent *ATurnManager::GetCurrentComponent()
{
	return TurnComponents[ComponentIndex];
}
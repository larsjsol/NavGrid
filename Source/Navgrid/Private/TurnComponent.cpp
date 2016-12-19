// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

void UTurnComponent::EndTurn()
{
	if (TurnManager)
	{
		TurnManager->EndTurn(this);
	}
}

void UTurnComponent::TurnStart()
{
	OnTurnStartEvent.Broadcast();
}

void UTurnComponent::TurnEnd()
{
	OnTurnEndEvent.Broadcast();
}

void UTurnComponent::RoundStart()
{
	OnRoundStartEvent.Broadcast();
}
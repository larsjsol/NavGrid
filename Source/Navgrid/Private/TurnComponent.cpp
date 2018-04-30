// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

UTurnComponent::UTurnComponent()
	:Super()
{
	ActionPoints = 1;
	bMyTurn = false;
}

void UTurnComponent::SetTurnManager(ATurnManager *InTurnManager)
{
	check(InTurnManager);
	TurnManager = InTurnManager;
}

void UTurnComponent::EndTurn()
{
	check(TurnManager);
	TurnManager->EndTurn(this);
}

void UTurnComponent::StartTurnNext()
{
	check(TurnManager);
	TurnManager->StartTurnNext();
}

void UTurnComponent::TurnStart()
{
	bMyTurn = true;
	OnTurnStart().Broadcast();
}

void UTurnComponent::TurnEnd()
{
	bMyTurn = false;
	OnTurnEnd().Broadcast();
}

void UTurnComponent::RoundStart()
{
	RemainingActionPoints = ActionPoints;
	OnRoundStart().Broadcast();
}

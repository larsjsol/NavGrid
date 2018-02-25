// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

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
	OnTurnStart().ExecuteIfBound();
}

void UTurnComponent::TurnEnd()
{
	OnTurnEnd().ExecuteIfBound();
}

void UTurnComponent::RoundStart()
{
	bCanStillActThisRound = true;
	OnRoundStart().ExecuteIfBound();
}
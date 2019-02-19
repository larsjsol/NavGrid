// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

UTurnComponent::UTurnComponent()
	:Super(),
	StartingActionPoints(1),
	RemainingActionPoints(1),
	TurnManager(nullptr)
{
}

void UTurnComponent::BeginPlay()
{
	Super::BeginPlay();
	TurnManager = GetWorld()->GetGameState<ANavGridGameState>()->GetTurnManager();
	TurnManager->Register(this);
}

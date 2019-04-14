// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

UTurnComponent::UTurnComponent()
	:Super(),
	TurnManager(nullptr),
	StartingActionPoints(1),
	RemainingActionPoints(1)
{
}

void UTurnComponent::BeginPlay()
{
	Super::BeginPlay();
	TurnManager = GetWorld()->GetGameState<ANavGridGameState>()->GetTurnManager();
	TurnManager->RegisterTurnComponent(this);
}

void UTurnComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	if (IsValid(TurnManager))
	{
		TurnManager->UnregisterTurnComponent(this);
	}
}

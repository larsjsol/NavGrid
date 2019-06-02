// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

UTurnComponent::UTurnComponent()
	:Super(),
	TurnManager(nullptr),
	StartingActionPoints(1),
	RemainingActionPoints(1),
	TurnTimeout(30)
{
}

void UTurnComponent::BeginPlay()
{
	Super::BeginPlay();
	RegisterWithTurnManager();
}

void UTurnComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	UnregisterWithTurnManager();
}

void UTurnComponent::OnTurnTimeout()
{
	if (MyTurn())
	{
		UE_LOG(NavGrid, Warning, TEXT("Turn timeout (%f sec) reached for %s"), TurnTimeout, *GetOwner()->GetName());
		RemainingActionPoints = 0;
		EndTurn();
	}
}

AActor *UTurnComponent::GetCurrentActor() const
{
	if (IsValid(TurnManager))
	{
		return TurnManager->GetCurrentActor();
	}
	return nullptr;
}

void UTurnComponent::RegisterWithTurnManager()
{
	UnregisterWithTurnManager();
	TurnManager = GetWorld()->GetGameState<ANavGridGameState>()->GetTurnManager();
	TurnManager->RegisterTurnComponent(this);
}

void UTurnComponent::UnregisterWithTurnManager()
{
	if (IsValid(TurnManager))
	{
		TurnManager->UnregisterTurnComponent(this);
	}
	TurnManager = nullptr;
}

void UTurnComponent::OnTurnStart()
{
	GetWorld()->GetTimerManager().SetTimer(TurnTimeoutHandle, this, &UTurnComponent::OnTurnTimeout, TurnTimeout);
}

void UTurnComponent::OnTurnEnd()
{
	GetWorld()->GetTimerManager().ClearTimer(TurnTimeoutHandle);
}

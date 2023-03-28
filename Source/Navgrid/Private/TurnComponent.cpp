// Fill out your copyright notice in the Description page of Project Settings.

#include "TurnComponent.H"

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

ATurnManager * UTurnComponent::GetTurnManager()
{
	if (!IsValid(TurnManager))
	{
		RegisterWithTurnManager();
	}
	return TurnManager;
}

void UTurnComponent::EndTurn()
{
	if (IsValid(TurnManager))
	{
		TurnManager->EndTurn(this);
	}
}

void UTurnComponent::EndTeamTurn()
{
	if (IsValid(TurnManager))
	{
		TurnManager->EndTeamTurn(FGenericTeamId::GetTeamIdentifier(GetOwner()));
	}
}

void UTurnComponent::RequestStartTurn()
{
	if (IsValid(TurnManager))
	{
		TurnManager->RequestStartTurn(this);

	}
}

void UTurnComponent::RequestStartNextComponent()
{
	if (IsValid(TurnManager))
	{
		TurnManager->RequestStartNextComponent(this);
	}
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

void UTurnComponent::OwnerReadyForInput()
{
	if (IsValid(TurnManager) && TurnManager->GetCurrentComponent() == this)
	{
		TurnManager->OnReadyForInput().Broadcast(this);
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
	ANavGridGameState *GameState = GetWorld()->GetGameState<ANavGridGameState>();
	if (IsValid(GameState))
	{
		TurnManager = GameState->GetTurnManager();
		TurnManager->RegisterTurnComponent(this);
	}
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

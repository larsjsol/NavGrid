#include "NavGridPrivatePCH.h"

bool FTeam::HasComponentWaitingForTurn()
{
	for (UTurnComponent *Comp : Components)
	{
		if (IsValid(Comp) && Comp->RemainingActionPoints > 0)
		{
			return true;
		}
	}
	return false;
}

ATurnManager::ATurnManager()
	:Super(),
	TurnDelay(0.5),
	CurrentComponent(0),
	CurrentTeam(0),
	Round(1),
	bIgnoreActionPointsForNextComponent(false)
{
}

void ATurnManager::BeginPlay()
{
	Super::BeginPlay();
	OnRoundStart().Broadcast();
	GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnManager::StartTurn, FMath::Max(0.01f, TurnDelay));
}

void ATurnManager::RegisterTurnComponent(UTurnComponent *TurnComponent)
{
	check(IsValid(TurnComponent));
	if (!Teams.Contains(TurnComponent->TeamId()))
	{
		Teams.Add(TurnComponent->TeamId(), FTeam());
	}
	Teams[TurnComponent->TeamId()].Components.AddUnique(TurnComponent);
}

void ATurnManager::UnregisterTurnComponent(UTurnComponent * TurnComponent)
{
	if (Teams.Contains(TurnComponent->TeamId()))
	{
		FTeam &Team = Teams[TurnComponent->TeamId()];
		Team.Components.Remove(TurnComponent);
		if (Team.Components.Num() == 0)
		{
			Teams.Remove(TurnComponent->TeamId());
		}
	}
}

void ATurnManager::StartTurnForNextComponent()
{
	check(Teams.Num() > 0);

	TArray<FGenericTeamId> TeamIdArray;
	Teams.GenerateKeyArray(TeamIdArray);

	bool bStartNewRound = false;

	// first find the correct team
	if (!Teams.Contains(CurrentTeam) || !Teams[CurrentTeam].HasComponentWaitingForTurn())
	{
		bStartNewRound = true; // start a new round if no teams have members that are waiting for their turn
		for (FGenericTeamId &TeamId : TeamIdArray)
		{
			if (Teams[TeamId].HasComponentWaitingForTurn())
			{
				CurrentTeam = TeamId;
				bStartNewRound = false;
				break;
			}
		}
	}

	if (bStartNewRound)
	{
		OnRoundEnd().Broadcast();
		for (FGenericTeamId &TeamId : TeamIdArray)
		{
			for (UTurnComponent *Comp : Teams[TeamId].Components)
			{
				Comp->RemainingActionPoints = Comp->StartingActionPoints;
			}
		}
		OnRoundStart().Broadcast();
		CurrentComponent = 0;
		CurrentTeam = TeamIdArray[0];
		Round++;
	}
	else
	{
		// go through the list of components, starting with the one follonwing the current component
		for (int32 Idx = 1; Idx <= Teams[CurrentTeam].Components.Num(); Idx++)
		{
			int32 CandidateComponentIndex = (CurrentComponent + Idx) % Teams[CurrentTeam].Components.Num();
			UTurnComponent *Comp = Teams[CurrentTeam].Components[CandidateComponentIndex];
			if (IsValid(Comp) && Comp->RemainingActionPoints > 0)
			{
				CurrentComponent = CandidateComponentIndex;
				break;
			}
		}
	}

	GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnManager::StartTurn, FMath::Max(0.01f, TurnDelay));
}

void ATurnManager::StartTurn()
{
	if (IsValid(Teams[CurrentTeam].Components[CurrentComponent]) &&
		(Teams[CurrentTeam].Components[CurrentComponent]->RemainingActionPoints > 0 ||
		bIgnoreActionPointsForNextComponent))
	{
		OnTurnStart().Broadcast(Teams[CurrentTeam].Components[CurrentComponent]);
		bIgnoreActionPointsForNextComponent = false;
	}
	else
	{
		StartTurnForNextComponent();
	}
}

bool ATurnManager::EndTurn(UTurnComponent *Ender)
{
	if (Ender == Teams[CurrentTeam].Components[CurrentComponent])
	{
		OnTurnEnd().Broadcast(Ender);
		GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnManager::StartTurn, FMath::Max(0.01f, TurnDelay));
		return true;
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("EndTurn(%s) was called out of turn"), *Ender->GetName());
		return false;
	}
}

bool ATurnManager::EndTeamTurn(FGenericTeamId InTeamId)
{
	if (InTeamId == CurrentTeam)
	{
		for (UTurnComponent *Comp : Teams[CurrentTeam].Components)
		{
			if (Comp->RemainingActionPoints > 0)
			{
				Comp->RemainingActionPoints = 0;
				OnTurnEnd().Broadcast(Comp);
			}
		}
		GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnManager::StartTurn, FMath::Max(0.01f, TurnDelay));
		return true;
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("EndTurn(%i) was called out of turn"), InTeamId.GetId());
		return false;
	}
}

bool ATurnManager::RequestStartTurn(UTurnComponent * CallingComponent, bool bIgnoreRemainingActionPoints)
{
	if (CallingComponent->RemainingActionPoints > 0 || bIgnoreRemainingActionPoints)
	{
		OnTurnEnd().Broadcast(Teams[CurrentTeam].Components[CurrentComponent]);
		CurrentTeam = CallingComponent->TeamId();
		CurrentComponent = Teams[CurrentTeam].Components.Find(CallingComponent);
		bIgnoreActionPointsForNextComponent = bIgnoreRemainingActionPoints;
		GetWorldTimerManager().SetTimer(TurnDelayHandle, this, &ATurnManager::StartTurn, FMath::Max(0.01f, TurnDelay));
		return true;
	}
	else
	{
		return false;
	}
}

bool ATurnManager::RequestStartNextComponent(UTurnComponent *CallingComponent)
{
	if (CallingComponent->TeamId() == CurrentTeam && Teams[CurrentTeam].HasComponentWaitingForTurn())
	{
		StartTurnForNextComponent();
		return true;
	}
	else
	{
		return false;
	}
}

UTurnComponent *ATurnManager::GetCurrentComponent()
{
	if (Teams.Contains(CurrentTeam) && Teams[CurrentTeam].Components.Num() > CurrentComponent)
	{
		return Teams[CurrentTeam].Components[CurrentComponent];
	}
	else
	{
		return nullptr;
	}
}

#include "NavGridPrivatePCH.h"

ATurnManager::ATurnManager() :
	CurrentComponent(nullptr),
	NextComponent(nullptr),
	Round(0),
	bStartNewTurn(true)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATurnManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bStartNewTurn && Teams.Num())
	{
		// broadcast TurnEnd and TeamTurnEnd
		if (IsValid(CurrentComponent))
		{
			OnTurnEnd().Broadcast(CurrentComponent);
			if (!IsValid(FindNextTeamMember(CurrentComponent->TeamId())))
			{
				OnTeamTurnEnd().Broadcast(CurrentComponent->TeamId());
			}
		}

		// start a new round if no more components can act this turn
		if (Round == 0 || !HasComponentsThatCanAct())
		{
			if (Round > 0)
			{
				OnRoundEnd().Broadcast();
			}

			for (TPair<FGenericTeamId, UTurnComponent *> &Pair : Teams)
			{
				Pair.Value->RemainingActionPoints = Pair.Value->StartingActionPoints;
			}

			CurrentComponent = nullptr;
			NextComponent = nullptr;

			Round++;
			OnRoundStart().Broadcast();
		}

		// figure out which component that has the next turn
		if (!IsValid(NextComponent))
		{
			if (IsValid(CurrentComponent) && CurrentComponent->RemainingActionPoints > 0)
			{
				NextComponent = CurrentComponent;
			}
			else
			{
				NextComponent = FindNextComponent();
			}
		}

		// broadcast TurnStart and TeamTurnStart
		if (IsValid(NextComponent))
		{
			UTurnComponent *PreviousComponent = CurrentComponent;
			CurrentComponent = NextComponent;
			if (!IsValid(PreviousComponent) || CurrentComponent->TeamId() != PreviousComponent->TeamId())
			{
				OnTeamTurnStart().Broadcast(CurrentComponent->TeamId());
			}
			OnTurnStart().Broadcast(CurrentComponent);
		}

		NextComponent = nullptr;
		bStartNewTurn = false;
	}
}

void ATurnManager::RegisterTurnComponent(UTurnComponent *TurnComponent)
{
	Teams.AddUnique(TurnComponent->TeamId(), TurnComponent);
}

void ATurnManager::UnregisterTurnComponent(UTurnComponent * TurnComponent)
{
	Teams.RemoveSingle(TurnComponent->TeamId(), TurnComponent);
	if (CurrentComponent == TurnComponent)
	{
		CurrentComponent = nullptr;
		bStartNewTurn = true;
	}
}

void ATurnManager::EndTurn(UTurnComponent *Ender)
{
	if (Ender == CurrentComponent)
	{
		bStartNewTurn = true;
	}
}

void ATurnManager::EndTeamTurn(FGenericTeamId InTeamId)
{
	if (CurrentComponent->TeamId() == InTeamId)
	{
		TArray<UTurnComponent *> TeamMemebers;
		Teams.MultiFind(InTeamId, TeamMemebers);
		for (UTurnComponent *Member : TeamMemebers)
		{
			Member->RemainingActionPoints = 0;
		}

		bStartNewTurn = true;
	}
}

void ATurnManager::RequestStartTurn(UTurnComponent * CallingComponent)
{
	if (!IsValid(CurrentComponent) || CurrentComponent->TeamId() == CallingComponent->TeamId())
	{
		NextComponent = CallingComponent;
		bStartNewTurn = true;
	}
}

void ATurnManager::RequestStartNextComponent(UTurnComponent *CallingComponent)
{
	if (IsValid(CurrentComponent) && CurrentComponent->TeamId() == CallingComponent->TeamId())
	{
		UTurnComponent *Candidate = FindNextTeamMember(CallingComponent->TeamId());
		if (IsValid(Candidate))
		{
			NextComponent = Candidate;
			bStartNewTurn = true;
		}
	}
}

FGenericTeamId ATurnManager::GetCurrentTeam() const
{
	return CurrentComponent ? CurrentComponent->TeamId() : FGenericTeamId::NoTeam;
}

AActor *ATurnManager::GetCurrentActor() const
{
	return IsValid(CurrentComponent) ? CurrentComponent->GetOwner() : nullptr;
}

UTurnComponent * ATurnManager::FindNextTeamMember(const FGenericTeamId & TeamId)
{
	TArray<UTurnComponent *> TeamMembers;
	Teams.MultiFind(TeamId, TeamMembers, true);
	int32 StartIndex;
	TeamMembers.Find(CurrentComponent, StartIndex);
	for (int32 Idx = 1; Idx < TeamMembers.Num(); Idx++)
	{
		UTurnComponent *Candidate = TeamMembers[(StartIndex + Idx) % TeamMembers.Num()];
		if (Candidate->RemainingActionPoints > 0)
		{
			return Candidate;
		}
	}
	return nullptr;
}

UTurnComponent * ATurnManager::FindNextComponent()
{
	TArray<FGenericTeamId> TeamIds;
	Teams.GenerateKeyArray(TeamIds);
	TeamIds.Sort();
	for (FGenericTeamId &TeamId : TeamIds)
	{
		UTurnComponent *Candidate = FindNextTeamMember(TeamId);
		if (IsValid(Candidate))
		{
			return Candidate;
		}
	}

	return nullptr;
}

bool ATurnManager::HasComponentsThatCanAct()
{
	for (TPair<FGenericTeamId, UTurnComponent *> &Pair : Teams)
	{
		if (Pair.Value->RemainingActionPoints > 0)
		{
			return true;
		}
	}

	return false;
}


// Fill out your copyright notice in the Description page of Project Settings.
#include "NavGridPrivatePCH.h"

ATeamTurnManager::ATeamTurnManager()
	:Super()
{
	Master = true;
	TurnComponent = CreateDefaultSubobject<UTurnComponent>("Turn Component");
}

void ATeamTurnManager::Register(UTurnComponent *TurnComponent)
{
	if (!TurnComponents.Contains(TurnComponent))
	{
		if (Master)
		{
			AGridPawn *GridPawnOwner = Cast<AGridPawn>(TurnComponent->GetOwner());
			check(GridPawnOwner);

			uint8 Id = GridPawnOwner->GetGenericTeamId().GetId();
			if (!Teams.Contains(Id))
			{
				UPROPERTY()
				ATeamTurnManager *NewManager = GetWorld()->SpawnActor<ATeamTurnManager>();
				NewManager->SetOwner(this);
				NewManager->Master = false;
				NewManager->TurnComponent->SetTurnManager(this);
				NewManager->TurnDelay = TurnDelay;
				// one round for the slave is a single turn for the master
				NewManager->TurnComponent->OnTurnStart().AddUObject(NewManager, &ATeamTurnManager::StartNewRound);
				
				Teams.Add(Id, NewManager);
				TurnComponents.Add(NewManager->TurnComponent);
			}
			Teams[Id]->Register(TurnComponent);
		}
		else
		{
			TurnComponents.Add(TurnComponent);
			TurnComponent->SetTurnManager(this);
		}
	}
}

void ATeamTurnManager::StartTurnNext()
{
	int32 NewIndex = GetNextIndexThatCanAct();
	if (NewIndex >= 0)
	{
		ChangeCurrent(NewIndex);
	}
	else
	{
		if (ComponentIndex >= 0 && ComponentIndex < TurnComponents.Num())
		{
			TurnComponents[ComponentIndex]->TurnEnd();
			OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
		}

		if (Master)
		{
			StartNewRound();
		}
		else
		{
			TurnComponent->RemainingActionPoints--;
			TurnComponent->EndTurn();
		}
	}
}

void ATeamTurnManager::EndRound()
{
	if (ComponentIndex >= 0 && ComponentIndex < TurnComponents.Num())
	{
		TurnComponents[ComponentIndex]->TurnEnd();
		OnTurnEnd.Broadcast(TurnComponents[ComponentIndex]);
	}
	OnRoundEnd.Broadcast();

	if (Master)
	{
		StartNewRound();
	}
	else
	{
		TurnComponent->RemainingActionPoints = 0;
		TurnComponent->EndTurn();
	}
}

ATurnManager *ATeamTurnManager::GetTurnManager(const FGenericTeamId& TeamID)
{
	if (Master && Teams.Contains(TeamID.GetId()))
	{
		return Teams[TeamID.GetId()];
	}
	else
	{
		return NULL;
	}
}

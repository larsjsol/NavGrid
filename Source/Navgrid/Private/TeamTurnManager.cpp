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

			int32 &Id = GridPawnOwner->TeamId;
			if (!Teams.Contains(Id))
			{
				UPROPERTY()
				ATeamTurnManager *NewManager = GetWorld()->SpawnActor<ATeamTurnManager>();
				NewManager->SetOwner(this);
				NewManager->Master = false;
				NewManager->TurnComponent->SetTurnManager(this);
				// one round for the slave is a single turn for the master
				NewManager->TurnComponent->OnTurnStart().BindUObject(NewManager, &ATeamTurnManager::StartNewRound);
				
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
			TurnComponent->bCanStillActThisRound = false;
			TurnComponent->EndTurn();
		}
	}
}

ATurnManager *ATeamTurnManager::GetTurnManager(int32 TeamId/* = 0*/)
{
	if (Master)
	{
		if (Teams.Contains(TeamId))
		{
			return Teams[TeamId];
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return this;
	}
}

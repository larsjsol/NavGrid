// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridPrivatePCH.h"

ANavGridGameMode::ANavGridGameMode()
	:Super()
{
	PlayerControllerClass = ANavGridPC::StaticClass();
	GameStateClass = ANavGridGameState::StaticClass();
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGridGameMode.h"
#include "NavGridPC.h"

ANavGridGameMode::ANavGridGameMode()
	:Super()
{
	PlayerControllerClass = ANavGridPC::StaticClass();
	GameStateClass = ANavGridGameState::StaticClass();
}

void ANavGridGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Uncomment for trace debug lines
	//GetWorld()->DebugDrawTraceTag = "NavGridMovement";
	//GetWorld()->DebugDrawTraceTag = "NavGridTile";
	//GetWorld()->DebugDrawTraceTag = "NavGridTilePlacement";
}

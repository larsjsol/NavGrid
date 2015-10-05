// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "NavGrid.h"
#include "GridMovementComponent.h"

void UGridMovementComponent::BeginPlay()
{
	/* Grab a reference to the NavGrid */
	TActorIterator<ANavGrid> NavGridItr(GetWorld());
	Grid = *NavGridItr;
	if (!Grid) { UE_LOG(NavGrid, Error, TEXT("%st: Unable to get reference to Navgrid."), *GetName()); }
}
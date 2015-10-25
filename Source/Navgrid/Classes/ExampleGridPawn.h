// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "GridPawn.h"
#include "ExampleGridPawn.generated.h"

class UStaticMeshComponent;
class UArrowComponent;

/*
**
* A simple pawn used for demonstrating the NavGrid plugin.
*/
UCLASS()
class NAVGRID_API AExampleGridPawn : public AGridPawn
{
	GENERATED_BODY()

public:
	AExampleGridPawn();
	/* Just a cone so we can see the pawn */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components") UStaticMeshComponent *StaticMesh = NULL;
	/* An arrow pointing forward */
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components") UArrowComponent *Arrow = NULL;
};
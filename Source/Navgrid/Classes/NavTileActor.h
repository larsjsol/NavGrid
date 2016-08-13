// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "NavTileActor.generated.h"

class UNavTileComponent;
/**
* A simple actor with a NavTileComponent and a static mesh
*/
UCLASS()
class NAVGRID_API ANavTileActor : public AActor
{
	GENERATED_BODY()

public:
	ANavTileActor(const FObjectInitializer &ObjectInitializer);

	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")  USceneComponent *SceneComponent;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")  UNavTileComponent *NavTileComponent;
	UPROPERTY(BlueprintReadOnly, EditAnyWhere, Category = "Components")  UStaticMeshComponent *Mesh;
};
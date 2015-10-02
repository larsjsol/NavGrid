// Fill out your copyright notice in the Description page of Project Settings.

#include "BoardGame.h"
#include "Tile.h"

ATile::ATile()
	:Super()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	Mesh->AttachParent = SceneComponent;
}



#include "NavGridPrivatePCH.h"
#include "NavTileActor.h"

ANavTileActor::ANavTileActor(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	Mesh->SetupAttachment(SceneComponent);
	NavTileComponent = CreateDefaultSubobject<UNavTileComponent>("NavTileComponent");
	NavTileComponent->SetupAttachment(SceneComponent);
	NavTileComponent->SetBoxExtent(FVector(100, 100, 5));

	const TCHAR* AssRef = TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Tile.NavGrid_Tile'");
	auto OF = ConstructorHelpers::FObjectFinder<UStaticMesh>(AssRef);
	if (OF.Succeeded())
	{
		Mesh->SetStaticMesh(OF.Object);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("Error loading %s"), AssRef);
	}

	SetActorTickEnabled(false);
}

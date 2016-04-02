#include "NavGrid.h"
#include "NavTileActor.h"
#include "NavTileComponent.h"

ANavTileActor::ANavTileActor()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	Mesh->AttachParent = SceneComponent;
	NavTileComponent = CreateDefaultSubobject<UNavTileComponent>("NavTileComponent");
	NavTileComponent->AttachParent = SceneComponent;

	TCHAR* AssRef = TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Tile.NavGrid_Tile'");
	auto OF = ConstructorHelpers::FObjectFinder<UStaticMesh>(AssRef);
	if (OF.Succeeded())
	{
		Mesh->SetStaticMesh(OF.Object);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("Error loading %s"), AssRef);
	}
}
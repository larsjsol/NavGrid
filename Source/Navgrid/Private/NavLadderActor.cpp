#include "NavGrid.h"
#include "NavLadderActor.h"
#include "NavLadderComponent.h"

ANavLadderActor::ANavLadderActor(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;

	NavLadderComponent = CreateDefaultSubobject<UNavLadderComponent>("NavLadderComponent");
	NavLadderComponent->SetRelativeLocation(FVector(0, -10, 0));
	NavLadderComponent->AttachParent = SceneComponent;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	Mesh->AttachParent = SceneComponent;

	TCHAR* AssRef = TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Ladder.NavGrid_Ladder'");
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
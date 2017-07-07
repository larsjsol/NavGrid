#include "NavGridPrivatePCH.h"
#include "NavLadderActor.h"

ANavLadderActor::ANavLadderActor(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;

	NavLadderComponent = CreateDefaultSubobject<UNavLadderComponent>("NavLadderComponent");
	NavLadderComponent->SetRelativeLocation(FVector(0, 0, 150));
	NavLadderComponent->SetBoxExtent(FVector(5, 100, 150));
	NavLadderComponent->SetupAttachment(SceneComponent);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("StaticMesh");
	Mesh->SetupAttachment(SceneComponent);

	const TCHAR* AssRef = TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Ladder.NavGrid_Ladder'");
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
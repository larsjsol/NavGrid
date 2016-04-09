#include "NavGrid.h"
#include "NavLadderActor.h"
#include "NavLadderComponent.h"

ANavLadderActor::ANavLadderActor(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer)
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;

	NavLadderComponent = CreateDefaultSubobject<UNavLadderComponent>("NavLadderComponent");
	NavLadderComponent->AttachParent = SceneComponent;
}
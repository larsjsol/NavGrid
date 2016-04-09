// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "NavTileComponent.h"
#include <limits>

UNavTileComponent::UNavTileComponent(const FObjectInitializer &ObjectInitializer)
	:Super(ObjectInitializer)
{
	Extent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, "Extent");
	Extent->AttachParent = this;
	Extent->SetBoxExtent(FVector(100, 100, 5));
	
	Extent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Extent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // So we get mouse over events
	Extent->SetCollisionResponseToChannel(ANavGrid::ECC_Walkable, ECollisionResponse::ECR_Block); // So we can find the floor with a line trace
	Extent->OnBeginCursorOver.AddDynamic(this, &UNavTileComponent::CursorOver);
	Extent->OnEndCursorOver.AddDynamic(this, &UNavTileComponent::EndCursorOver);
	Extent->OnClicked.AddDynamic(this, &UNavTileComponent::Clicked);

	PawnLocationOffset = CreateDefaultSubobject<USceneComponent>(TEXT("PawnLocationOffset"));
	PawnLocationOffset->SetRelativeLocation(FVector::ZeroVector);
	PawnLocationOffset->AttachParent = this;

	HoverCursor = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, "HoverCursor");
	HoverCursor->AttachParent = PawnLocationOffset;
	HoverCursor->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	HoverCursor->ToggleVisibility(false);
	HoverCursor->SetRelativeLocation(FVector(0, 0, 20));
	auto HCRef = TEXT("StaticMesh'/NavGrid/SMesh/NavGrid_Cursor.NavGrid_Cursor'");
	auto HCFinder = ConstructorHelpers::FObjectFinder<UStaticMesh>(HCRef);
	if (HCFinder.Succeeded()) 
	{ 
		HoverCursor->SetStaticMesh(HCFinder.Object);
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("Error loading %s"), HCRef);
	}
}

void UNavTileComponent::BeginPlay()
{
	TActorIterator<ANavGrid> Itr(GetWorld());
	if (Itr)
	{
		Grid = *Itr;
	}
	else
	{
		UE_LOG(NavGrid, Error, TEXT("%s: Unable to find NavGrid"), *GetName());
	}
}

void UNavTileComponent::ResetPath()
{
	Distance = std::numeric_limits<float>::infinity();
	Backpointer = NULL;
	Visited = false;
}

TArray<FVector>* UNavTileComponent::GetContactPoints()
{	
	if (!ContactPoints.Num())
	{
		int32 XExtent = Extent->GetScaledBoxExtent().X;
		int32 YExtent = Extent->GetScaledBoxExtent().Y;
		for (int32 X = -XExtent; X <= XExtent; X += YExtent)
		{
			for (int32 Y = -YExtent; Y <= YExtent; Y += YExtent)
			{
				FVector RelativeLocation = GetComponentRotation().RotateVector(FVector(X, Y, 0));
				FVector WorldLocation = GetComponentLocation() + RelativeLocation;
				ContactPoints.Add(WorldLocation);
			}
		}
	}
	return &ContactPoints;
}

TArray<UNavTileComponent*>* UNavTileComponent::GetNeighbours()
{
	// Find neighbours if we have'nt already done so
	if (!Neighbours.Num())
	{
		for (TObjectIterator<UNavTileComponent> Itr; Itr; ++Itr)
		{
			if (Itr->GetWorld() == GetWorld() && *Itr != this)
			{
				bool Added = false; // stop comparing CPs when we know a tile is a neighbour
				for (const FVector &OtherCP : *Itr->GetContactPoints())
				{
					for (const FVector &MyCP : *GetContactPoints())
					{
						if ((OtherCP - MyCP).Size() < 25)
						{
							Neighbours.Add(*Itr);
							DrawDebugLine(GetWorld(), PawnLocationOffset->GetComponentLocation() + 10, (*Itr)->PawnLocationOffset->GetComponentLocation() + 10, FColor::Green, true, -1, 0, 1);
							Added = true;
							break;
						}
					}
					if (Added) { break; }
				}
			}
		}
	}
	return &Neighbours;
}

bool UNavTileComponent::Obstructed(const FVector &FromPos, const UCapsuleComponent &CollisionCapsule)
{
	return Obstructed(FromPos, PawnLocationOffset->GetComponentLocation(), CollisionCapsule);
}

bool UNavTileComponent::Obstructed(const FVector & From, const FVector & To, const UCapsuleComponent & CollisionCapsule)
{
	FHitResult OutHit;
	FVector Start = From + CollisionCapsule.RelativeLocation;
	FVector End = To + CollisionCapsule.RelativeLocation;
	FQuat Rot = FQuat::Identity;
	FCollisionShape CollisionShape = CollisionCapsule.GetCollisionShape();
	FCollisionQueryParams CQP;
	CQP.AddIgnoredActor(CollisionCapsule.GetOwner());
	FCollisionResponseParams CRP;
	bool HitSomething = CollisionCapsule.GetWorld()->SweepSingleByChannel(OutHit, Start, End, Rot, ECollisionChannel::ECC_Pawn, CollisionShape, CQP, CRP);
	return HitSomething;
}

void UNavTileComponent::GetUnobstructedNeighbours(const UCapsuleComponent &CollisionCapsule, TArray<UNavTileComponent *> &OutNeighbours)
{
	OutNeighbours.Empty();
	for (auto N : *GetNeighbours())
	{
		if (!N->Obstructed(PawnLocationOffset->GetComponentLocation(), CollisionCapsule)) 
		{ 
			OutNeighbours.Add(N);
		}
	}
}

void UNavTileComponent::Clicked(UPrimitiveComponent* TouchedComponent)
{
	if (Grid)
	{
		Grid->TileClicked(*this);
	}
}

void UNavTileComponent::CursorOver(UPrimitiveComponent* TouchedComponent)
{
	HoverCursor->SetVisibility(true);
	if (Grid)
	{
		Grid->TileCursorOver(*this);
	}
}

void UNavTileComponent::EndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	HoverCursor->SetVisibility(false);
	if (Grid)
	{
		Grid->EndTileCursorOver(*this);
	}
}

void UNavTileComponent::GetPathPoints(const FVector &FromPos, TArray<FVector>& OutPathPoints, TArray<FVector> &OutUpVectors)
{
	OutPathPoints.Empty();
	OutPathPoints.Add(PawnLocationOffset->GetComponentLocation());
	OutUpVectors.Empty();
	OutUpVectors.Add(GetComponentRotation().RotateVector(FVector(0, 0, 1)));
}

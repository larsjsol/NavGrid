// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <initializer_list>

#include "GameFramework/Actor.h"
#include "NavSphere.generated.h"


USTRUCT()
struct FPolygon
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> VertexIds;

	FPolygon() {}
	FPolygon(std::initializer_list<int32> VertIds)
	{
		for (int32 VertId : VertIds)
		{
			VertexIds.Add(VertId);
		}
	}
};

/*
* Holds the geometry for a sphere made up of hexagons and pentagons
*
* Each vertex is 1.0 from the center which is atq 0.0, 0.0, 0.0
*
* This code borrows heavily from:
* https://en.wikipedia.org/wiki/Regular_icosahedron
* http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
* https://experilous.com/1/blog/post/procedural-planet-generation
*/
USTRUCT()
struct FIcoSphere {
	GENERATED_BODY()

	static const float Phi;

	/* Stored as unit vectors*/
	UPROPERTY()
	TArray<FVector> Vertices;
	UPROPERTY()
	TArray<FPolygon> Triangles;
	UPROPERTY()
	TArray<FPolygon> Polygons;

	UPROPERTY()
	int32 CurrentSubdivisions = 0;
	
	FIcoSphere()
	{
		MakeIcosahedron();
	}

	/*
	* Initialize as a icosahedron
	*/
	void MakeIcosahedron();

	/*
	* Make a dodecahedron
	*/
	void MakeDodecahedron();

	/*
	* Find the point in the middle of A and B with the same distance to the center
	*/
	FVector FindMiddle(int32 VertexIdA, int32 VertexIdB);

	/*
	* Find the center of a polygon
	*/
	FVector FindCenter(const FPolygon &Polygon);

	void SubDivide(int32 Iterations = 1);

	/*
	* Convenience function to add a vertex
	*/
	int AddVertex(float X, float Y, float Z)
	{
		return Vertices.AddUnique(FVector(X, Y, Z).GetSafeNormal());
	}

	/*
	* Convenience function to add a triangle
	*/
	void AddTriangle(int32 A, int32 B, int32 C)
	{
		Triangles.Add(FPolygon({ A, B, C }));
	}

	void DrawDebug(const UWorld* World, const FTransform &Transform);
};

UCLASS()
class NAVGRID_API ANavSphere : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANavSphere();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnConstruction(const FTransform &Transform) override;

	/* Scene Component (root) */
	UPROPERTY(EditAnyWhere, BlueprintReadWrite, Category = "Components") USceneComponent *SceneComponent = NULL;

	UPROPERTY()
	FIcoSphere IcoSphere;
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "NavGrid.h"
#include "NavSphere.h"

#include <cmath>
#include <limits>

const float FIcoSphere::Phi = (1 + sqrt(5.0)) / 2.0;

void FIcoSphere::MakeIcosahedron()
{
	/* remove any existing geometry */
	Vertices.Empty();
	Triangles.Empty();

	/* Add vertices */
	AddVertex(-1, Phi, 0);
	AddVertex(1, Phi, 0);
	AddVertex(-1, -Phi, 0);
	AddVertex(1, -Phi, 0);

	AddVertex(0, -1, Phi);
	AddVertex(0, 1, Phi);
	AddVertex(0, -1, -Phi);
	AddVertex(0, 1, -Phi);

	AddVertex(Phi, 0, -1);
	AddVertex(Phi, 0, 1);
	AddVertex(-Phi, 0, -1);
	AddVertex(-Phi, 0, 1);

	/* Add triangles */
	// 5 faces around point 0
	AddTriangle(0, 11, 5);
	AddTriangle(0, 5, 1);
	AddTriangle(0, 1, 7);
	AddTriangle(0, 7, 10);
	AddTriangle(0, 10, 11);

	// 5 adjacent faces
	AddTriangle(1, 5, 9);
	AddTriangle(5, 11, 4);
	AddTriangle(11, 10, 2);
	AddTriangle(10, 7, 6);
	AddTriangle(7, 1, 8);

	// 5 faces around point 3
	AddTriangle(3, 9, 4);
	AddTriangle(3, 4, 2);
	AddTriangle(3, 2, 6);
	AddTriangle(3, 6, 8);
	AddTriangle(3, 8, 9);

	// 5 adjacent faces
	AddTriangle(4, 9, 5);
	AddTriangle(2, 4, 11);
	AddTriangle(6, 2, 10);
	AddTriangle(8, 6, 7);
	AddTriangle(9, 8, 1);
}

void FIcoSphere::MakeDodecahedron()
{
	int32 CurrentVertexNum = Vertices.Num(); // Don't iterate over the new vertices we add in the loop
	for (int32 VertId = 0; VertId < CurrentVertexNum; VertId++)
	{
		FPolygon Polygon;
		for (FPolygon &Triangle : Triangles)
		{
			// the center for each triangle is a corner for our new polygon
			if (Triangle.VertexIds.Contains(VertId))
			{
				// find the center
				FVector Center = (Vertices[Triangle.VertexIds[0]] + Vertices[Triangle.VertexIds[1]] + Vertices[Triangle.VertexIds[2]]) / 3.0;
				int32 CenterVertId = AddVertex(Center.X, Center.Y, Center.Z);
				// add it as a vertex to the polygon
				Polygon.VertexIds.Add(CenterVertId);
			}
		}

		// order the vertices so the edges are properly defined
		TArray<int32> OrderedVertices;
		OrderedVertices.Add(Polygon.VertexIds.Pop());
		while (Polygon.VertexIds.Num())
		{
			int32 ClosestId;
			float ClosestDistance = std::numeric_limits<float>::infinity();
			FVector Current = Vertices[OrderedVertices.Last()];
			for (int32 Id : Polygon.VertexIds)
			{
				float Distance = (Current - Vertices[Id]).Size();
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					ClosestId = Id;
				}
			}

			OrderedVertices.Add(ClosestId);
			Polygon.VertexIds.RemoveSingle(ClosestId);
		}
		Polygon.VertexIds = OrderedVertices;

		// finaly add the polygon
		Polygons.Add(Polygon);
	}
}

FVector FIcoSphere::FindMiddle(int32 VertexIdA, int32 VertexIdB)
{
	// floating point arithmetic anxiety 
	FVector A = Vertices[FMath::Max(VertexIdA, VertexIdB)];
	FVector B = Vertices[FMath::Min(VertexIdA, VertexIdB)];

	FVector Middle = (A + B) / 2.0;

	return Middle;
}

void FIcoSphere::SubDivide(int32 Iterations/* = 1*/)
{
	// Rebuild everything. Maybe figure out how to do this better in the future
	if (CurrentSubdivisions != 0)
	{
		MakeIcosahedron();
	}
	CurrentSubdivisions = Iterations;

	while (Iterations > 0)
	{
		// make a copy of the existing triangles so we can iterate over them
		TArray<FPolygon> TrianglesCopy(Triangles);

		// make room for our new subdiveded triangles
		Triangles.Empty();

		for (FPolygon const &Tri : TrianglesCopy)
		{
			// find the middle point of the edges
			FVector A = FindMiddle(Tri.VertexIds[0], Tri.VertexIds[1]);
			FVector B = FindMiddle(Tri.VertexIds[1], Tri.VertexIds[2]);
			FVector C = FindMiddle(Tri.VertexIds[2], Tri.VertexIds[0]);
			int32 VertIdA = AddVertex(A.X, A.Y, A.Z);
			int32 VertIdB = AddVertex(B.X, B.Y, B.Z);
			int32 VertIdC = AddVertex(C.X, C.Y, C.Z);

			//generate 4 new triangles
			AddTriangle(Tri.VertexIds[0], VertIdA, VertIdC);
			AddTriangle(Tri.VertexIds[1], VertIdB, VertIdA);
			AddTriangle(Tri.VertexIds[2], VertIdC, VertIdB);
			AddTriangle(VertIdA, VertIdB, VertIdC);
		}

		Iterations--;
	}
}

void FIcoSphere::DrawDebug(const UWorld* World, const FTransform &Transform)
{
	FVector Center = Transform.GetLocation();
	FVector Scale = Transform.GetScale3D();

	FlushPersistentDebugLines(World);
	for (FVector &Vert : Vertices)
	{
		DrawDebugPoint(World, Center + (Vert * Scale), 10, FColor(255, 0, 0), true);
	}
	for (FPolygon &Tri : Triangles)
	{
		for (int32 i = 0; i < 3; i++)
		{
			FVector From = Center + (Vertices[Tri.VertexIds[i % 3]] * Scale);
			FVector To = Center + (Vertices[Tri.VertexIds[(i + 1) % 3]] * Scale);
			DrawDebugLine(World, From, To, FColor(0, 0, 255), true);
		}
	}
	for (FPolygon &Poly : Polygons)
	{
		int32 N = Poly.VertexIds.Num();
		for (int32 i = 0; i < N; i++)
		{
			FVector From = Center + (Vertices[Poly.VertexIds[i % N]] * Scale);
			FVector To = Center + (Vertices[Poly.VertexIds[(i + 1) % N]] * Scale);
			DrawDebugLine(World, From, To, FColor(0, 255, 0), true);
		}
	}
}

// Sets default values
ANavSphere::ANavSphere()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	RootComponent = SceneComponent;
}

// Called when the game starts or when spawned
void ANavSphere::BeginPlay()
{
	Super::BeginPlay();


}

// Called every frame
void ANavSphere::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void ANavSphere::OnConstruction(const FTransform &Transform)
{
	Super::OnConstruction(Transform);

	IcoSphere.SubDivide(3);
	IcoSphere.MakeDodecahedron();
	IcoSphere.DrawDebug(GetWorld(), GetTransform());
}

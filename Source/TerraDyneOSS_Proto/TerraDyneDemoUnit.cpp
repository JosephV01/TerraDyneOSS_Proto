#include "TerraDyneDemoUnit.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "TerraDyneGardenDemoHost.h"
#include "UObject/ConstructorHelpers.h"

ATerraDyneDemoUnit::ATerraDyneDemoUnit()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (MeshFinder.Succeeded())
	{
		Mesh->SetStaticMesh(MeshFinder.Object);
	}
	Mesh->SetWorldScale3D(FVector(1.2f, 1.2f, 1.8f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->MaxSpeed = MoveSpeed;
	AutoPossessAI = EAutoPossessAI::Disabled;
}

void ATerraDyneDemoUnit::BeginPlay()
{
	Super::BeginPlay();
	SnapToTerrain(GetActorLocation());
}

void ATerraDyneDemoUnit::SetSelected(bool bInSelected)
{
	bSelected = bInSelected;
	Mesh->SetVectorParameterValueOnMaterials(TEXT("Color"), bSelected ? FVector(0.1f, 1.f, 0.2f) : FVector(0.15f, 0.55f, 0.2f));
}

void ATerraDyneDemoUnit::OrderAt(const FVector& WorldLocation)
{
	OrderLocation = WorldLocation;
	bHasOrder = true;
	ApplyTimeRemaining = 0.f;
	IntervalAccumulator = 0.f;
}

void ATerraDyneDemoUnit::ClearOrder()
{
	bHasOrder = false;
	ApplyTimeRemaining = 0.f;
}

void ATerraDyneDemoUnit::SnapToTerrain(const FVector& Approximate)
{
	ATerraDyneGardenDemoHost* LiveHost = Host.Get();
	FVector Loc = Approximate;
	if (LiveHost && LiveHost->IsTerrainReady())
	{
		Loc.Z = LiveHost->SampleHeight(Approximate) + 90.f;
	}
	SetActorLocation(Loc, false, nullptr, ETeleportType::TeleportPhysics);
}

void ATerraDyneDemoUnit::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	TickOrder(DeltaSeconds);

	if (UWorld* World = GetWorld())
	{
		const FColor Color = ToolMode == ETerraDyneDemoToolMode::Lower ? FColor(255, 140, 40)
			: (ToolMode == ETerraDyneDemoToolMode::Paint ? FColor(40, 200, 255) : FColor(60, 255, 90));
		if (bHasOrder)
		{
			const ATerraDyneGardenDemoHost* LiveHost = Host.Get();
			const float Radius = LiveHost ? LiveHost->BrushRadiusUU : 1200.f;
			DrawDebugCircle(World, OrderLocation + FVector(0, 0, 20), Radius, 48, Color, false, -1.f, 0, 5.f, FVector(1, 0, 0), FVector(0, 1, 0), false);
			DrawDebugString(World, OrderLocation + FVector(0, 0, 120),
				ToolMode == ETerraDyneDemoToolMode::Lower ? TEXT("LOWER")
					: (ToolMode == ETerraDyneDemoToolMode::Paint ? TEXT("PAINT") : TEXT("RAISE")),
				nullptr, Color, 0.f, true, 1.2f);
		}
		if (bSelected)
		{
			DrawDebugCapsule(World, GetActorLocation(), 120.f, 50.f, FQuat::Identity, FColor::Yellow, false, -1.f, 0, 2.f);
		}
	}
}

void ATerraDyneDemoUnit::TickOrder(float DeltaSeconds)
{
	if (!bHasOrder)
	{
		SnapToTerrain(GetActorLocation());
		return;
	}

	ATerraDyneGardenDemoHost* LiveHost = Host.Get();
	FVector Loc = GetActorLocation();
	FVector To = OrderLocation - Loc;
	To.Z = 0.f;
	const float Dist = To.Size();
	if (Dist > InteractRadius)
	{
		const FVector Step = To.GetSafeNormal() * FMath::Min(Dist, MoveSpeed * DeltaSeconds);
		SnapToTerrain(Loc + Step);
		return;
	}

	if (ApplyTimeRemaining <= 0.f)
	{
		ApplyTimeRemaining = ApplyDurationSeconds;
		IntervalAccumulator = ApplyIntervalSeconds;
	}

	ApplyTimeRemaining -= DeltaSeconds;
	IntervalAccumulator += DeltaSeconds;
	if (LiveHost && IntervalAccumulator >= ApplyIntervalSeconds)
	{
		IntervalAccumulator = 0.f;
		bool bOk = false;
		switch (ToolMode)
		{
		case ETerraDyneDemoToolMode::Raise: bOk = LiveHost->ApplyRaise(OrderLocation); break;
		case ETerraDyneDemoToolMode::Lower: bOk = LiveHost->ApplyLower(OrderLocation); break;
		case ETerraDyneDemoToolMode::Paint: bOk = LiveHost->ApplyPaint(OrderLocation, PaintLayerIndex); break;
		}
		if (UWorld* World = GetWorld())
		{
			DrawDebugSphere(World, OrderLocation + FVector(0, 0, 40), bOk ? 70.f : 30.f, 10,
				bOk ? FColor::Emerald : FColor::Red, false, 0.35f, 0, 3.f);
		}
	}

	SnapToTerrain(OrderLocation);
	if (ApplyTimeRemaining <= 0.f)
	{
		ClearOrder();
	}
}

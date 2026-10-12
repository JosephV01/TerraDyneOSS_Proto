#include "TerraDyneGardenDemoGameMode.h"

#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "GameFramework/DefaultPawn.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "TerraDyneDemoPlayerController.h"
#include "TerraDyneDemoTypes.h"
#include "TerraDyneDemoUnit.h"
#include "TerraDyneGardenDemoHost.h"

ATerraDyneGardenDemoGameMode::ATerraDyneGardenDemoGameMode()
{
	PlayerControllerClass = ATerraDyneDemoPlayerController::StaticClass();
	DefaultPawnClass = ADefaultPawn::StaticClass();
	PrimaryActorTick.bCanEverTick = false;
}

void ATerraDyneGardenDemoGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATerraDyneGardenDemoHost* Host = World->SpawnActor<ATerraDyneGardenDemoHost>(
		ATerraDyneGardenDemoHost::StaticClass(), FTransform::Identity, Params);

	FVector Origin = FVector::ZeroVector;
	if (AActor* Start = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass()))
	{
		Origin = Start->GetActorLocation();
	}
	if (Host && Host->IsTerrainReady())
	{
		Origin.Z = Host->SampleHeight(Origin) + 90.f;
	}

	if (Host && FParse::Param(FCommandLine::Get(), TEXT("PaintSelfTest")))
	{
		Host->RequestPaintSelfTest();
	}
	if (Host && FParse::Param(FCommandLine::Get(), TEXT("LowerSelfTest")))
	{
		Host->RequestLowerSelfTest();
	}
	if (Host && FParse::Param(FCommandLine::Get(), TEXT("M4CompatSelfTest")))
	{
		Host->RequestM4CompatSelfTest();
	}

	ATerraDyneDemoUnit* Unit = World->SpawnActor<ATerraDyneDemoUnit>(
		ATerraDyneDemoUnit::StaticClass(), Origin, FRotator::ZeroRotator, Params);
	if (Unit && Host)
	{
		Unit->BindHost(Host);
		Unit->SetSelected(true);
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			FVector Cam = Origin + FVector(-1800.f, -1800.f, 2200.f);
			Pawn->SetActorLocation(Cam);
			PC->SetControlRotation((Origin - Cam).Rotation());
		}
		if (ATerraDyneDemoPlayerController* DemoPC = Cast<ATerraDyneDemoPlayerController>(PC))
		{
			if (Unit)
			{
				DemoPC->SetToolMode(ETerraDyneDemoToolMode::Raise);
			}
		}
	}
}

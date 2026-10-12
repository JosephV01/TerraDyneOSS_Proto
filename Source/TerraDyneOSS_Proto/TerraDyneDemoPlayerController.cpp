#include "TerraDyneDemoPlayerController.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TerraDyneDemoHUD.h"
#include "TerraDyneDemoUnit.h"
#include "TerraDyneGardenDemoHost.h"

ATerraDyneDemoPlayerController::ATerraDyneDemoPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	PrimaryActorTick.bCanEverTick = true;
}

void ATerraDyneDemoPlayerController::BeginPlay()
{
	Super::BeginPlay();
	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ATerraDyneGardenDemoHost> It(World); It; ++It)
		{
			Host = *It;
			break;
		}
	}

	HUDWidget = CreateWidget<UTerraDyneDemoHUD>(this, UTerraDyneDemoHUD::StaticClass());
	if (HUDWidget)
	{
		HUDWidget->BindController(this);
		HUDWidget->AddToViewport(100);
	}

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ATerraDyneDemoUnit> It(World); It; ++It)
		{
			SelectedUnit = *It;
			It->SetSelected(true);
			It->SetToolMode(ToolMode);
			It->SetPaintLayer(PaintLayerIndex);
			break;
		}
	}
}

void ATerraDyneDemoPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ATerraDyneDemoPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (!IsLocalController())
	{
		return;
	}
	if (!Host.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<ATerraDyneGardenDemoHost> It(World); It; ++It)
			{
				Host = *It;
				break;
			}
		}
	}
	if (!SelectedUnit.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<ATerraDyneDemoUnit> It(World); It; ++It)
			{
				SelectedUnit = *It;
				It->SetSelected(true);
				It->SetToolMode(ToolMode);
				It->SetPaintLayer(PaintLayerIndex);
				It->BindHost(Host.Get());
				break;
			}
		}
	}
	if (WasInputKeyJustPressed(EKeys::R))
	{
		SetToolMode(ETerraDyneDemoToolMode::Raise);
	}
	if (WasInputKeyJustPressed(EKeys::F))
	{
		SetToolMode(ETerraDyneDemoToolMode::Lower);
	}
	if (WasInputKeyJustPressed(EKeys::T))
	{
		SetToolMode(ETerraDyneDemoToolMode::Paint);
	}
	if (WasInputKeyJustPressed(EKeys::G))
	{
		CyclePaintLayer();
	}
	if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		HandleSelect();
	}
	if (WasInputKeyJustPressed(EKeys::RightMouseButton))
	{
		HandleCommand();
	}
}

void ATerraDyneDemoPlayerController::SetToolMode(ETerraDyneDemoToolMode Mode)
{
	ToolMode = Mode;
	if (ATerraDyneDemoUnit* Unit = SelectedUnit.Get())
	{
		Unit->SetToolMode(Mode);
	}
}

void ATerraDyneDemoPlayerController::SetPaintLayer(int32 Index)
{
	PaintLayerIndex = FMath::Clamp(Index, 0, TerraDyneDemoLayers::Count() - 1);
	if (ATerraDyneDemoUnit* Unit = SelectedUnit.Get())
	{
		Unit->SetPaintLayer(PaintLayerIndex);
	}
}

void ATerraDyneDemoPlayerController::CyclePaintLayer()
{
	SetPaintLayer((PaintLayerIndex + 1) % TerraDyneDemoLayers::Count());
	SetToolMode(ETerraDyneDemoToolMode::Paint);
}

void ATerraDyneDemoPlayerController::RequestM4Test()
{
	if (ATerraDyneGardenDemoHost* LiveHost = Host.Get())
	{
		LiveHost->TryApplyM4Material();
	}
}

void ATerraDyneDemoPlayerController::RequestPathBTest()
{
	if (ATerraDyneGardenDemoHost* LiveHost = Host.Get())
	{
		LiveHost->TryApplyTerraDyneMasterMaterial();
	}
}

void ATerraDyneDemoPlayerController::RequestRestoreDemoMaterial()
{
	if (ATerraDyneGardenDemoHost* LiveHost = Host.Get())
	{
		LiveHost->RestoreDemoOverlayMaterial();
	}
}

bool ATerraDyneDemoPlayerController::TraceUnderCursor(FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}
	FVector Origin, Dir;
	if (!DeprojectMousePositionToWorld(Origin, Dir))
	{
		return false;
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DemoCursor), true);
	if (GetPawn())
	{
		Params.AddIgnoredActor(GetPawn());
	}
	return World->LineTraceSingleByChannel(OutHit, Origin, Origin + Dir * 200000.f, ECC_Visibility, Params)
		|| World->LineTraceSingleByChannel(OutHit, Origin, Origin + Dir * 200000.f, ECC_WorldStatic, Params);
}

void ATerraDyneDemoPlayerController::HandleSelect()
{
	FHitResult Hit;
	if (!TraceUnderCursor(Hit))
	{
		return;
	}
	if (ATerraDyneDemoUnit* Unit = Cast<ATerraDyneDemoUnit>(Hit.GetActor()))
	{
		if (ATerraDyneDemoUnit* Prev = SelectedUnit.Get())
		{
			Prev->SetSelected(false);
		}
		SelectedUnit = Unit;
		Unit->SetSelected(true);
		Unit->SetToolMode(ToolMode);
		Unit->SetPaintLayer(PaintLayerIndex);
	}
}

void ATerraDyneDemoPlayerController::HandleCommand()
{
	FHitResult Hit;
	if (!TraceUnderCursor(Hit))
	{
		return;
	}
	ATerraDyneDemoUnit* Unit = SelectedUnit.Get();
	if (!Unit)
	{
		return;
	}
	FVector Loc = Hit.ImpactPoint;
	if (ATerraDyneGardenDemoHost* LiveHost = Host.Get())
	{
		Loc.Z = LiveHost->SampleHeight(Loc);
	}
	Unit->OrderAt(Loc);
}

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TerraDyneDemoTypes.h"
#include "TerraDyneDemoPlayerController.generated.h"

class ATerraDyneDemoUnit;
class ATerraDyneGardenDemoHost;
class UTerraDyneDemoHUD;

UCLASS()
class TERRADYNEOSS_PROTO_API ATerraDyneDemoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATerraDyneDemoPlayerController();

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	void SetToolMode(ETerraDyneDemoToolMode Mode);
	void SetPaintLayer(int32 Index);
	void CyclePaintLayer();
	void RequestM4Test();
	void RequestPathBTest();
	void RequestRestoreDemoMaterial();
	ATerraDyneDemoUnit* GetSelectedUnit() const { return SelectedUnit.Get(); }
	ATerraDyneGardenDemoHost* GetHost() const { return Host.Get(); }
	ETerraDyneDemoToolMode GetToolMode() const { return ToolMode; }
	int32 GetPaintLayer() const { return PaintLayerIndex; }

protected:
	bool TraceUnderCursor(FHitResult& OutHit) const;
	void HandleSelect();
	void HandleCommand();

	TWeakObjectPtr<ATerraDyneDemoUnit> SelectedUnit;
	TWeakObjectPtr<ATerraDyneGardenDemoHost> Host;
	UPROPERTY()
	TObjectPtr<UTerraDyneDemoHUD> HUDWidget;
	ETerraDyneDemoToolMode ToolMode = ETerraDyneDemoToolMode::Raise;
	int32 PaintLayerIndex = 1;
};

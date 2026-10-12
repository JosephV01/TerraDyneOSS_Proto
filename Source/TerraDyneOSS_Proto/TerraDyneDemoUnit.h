#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TerraDyneDemoTypes.h"
#include "TerraDyneDemoUnit.generated.h"

class UFloatingPawnMovement;
class UStaticMeshComponent;
class ATerraDyneGardenDemoHost;

UCLASS()
class TERRADYNEOSS_PROTO_API ATerraDyneDemoUnit : public APawn
{
	GENERATED_BODY()

public:
	ATerraDyneDemoUnit();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetSelected(bool bInSelected);
	bool IsSelected() const { return bSelected; }
	void SetToolMode(ETerraDyneDemoToolMode InMode) { ToolMode = InMode; }
	ETerraDyneDemoToolMode GetToolMode() const { return ToolMode; }
	void SetPaintLayer(int32 Index) { PaintLayerIndex = Index; }
	int32 GetPaintLayer() const { return PaintLayerIndex; }
	void OrderAt(const FVector& WorldLocation);
	void ClearOrder();
	void BindHost(ATerraDyneGardenDemoHost* InHost) { Host = InHost; }

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UFloatingPawnMovement> Movement;

protected:
	void SnapToTerrain(const FVector& Approximate);
	void TickOrder(float DeltaSeconds);

	TWeakObjectPtr<ATerraDyneGardenDemoHost> Host;
	bool bSelected = false;
	bool bHasOrder = false;
	FVector OrderLocation = FVector::ZeroVector;
	ETerraDyneDemoToolMode ToolMode = ETerraDyneDemoToolMode::Raise;
	int32 PaintLayerIndex = 1;
	float ApplyTimeRemaining = 0.f;
	float IntervalAccumulator = 0.f;
	float InteractRadius = 220.f;
	float ApplyDurationSeconds = 2.f;
	float ApplyIntervalSeconds = 0.2f;
	float MoveSpeed = 700.f;
};

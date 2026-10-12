#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerraDyneDemoTypes.h"
#include "TerraDyneGardenDemoHost.generated.h"

class ATerraDyneManager;
class ATerraDyneChunk;
class UTerraDyneDemoPaintStore;
class UMaterialInterface;

UCLASS()
class TERRADYNEOSS_PROTO_API ATerraDyneGardenDemoHost : public AActor
{
	GENERATED_BODY()

public:
	ATerraDyneGardenDemoHost();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	bool IsTerrainReady() const;
	bool IsDisplayMeshReady() const;
	void RequestPaintSelfTest() { bPendingPaintSelfTest = true; }
	void RequestLowerSelfTest() { bPendingLowerSelfTest = true; }
	void RequestM4CompatSelfTest() { bPendingM4CompatSelfTest = true; }
	bool ApplyRaise(const FVector& WorldLocation);
	bool ApplyLower(const FVector& WorldLocation);
	bool ApplyPaint(const FVector& WorldLocation, int32 LayerIndex);

	float SampleHeight(const FVector& WorldLocation) const;
	bool TraceCollisionZ(const FVector& WorldLocation, float& OutZ, FString& OutName) const;
	bool WorldToNormalized(const FVector& WorldLocation, float& OutU, float& OutV) const;
	FLinearColor SamplePaintColor(const FVector& WorldLocation) const;
	FString TryApplyM4Material();
	FString GetStatusLine() const;
	void RunPaintSelfTest();
	void RunLowerSelfTest();
	void RunM4CompatSelfTest();

	ATerraDyneManager* GetManager() const { return Manager.Get(); }
	ATerraDyneChunk* GetChunk() const { return Chunk.Get(); }
	UTerraDyneDemoPaintStore* GetPaintStore() const { return PaintStore; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	float ChunkWorldSize = 13652.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	int32 ChunkResolution = 128;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	float BrushRadiusUU = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	float BrushStrengthUU = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Demo")
	float PaintStrength = 0.9f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Demo")
	FString LastOperation = TEXT("Idle");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Demo")
	FString M4Verdict = TEXT("Not attempted");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Demo")
	double LastBrushSeconds = 0.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Demo")
	double LastMeshWaitSeconds = 0.0;

protected:
	bool SpawnTerrain();
	void RecolorMeshFromPaint();
	void SyncTerraDyneWeightBuffers();
	void ApplyFallbackMaterial();
	void EnablePaintColorDisplay();
	void HideTemplateFloor();

	TWeakObjectPtr<ATerraDyneManager> Manager;
	TWeakObjectPtr<ATerraDyneChunk> Chunk;

	UPROPERTY()
	TObjectPtr<UTerraDyneDemoPaintStore> PaintStore;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> FallbackMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> AppliedMeshMaterial;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> VertexColorDisplayMaterial;

	int32 LastColoredMeshId = 0;
	bool bForceRecolor = true;
	bool bM4Tried = false;
	bool bUseNativeM4Material = false;
	bool bPendingPaintSelfTest = false;
	bool bPendingLowerSelfTest = false;
	bool bPendingM4CompatSelfTest = false;

	void AppendM4MaterialDiagnostics(FString& OutLog, UMaterialInterface* Material) const;
};

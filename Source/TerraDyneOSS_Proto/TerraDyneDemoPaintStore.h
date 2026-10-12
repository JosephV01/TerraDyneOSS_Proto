#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TerraDyneDemoPaintStore.generated.h"

class UTextureRenderTarget2D;

UCLASS()
class TERRADYNEOSS_PROTO_API UTerraDyneDemoPaintStore : public UObject
{
	GENERATED_BODY()

public:
	void Initialize(int32 InResolution);
	bool IsReady() const { return bReady; }

	bool ApplyPaint(float NormalizedX, float NormalizedY, float RadiusNorm, int32 LayerIndex, float Strength);
	FLinearColor SampleColor(float NormalizedX, float NormalizedY) const;
	uint8 SampleWeight(int32 LayerIndex, float NormalizedX, float NormalizedY) const;
	int32 DominantLayer(float NormalizedX, float NormalizedY) const;
	void FillLayer(int32 LayerIndex, uint8 Value);
	void UploadToRT();

	UTextureRenderTarget2D* GetWeightRT0() const { return WeightRT0; }
	UTextureRenderTarget2D* GetWeightRT1() const { return WeightRT1; }
	UTextureRenderTarget2D* GetWeightRT2() const { return WeightRT2; }

	int32 GetResolution() const { return Resolution; }

private:
	int32 IndexAt(int32 X, int32 Y, int32 Layer) const;
	uint8* LayerData(int32 Layer);
	const uint8* LayerData(int32 Layer) const;

	int32 Resolution = 0;
	bool bReady = false;
	TArray<uint8> Weights;
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> WeightRT0;
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> WeightRT1;
	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> WeightRT2;
};

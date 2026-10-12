// Isolated prototype stub — TerraDyneOSS_Proto_270c311 / OSS commit 270c311.
// The public tree does not contain this header (Strip Fab features).
// Dispatch returning false matches ApplyBrushGPU's existing failure path
// (TerraDyneChunk.cpp), which then applies CPU layer edits.
// GPU simulation is not available and is not claimed.

#pragma once

#include "CoreMinimal.h"
#include "World/TerraDyneTileData.h"

class UTextureRenderTarget2D;

struct FTerraDyneSimulationBrushParams
{
	FVector2f BrushCenterUV = FVector2f::ZeroVector;
	float BrushRadiusUV = 0.0f;
	float BrushStrengthNormalized = 0.0f;
	float FlattenHeightNormalized = 0.0f;
	int32 Resolution = 0;
	ETerraDyneBrushMode BrushMode = ETerraDyneBrushMode::Raise;
};

inline bool DispatchTerraDyneBrushSimulation(
	UTextureRenderTarget2D* /*HeightRT*/,
	UTextureRenderTarget2D* /*HeightRT_Swap*/,
	const FTerraDyneSimulationBrushParams& /*Params*/)
{
	return false;
}

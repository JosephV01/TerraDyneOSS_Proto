#pragma once

#include "CoreMinimal.h"
#include "TerraDyneDemoTypes.generated.h"

UENUM(BlueprintType)
enum class ETerraDyneDemoToolMode : uint8
{
	Raise UMETA(DisplayName = "Raise"),
	Lower UMETA(DisplayName = "Lower"),
	Paint UMETA(DisplayName = "Paint")
};

namespace TerraDyneDemoLayers
{
	inline const TArray<FName>& Names()
	{
		static const TArray<FName> LayerNames = {
			FName(TEXT("Auto")),
			FName(TEXT("Grass")),
			FName(TEXT("Dirt")),
			FName(TEXT("Sand")),
			FName(TEXT("Mud")),
			FName(TEXT("Forest")),
			FName(TEXT("Stones")),
			FName(TEXT("Jagged_Rock")),
			FName(TEXT("Flat_Ice")),
			FName(TEXT("Custom01")),
			FName(TEXT("Custom02"))
		};
		return LayerNames;
	}

	inline int32 Count() { return Names().Num(); }

	inline FLinearColor Color(int32 Index)
	{
		static const FLinearColor Palette[] = {
			FLinearColor(0.18f, 0.42f, 0.16f), // Auto
			FLinearColor(0.20f, 0.70f, 0.18f), // Grass
			FLinearColor(0.45f, 0.28f, 0.12f), // Dirt
			FLinearColor(0.86f, 0.74f, 0.32f), // Sand
			FLinearColor(0.22f, 0.16f, 0.10f), // Mud
			FLinearColor(0.06f, 0.28f, 0.08f), // Forest
			FLinearColor(0.55f, 0.55f, 0.58f), // Stones
			FLinearColor(0.62f, 0.28f, 0.42f), // Jagged_Rock
			FLinearColor(0.78f, 0.88f, 0.95f), // Flat_Ice
			FLinearColor(0.90f, 0.40f, 0.12f), // Custom01
			FLinearColor(0.28f, 0.42f, 0.90f)  // Custom02
		};
		const int32 Safe = FMath::Clamp(Index, 0, UE_ARRAY_COUNT(Palette) - 1);
		return Palette[Safe];
	}
}

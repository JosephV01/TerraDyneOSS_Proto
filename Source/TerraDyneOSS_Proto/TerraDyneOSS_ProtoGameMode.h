#pragma once

#include "CoreMinimal.h"
#include "TerraDyneGardenDemoGameMode.h"
#include "TerraDyneOSS_ProtoGameMode.generated.h"

/** Compatibility alias. Playable demo is ATerraDyneGardenDemoGameMode. Phase 2B lives in ATerraDyneOSS_Phase2BGameMode. */
UCLASS()
class TERRADYNEOSS_PROTO_API ATerraDyneOSS_ProtoGameMode : public ATerraDyneGardenDemoGameMode
{
	GENERATED_BODY()
};

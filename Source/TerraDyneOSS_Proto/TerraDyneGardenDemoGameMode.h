#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TerraDyneGardenDemoGameMode.generated.h"

UCLASS()
class TERRADYNEOSS_PROTO_API ATerraDyneGardenDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATerraDyneGardenDemoGameMode();

protected:
	virtual void BeginPlay() override;
};

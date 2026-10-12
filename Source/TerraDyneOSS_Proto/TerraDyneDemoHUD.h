#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TerraDyneDemoHUD.generated.h"

class ATerraDyneDemoPlayerController;
class UButton;
class UTextBlock;
class UCanvasPanel;
class UHorizontalBox;

UCLASS()
class TERRADYNEOSS_PROTO_API UTerraDyneDemoHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void BindController(ATerraDyneDemoPlayerController* InPC);

protected:
	UButton* AddButton(UHorizontalBox* Row, const FString& Label, FLinearColor Color);
	UFUNCTION() void OnRaise();
	UFUNCTION() void OnLower();
	UFUNCTION() void OnPaintMode();
	UFUNCTION() void OnCycleLayer();
	UFUNCTION() void OnM4();
	UFUNCTION() void OnPathB();
	UFUNCTION() void OnRestoreOverlay();
	UFUNCTION() void OnLayer0();
	UFUNCTION() void OnLayer1();
	UFUNCTION() void OnLayer2();
	UFUNCTION() void OnLayer3();
	UFUNCTION() void OnLayer4();
	UFUNCTION() void OnLayer5();
	UFUNCTION() void OnLayer6();
	UFUNCTION() void OnLayer7();
	UFUNCTION() void OnLayer8();
	UFUNCTION() void OnLayer9();
	UFUNCTION() void OnLayer10();
	void SelectLayer(int32 Index);

	TWeakObjectPtr<ATerraDyneDemoPlayerController> PC;
	UPROPERTY() TObjectPtr<UTextBlock> StatusText;
	UPROPERTY() TObjectPtr<UTextBlock> MeasureText;
	UPROPERTY() TObjectPtr<UTextBlock> HelpText;
	TArray<TObjectPtr<UButton>> LayerButtons;
};

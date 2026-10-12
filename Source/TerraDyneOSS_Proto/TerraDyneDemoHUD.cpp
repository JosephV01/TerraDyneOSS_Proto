#include "TerraDyneDemoHUD.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "TerraDyneDemoPaintStore.h"
#include "TerraDyneDemoPlayerController.h"
#include "TerraDyneDemoTypes.h"
#include "TerraDyneDemoUnit.h"
#include "TerraDyneGardenDemoHost.h"

void UTerraDyneDemoHUD::BindController(ATerraDyneDemoPlayerController* InPC)
{
	PC = InPC;
}

void UTerraDyneDemoHUD::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!WidgetTree)
	{
		return;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Box"));
	if (UCanvasPanelSlot* CanvasSlot = Root->AddChildToCanvas(Box))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
		CanvasSlot->SetOffsets(FMargin(16.f, 16.f, 16.f, 0.f));
		CanvasSlot->SetAutoSize(true);
	}

	auto AddLabel = [&](const FString& Name, int32 Size) -> UTextBlock*
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *Name);
		Text->SetAutoWrapText(true);
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = Size;
		Text->SetFont(Font);
		Box->AddChildToVerticalBox(Text);
		return Text;
	};

	StatusText = AddLabel(TEXT("Status"), 16);
	MeasureText = AddLabel(TEXT("Measure"), 15);
	HelpText = AddLabel(TEXT("Help"), 14);
	HelpText->SetText(FText::FromString(TEXT(
		"Play-ready: clic gauche = selectionner le Terraformer. Clic droit = ordre Raise/Lower/Paint.\n"
		"R Raise · F Lower · T Paint · G couche suivante. Attendre ~0.2s pour juger la collision.\n"
		"Auto = couche de base (vert). Les 10 autres = remplacement couleur, pas M4.\n"
		"Sequence A: Paint -> Raise -> Lower. Sequence B: Raise -> Paint -> Raise.\n"
		"Relancer PIE remet le chunk a plat (pas de persist inventee). show Collision = optionnel.")));

	UHorizontalBox* Modes = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Modes"));
	Box->AddChildToVerticalBox(Modes);
	AddButton(Modes, TEXT("[R] Raise"), FLinearColor(0.12f, 0.4f, 0.18f))->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnRaise);
	AddButton(Modes, TEXT("[F] Lower"), FLinearColor(0.4f, 0.22f, 0.08f))->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLower);
	AddButton(Modes, TEXT("[T] Paint"), FLinearColor(0.08f, 0.28f, 0.4f))->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnPaintMode);
	AddButton(Modes, TEXT("[G] Next layer"), FLinearColor(0.2f, 0.2f, 0.25f))->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnCycleLayer);
	AddButton(Modes, TEXT("Test M4 MIC"), FLinearColor(0.35f, 0.12f, 0.35f))->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnM4);
	AddButton(Modes, TEXT("Test TD Master"), FLinearColor(0.15f, 0.35f, 0.45f))->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnPathB);
	AddButton(Modes, TEXT("Restore overlay"), FLinearColor(0.25f, 0.25f, 0.2f))->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnRestoreOverlay);

	UHorizontalBox* Layers = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Layers"));
	Box->AddChildToVerticalBox(Layers);
	UButton* B0 = AddButton(Layers, TerraDyneDemoLayers::Names()[0].ToString(), TerraDyneDemoLayers::Color(0));
	UButton* B1 = AddButton(Layers, TerraDyneDemoLayers::Names()[1].ToString(), TerraDyneDemoLayers::Color(1));
	UButton* B2 = AddButton(Layers, TerraDyneDemoLayers::Names()[2].ToString(), TerraDyneDemoLayers::Color(2));
	UButton* B3 = AddButton(Layers, TerraDyneDemoLayers::Names()[3].ToString(), TerraDyneDemoLayers::Color(3));
	UButton* B4 = AddButton(Layers, TerraDyneDemoLayers::Names()[4].ToString(), TerraDyneDemoLayers::Color(4));
	UButton* B5 = AddButton(Layers, TerraDyneDemoLayers::Names()[5].ToString(), TerraDyneDemoLayers::Color(5));
	UButton* B6 = AddButton(Layers, TerraDyneDemoLayers::Names()[6].ToString(), TerraDyneDemoLayers::Color(6));
	UButton* B7 = AddButton(Layers, TerraDyneDemoLayers::Names()[7].ToString(), TerraDyneDemoLayers::Color(7));
	UButton* B8 = AddButton(Layers, TerraDyneDemoLayers::Names()[8].ToString(), TerraDyneDemoLayers::Color(8));
	UButton* B9 = AddButton(Layers, TerraDyneDemoLayers::Names()[9].ToString(), TerraDyneDemoLayers::Color(9));
	UButton* B10 = AddButton(Layers, TerraDyneDemoLayers::Names()[10].ToString(), TerraDyneDemoLayers::Color(10));
	LayerButtons.Append({B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10});
	B0->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer0);
	B1->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer1);
	B2->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer2);
	B3->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer3);
	B4->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer4);
	B5->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer5);
	B6->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer6);
	B7->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer7);
	B8->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer8);
	B9->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer9);
	B10->OnClicked.AddDynamic(this, &UTerraDyneDemoHUD::OnLayer10);
}

UButton* UTerraDyneDemoHUD::AddButton(UHorizontalBox* Row, const FString& Label, FLinearColor Color)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	FButtonStyle Style = Button->GetStyle();
	Style.Normal.TintColor = FSlateColor(Color);
	Style.Hovered.TintColor = FSlateColor(Color + FLinearColor(0.1f, 0.1f, 0.1f));
	Button->SetStyle(Style);
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(FText::FromString(Label));
	Button->AddChild(Text);
	if (UHorizontalBoxSlot* BoxSlot = Row->AddChildToHorizontalBox(Button))
	{
		BoxSlot->SetPadding(FMargin(4.f));
	}
	return Button;
}

void UTerraDyneDemoHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	ATerraDyneDemoPlayerController* LivePC = PC.Get();
	ATerraDyneGardenDemoHost* Host = LivePC ? LivePC->GetHost() : nullptr;
	if (!StatusText || !Host)
	{
		return;
	}

	const TCHAR* Mode = TEXT("Raise");
	switch (LivePC->GetToolMode())
	{
	case ETerraDyneDemoToolMode::Lower: Mode = TEXT("Lower"); break;
	case ETerraDyneDemoToolMode::Paint: Mode = TEXT("Paint"); break;
	default: break;
	}
	const int32 Layer = LivePC->GetPaintLayer();
	const FString LayerName = TerraDyneDemoLayers::Names()[FMath::Clamp(Layer, 0, TerraDyneDemoLayers::Count() - 1)].ToString();
	StatusText->SetText(FText::FromString(FString::Printf(
		TEXT("Mode %s | Couche %s (%d) | r=%.0f dH=%.0f | %s"),
		Mode, *LayerName, Layer, Host->BrushRadiusUU, Host->BrushStrengthUU, *Host->GetStatusLine())));

	FVector Probe = FVector::ZeroVector;
	if (ATerraDyneDemoUnit* Unit = LivePC->GetSelectedUnit())
	{
		Probe = Unit->GetActorLocation();
	}
	float ColZ = 0.f;
	FString ColName;
	const float ApiZ = Host->SampleHeight(Probe);
	const bool bCol = Host->TraceCollisionZ(Probe, ColZ, ColName);
	int32 PaintIdx = 0;
	if (Host->GetPaintStore() && Host->GetPaintStore()->IsReady())
	{
		float U = 0.f, V = 0.f;
		if (Host->WorldToNormalized(Probe, U, V))
		{
			PaintIdx = Host->GetPaintStore()->DominantLayer(U, V);
		}
	}
	if (MeasureText)
	{
		MeasureText->SetText(FText::FromString(FString::Printf(
			TEXT("Mesure sous unite: API TerraDyne Z=%.1f | Collision %s Z=%.1f (%s) | delta=%.1f | paint=%s"),
			ApiZ, bCol ? TEXT("HIT") : TEXT("MISS"), ColZ, *ColName, bCol ? (ColZ - ApiZ) : 0.f,
			*TerraDyneDemoLayers::Names()[FMath::Clamp(PaintIdx, 0, TerraDyneDemoLayers::Count() - 1)].ToString())));
	}
}

void UTerraDyneDemoHUD::OnRaise() { if (ATerraDyneDemoPlayerController* P = PC.Get()) P->SetToolMode(ETerraDyneDemoToolMode::Raise); }
void UTerraDyneDemoHUD::OnLower() { if (ATerraDyneDemoPlayerController* P = PC.Get()) P->SetToolMode(ETerraDyneDemoToolMode::Lower); }
void UTerraDyneDemoHUD::OnPaintMode() { if (ATerraDyneDemoPlayerController* P = PC.Get()) P->SetToolMode(ETerraDyneDemoToolMode::Paint); }
void UTerraDyneDemoHUD::OnCycleLayer() { if (ATerraDyneDemoPlayerController* P = PC.Get()) P->CyclePaintLayer(); }
void UTerraDyneDemoHUD::OnM4() { if (ATerraDyneDemoPlayerController* P = PC.Get()) P->RequestM4Test(); }
void UTerraDyneDemoHUD::OnPathB() { if (ATerraDyneDemoPlayerController* P = PC.Get()) P->RequestPathBTest(); }
void UTerraDyneDemoHUD::OnRestoreOverlay() { if (ATerraDyneDemoPlayerController* P = PC.Get()) P->RequestRestoreDemoMaterial(); }
void UTerraDyneDemoHUD::SelectLayer(int32 Index)
{
	if (ATerraDyneDemoPlayerController* P = PC.Get())
	{
		P->SetPaintLayer(Index);
		P->SetToolMode(ETerraDyneDemoToolMode::Paint);
	}
}
void UTerraDyneDemoHUD::OnLayer0() { SelectLayer(0); }
void UTerraDyneDemoHUD::OnLayer1() { SelectLayer(1); }
void UTerraDyneDemoHUD::OnLayer2() { SelectLayer(2); }
void UTerraDyneDemoHUD::OnLayer3() { SelectLayer(3); }
void UTerraDyneDemoHUD::OnLayer4() { SelectLayer(4); }
void UTerraDyneDemoHUD::OnLayer5() { SelectLayer(5); }
void UTerraDyneDemoHUD::OnLayer6() { SelectLayer(6); }
void UTerraDyneDemoHUD::OnLayer7() { SelectLayer(7); }
void UTerraDyneDemoHUD::OnLayer8() { SelectLayer(8); }
void UTerraDyneDemoHUD::OnLayer9() { SelectLayer(9); }
void UTerraDyneDemoHUD::OnLayer10() { SelectLayer(10); }

#include "TerraDyneGardenDemoHost.h"

#include "CollisionQueryParams.h"
#include "Components/BaseDynamicMeshComponent.h"
#include "Core/TerraDyneManager.h"
#include "Core/TerraDyneSubsystem.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "DynamicMesh/DynamicMeshOverlay.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/SoftObjectPath.h"
#include "Settings/TerraDyneSettings.h"
#include "TerraDyneDemoPaintStore.h"
#include "UDynamicMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "World/TerraDyneChunk.h"
#include "World/TerraDyneTileData.h"

DEFINE_LOG_CATEGORY_STATIC(LogTerraDyneGardenDemo, Log, All);

ATerraDyneGardenDemoHost::ATerraDyneGardenDemoHost()
{
	PrimaryActorTick.bCanEverTick = true;

	// Hard reference so the cooker keeps VertexColorViewMode even though cooked
	// games skip GEngine debug-viewmode materials (AllowDebugViewmodes == false).
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> VertexColorMat(
		TEXT("/Engine/EngineDebugMaterials/VertexColorViewMode_ColorOnly.VertexColorViewMode_ColorOnly"));
	if (VertexColorMat.Succeeded())
	{
		VertexColorDisplayMaterial = VertexColorMat.Object;
	}
}

void ATerraDyneGardenDemoHost::BeginPlay()
{
	Super::BeginPlay();
	PaintStore = NewObject<UTerraDyneDemoPaintStore>(this);
	SpawnTerrain();
}

void ATerraDyneGardenDemoHost::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RecolorMeshFromPaint();
	if (bPendingPaintSelfTest && IsDisplayMeshReady())
	{
		bPendingPaintSelfTest = false;
		RunPaintSelfTest();
	}
	if (bPendingLowerSelfTest && IsTerrainReady())
	{
		bPendingLowerSelfTest = false;
		RunLowerSelfTest();
	}
	if (bPendingM4CompatSelfTest && IsTerrainReady())
	{
		bPendingM4CompatSelfTest = false;
		RunM4CompatSelfTest();
	}
}

bool ATerraDyneGardenDemoHost::SpawnTerrain()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	HideTemplateFloor();

	FActorSpawnParameters ManagerParams;
	ManagerParams.bDeferConstruction = true;
	ManagerParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATerraDyneManager* NewManager = World->SpawnActor<ATerraDyneManager>(
		ATerraDyneManager::StaticClass(), FTransform::Identity, ManagerParams);
	if (!NewManager)
	{
		LastOperation = TEXT("FAIL: manager spawn");
		return false;
	}

	NewManager->GlobalChunkSize = ChunkWorldSize;
	NewManager->bSpawnDefaultChunksOnBeginPlay = false;
	NewManager->bStreamingPaused = true;
	NewManager->ProceduralWorldSettings.bEnableSeededOutskirts = false;
	NewManager->FinishSpawning(FTransform::Identity);
	Manager = NewManager;

	FActorSpawnParameters ChunkParams;
	ChunkParams.bDeferConstruction = true;
	ChunkParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ATerraDyneChunk* NewChunk = World->SpawnActor<ATerraDyneChunk>(
		ATerraDyneChunk::StaticClass(), FTransform::Identity, ChunkParams);
	if (!NewChunk)
	{
		LastOperation = TEXT("FAIL: chunk spawn");
		return false;
	}

	const FIntPoint Coord(0, 0);
	NewChunk->GridCoordinate = Coord;
	NewChunk->WorldSize = ChunkWorldSize;
	NewChunk->ChunkSizeWorldUnits = ChunkWorldSize;
	NewChunk->Resolution = ChunkResolution;
	NewChunk->FinishSpawning(FTransform::Identity);
	NewChunk->InitializeChunk(Coord, ChunkWorldSize, ChunkResolution, nullptr, nullptr);
	if (NewManager->GetChunkAtCoord(Coord) != NewChunk)
	{
		NewManager->RegisterChunk(NewChunk);
	}
	Chunk = NewChunk;

	if (PaintStore)
	{
		PaintStore->Initialize(ChunkResolution);
	}
	ApplyFallbackMaterial();
	RecolorMeshFromPaint();

	LastOperation = FString::Printf(
		TEXT("Terrain ready  size=%.0f res=%d step=%.1f GPU=off"),
		ChunkWorldSize, ChunkResolution,
		ChunkResolution > 1 ? ChunkWorldSize / static_cast<float>(ChunkResolution - 1) : 0.f);
	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("%s"), *LastOperation);
	return true;
}

void ATerraDyneGardenDemoHost::ApplyFallbackMaterial()
{
	bUseNativeM4Material = false;
	ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk)
	{
		return;
	}

	UMaterialInterface* Mat = VertexColorDisplayMaterial;
	if (!Mat)
	{
		Mat = LoadObject<UMaterialInterface>(
			nullptr, TEXT("/Engine/EngineDebugMaterials/VertexColorViewMode_ColorOnly.VertexColorViewMode_ColorOnly"));
	}
	if (!Mat)
	{
		Mat = LoadObject<UMaterialInterface>(
			nullptr, TEXT("/TerraDyne/Materials/VHFM/M_TerraDyne_Master.M_TerraDyne_Master"));
	}
	if (!Mat)
	{
		Mat = UMaterial::GetDefaultMaterial(MD_Surface);
	}
	FallbackMaterial = Mat;
	AppliedMeshMaterial = Mat;
	LiveChunk->SetMaterial(Mat);
	EnablePaintColorDisplay();
}

void ATerraDyneGardenDemoHost::EnablePaintColorDisplay()
{
	ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk || !LiveChunk->DynamicMeshComp)
	{
		return;
	}

	// ColorOverrideMode::VertexColors uses GEngine->VertexColorViewModeMaterial_ColorOnly via
	// GetDefaultVertexColorMaterial_RenderThread()->GetRenderProxy() with no null check.
	// Cooked games set AllowDebugViewmodes() false (RequiresCookedData), so that material
	// is never loaded and Shipping D3D11/D3D12 crash on the first gather. Keep overlay
	// colors on the mesh material instead of the engine color-override path.
	LiveChunk->DynamicMeshComp->SetColorOverrideMode(EDynamicMeshComponentColorOverrideMode::None);
}

void ATerraDyneGardenDemoHost::HideTemplateFloor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	int32 Hidden = 0;
	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		AStaticMeshActor* FloorActor = *It;
		if (!FloorActor)
		{
			continue;
		}
		const UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent();
		const UStaticMesh* Mesh = MeshComp ? MeshComp->GetStaticMesh() : nullptr;
		const FString ActorName = FloorActor->GetName();
		const FString MeshName = Mesh ? Mesh->GetName() : FString();
		if (!ActorName.Contains(TEXT("Floor")) && !MeshName.Contains(TEXT("Template_Map_Floor")))
		{
			continue;
		}
		FloorActor->SetActorHiddenInGame(true);
		FloorActor->SetActorEnableCollision(false);
		++Hidden;
	}
	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("Hidden %d template floor actor(s) so Lower can show below Z=0"), Hidden);
}

bool ATerraDyneGardenDemoHost::IsTerrainReady() const
{
	const ATerraDyneChunk* LiveChunk = Chunk.Get();
	return Manager.IsValid() && LiveChunk && LiveChunk->HeightBuffer.Num() == ChunkResolution * ChunkResolution;
}

bool ATerraDyneGardenDemoHost::IsDisplayMeshReady() const
{
	const ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk || !LiveChunk->DynamicMeshComp)
	{
		return false;
	}
	const UDynamicMesh* Dyn = LiveChunk->DynamicMeshComp->GetDynamicMesh();
	if (!Dyn)
	{
		return false;
	}
	int32 TriangleCount = 0;
	const_cast<UDynamicMesh*>(Dyn)->ProcessMesh([&](const UE::Geometry::FDynamicMesh3& Mesh)
	{
		TriangleCount = Mesh.TriangleCount();
	});
	return TriangleCount > 0;
}

bool ATerraDyneGardenDemoHost::WorldToNormalized(const FVector& WorldLocation, float& OutU, float& OutV) const
{
	const ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk)
	{
		return false;
	}
	const FVector Local = WorldLocation - LiveChunk->GetActorLocation();
	const float Half = LiveChunk->WorldSize * 0.5f;
	OutU = FMath::Clamp((Local.X + Half) / LiveChunk->WorldSize, 0.f, 1.f);
	OutV = FMath::Clamp((Local.Y + Half) / LiveChunk->WorldSize, 0.f, 1.f);
	return true;
}

bool ATerraDyneGardenDemoHost::ApplyRaise(const FVector& WorldLocation)
{
	ATerraDyneManager* LiveManager = Manager.Get();
	if (!LiveManager || !IsTerrainReady())
	{
		LastOperation = TEXT("FAIL: terrain not ready");
		return false;
	}
	const double T0 = FPlatformTime::Seconds();
	const float Before = SampleHeight(WorldLocation);
	LiveManager->ApplyGlobalBrush(WorldLocation, BrushRadiusUU, BrushStrengthUU, ETerraDyneBrushMode::Raise);
	const float After = SampleHeight(WorldLocation);
	LastBrushSeconds = FPlatformTime::Seconds() - T0;
	bForceRecolor = true;
	LastOperation = FString::Printf(TEXT("Raise API Z %.1f -> %.1f  dt=%.3fs"), Before, After, LastBrushSeconds);
	return After > Before + 0.5f;
}

bool ATerraDyneGardenDemoHost::ApplyLower(const FVector& WorldLocation)
{
	ATerraDyneManager* LiveManager = Manager.Get();
	if (!LiveManager || !IsTerrainReady())
	{
		LastOperation = TEXT("FAIL: terrain not ready");
		return false;
	}
	const double T0 = FPlatformTime::Seconds();
	const float Before = SampleHeight(WorldLocation);
	LiveManager->ApplyGlobalBrush(WorldLocation, BrushRadiusUU, BrushStrengthUU, ETerraDyneBrushMode::Lower);
	const float After = SampleHeight(WorldLocation);
	LastBrushSeconds = FPlatformTime::Seconds() - T0;
	bForceRecolor = true;
	LastOperation = FString::Printf(TEXT("Lower API Z %.1f -> %.1f  dt=%.3fs"), Before, After, LastBrushSeconds);
	return After < Before - 0.5f;
}

bool ATerraDyneGardenDemoHost::ApplyPaint(const FVector& WorldLocation, int32 LayerIndex)
{
	ATerraDyneManager* LiveManager = Manager.Get();
	if (!LiveManager || !PaintStore || !PaintStore->IsReady())
	{
		LastOperation = TEXT("FAIL: paint store");
		return false;
	}
	float U = 0.f, V = 0.f;
	if (!WorldToNormalized(WorldLocation, U, V))
	{
		return false;
	}
	const float RadiusNorm = BrushRadiusUU / ChunkWorldSize;
	const bool bHost = PaintStore->ApplyPaint(U, V, RadiusNorm, LayerIndex, PaintStrength);
	if (bHost)
	{
		SyncTerraDyneWeightBuffers();
	}

	bForceRecolor = true;
	RecolorMeshFromPaint();
	const FLinearColor Seen = PaintStore->SampleColor(U, V);
	LastOperation = FString::Printf(
		TEXT("Paint %s host=%d rgb=(%.2f,%.2f,%.2f) storeW=%u"),
		*TerraDyneDemoLayers::Names()[FMath::Clamp(LayerIndex, 0, TerraDyneDemoLayers::Count() - 1)].ToString(),
		bHost ? 1 : 0, Seen.R, Seen.G, Seen.B,
		PaintStore->SampleWeight(LayerIndex, U, V));
	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("%s at %s"), *LastOperation, *WorldLocation.ToCompactString());
	return bHost;
}

void ATerraDyneGardenDemoHost::SyncTerraDyneWeightBuffers()
{
	ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk || !PaintStore || !PaintStore->IsReady())
	{
		return;
	}

	const int32 NumPixels = ChunkResolution * ChunkResolution;
	for (int32 Layer = 0; Layer < ATerraDyneChunk::NumWeightLayers; ++Layer)
	{
		if (LiveChunk->WeightBuffers[Layer].Num() != NumPixels)
		{
			LiveChunk->WeightBuffers[Layer].SetNumZeroed(NumPixels);
		}
	}

	for (int32 Y = 0; Y < ChunkResolution; ++Y)
	{
		for (int32 X = 0; X < ChunkResolution; ++X)
		{
			const int32 Pixel = Y * ChunkResolution + X;
			const float U = (ChunkResolution > 1) ? static_cast<float>(X) / static_cast<float>(ChunkResolution - 1) : 0.f;
			const float V = (ChunkResolution > 1) ? static_cast<float>(Y) / static_cast<float>(ChunkResolution - 1) : 0.f;
			for (int32 Layer = 0; Layer < ATerraDyneChunk::NumWeightLayers; ++Layer)
			{
				LiveChunk->WeightBuffers[Layer][Pixel] =
					PaintStore->SampleWeight(Layer + 1, U, V) / 255.f;
			}
		}
	}
	LiveChunk->UploadWeightTexture();
}

float ATerraDyneGardenDemoHost::SampleHeight(const FVector& WorldLocation) const
{
	const ATerraDyneManager* LiveManager = Manager.Get();
	return LiveManager ? LiveManager->GetWorldHeightAtLocation(WorldLocation) : 0.f;
}

bool ATerraDyneGardenDemoHost::TraceCollisionZ(const FVector& WorldLocation, float& OutZ, FString& OutName) const
{
	const ATerraDyneChunk* LiveChunk = Chunk.Get();
	UWorld* World = GetWorld();
	if (!LiveChunk || !World || !LiveChunk->DynamicMeshComp)
	{
		return false;
	}
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(DemoTerrainTrace), true);
	const FVector Start(WorldLocation.X, WorldLocation.Y, 20000.f);
	const FVector End(WorldLocation.X, WorldLocation.Y, -20000.f);
	if (LiveChunk->DynamicMeshComp->LineTraceComponent(Hit, Start, End, Params) && Hit.bBlockingHit)
	{
		OutZ = Hit.ImpactPoint.Z;
		OutName = Hit.GetComponent() ? Hit.GetComponent()->GetName() : TEXT("mesh");
		return true;
	}
	if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params) && Hit.bBlockingHit)
	{
		OutZ = Hit.ImpactPoint.Z;
		OutName = Hit.GetActor() ? Hit.GetActor()->GetName() : TEXT("world");
		return true;
	}
	return false;
}

FLinearColor ATerraDyneGardenDemoHost::SamplePaintColor(const FVector& WorldLocation) const
{
	float U = 0.f, V = 0.f;
	if (!PaintStore || !WorldToNormalized(WorldLocation, U, V))
	{
		return FLinearColor::Green;
	}
	return PaintStore->SampleColor(U, V);
}

void ATerraDyneGardenDemoHost::RecolorMeshFromPaint()
{
	if (bUseNativeM4Material)
	{
		return;
	}

	ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk || !LiveChunk->DynamicMeshComp || !PaintStore)
	{
		return;
	}
	UDynamicMesh* Dyn = LiveChunk->DynamicMeshComp->GetDynamicMesh();
	if (!Dyn)
	{
		return;
	}

	bool bHasColorOverlay = false;
	int32 VertexCount = 0;
	int32 TriangleCount = 0;
	Dyn->ProcessMesh([&](const UE::Geometry::FDynamicMesh3& Mesh)
	{
		VertexCount = Mesh.VertexCount();
		TriangleCount = Mesh.TriangleCount();
		bHasColorOverlay = Mesh.HasAttributes()
			&& Mesh.Attributes()->HasPrimaryColors()
			&& Mesh.Attributes()->PrimaryColors()
			&& Mesh.Attributes()->PrimaryColors()->ElementCount() > 0;
	});
	if (VertexCount < 2)
	{
		return;
	}

	const int32 MeshId = VertexCount + TriangleCount;
	const bool bNeedsColor = bForceRecolor || MeshId != LastColoredMeshId || !bHasColorOverlay;
	if (!bNeedsColor)
	{
		return;
	}

	const float Half = LiveChunk->WorldSize * 0.5f;
	int32 ColorElements = 0;
	Dyn->EditMesh([&](UE::Geometry::FDynamicMesh3& Mesh)
	{
		Mesh.EnableAttributes();
		if (!Mesh.Attributes()->HasPrimaryColors())
		{
			Mesh.Attributes()->EnablePrimaryColors();
		}
		UE::Geometry::FDynamicMeshColorOverlay* Colors = Mesh.Attributes()->PrimaryColors();
		if (!Colors)
		{
			return;
		}
		Colors->ClearElements();

		TArray<int32> ElemOfVertex;
		ElemOfVertex.Init(INDEX_NONE, Mesh.MaxVertexID());
		for (const int32 Vid : Mesh.VertexIndicesItr())
		{
			const FVector3d P = Mesh.GetVertex(Vid);
			const float U = FMath::Clamp(static_cast<float>((P.X + Half) / LiveChunk->WorldSize), 0.f, 1.f);
			const float V = FMath::Clamp(static_cast<float>((P.Y + Half) / LiveChunk->WorldSize), 0.f, 1.f);
			const FLinearColor C = PaintStore->SampleColor(U, V);
			ElemOfVertex[Vid] = Colors->AppendElement(FVector4f(C.R, C.G, C.B, 1.f));
		}
		for (const int32 Tid : Mesh.TriangleIndicesItr())
		{
			const UE::Geometry::FIndex3i Tri = Mesh.GetTriangle(Tid);
			if (ElemOfVertex.IsValidIndex(Tri.A) && ElemOfVertex.IsValidIndex(Tri.B) && ElemOfVertex.IsValidIndex(Tri.C)
				&& ElemOfVertex[Tri.A] != INDEX_NONE && ElemOfVertex[Tri.B] != INDEX_NONE && ElemOfVertex[Tri.C] != INDEX_NONE)
			{
				Colors->SetTriangle(Tid, UE::Geometry::FIndex3i(ElemOfVertex[Tri.A], ElemOfVertex[Tri.B], ElemOfVertex[Tri.C]));
			}
		}
		ColorElements = Colors->ElementCount();
	},
	EDynamicMeshChangeType::AttributeEdit,
	EDynamicMeshAttributeChangeFlags::VertexColors);

	EnablePaintColorDisplay();
	LiveChunk->DynamicMeshComp->FastNotifyColorsUpdated();
	LastColoredMeshId = MeshId;
	bForceRecolor = false;
	UE_LOG(LogTerraDyneGardenDemo, Verbose, TEXT("Recolor overlay elements=%d verts=%d tris=%d"),
		ColorElements, VertexCount, TriangleCount);
}

namespace TerraDyneM4Compat
{
	static const TCHAR* RealIslandMic = TEXT("/Game/M4/Materials/PresetInstances/Island/Island_UE5Preset5_Inst.Island_UE5Preset5_Inst");
	static const TCHAR* RealMaster = TEXT("/Game/M4/Materials/MasterMaterials/M4UE5MasterMat.M4UE5MasterMat");
	static const TCHAR* OrphanEvalMic = TEXT("/Game/Demo/M4_Eval/Island_UE5Preset5_Inst.Island_UE5Preset5_Inst");
	static const TCHAR* ObsoleteSharedMic = TEXT("/Game/M4/Maps/Island_Demo_sharedassets/Island_UE5Preset5_Inst.Island_UE5Preset5_Inst");

	static const TCHAR* RequiredAssets[] = {
		TEXT("/Game/M4/Materials/MasterMaterials/M4UE5MasterMat.M4UE5MasterMat"),
		TEXT("/Game/M4/Materials/PresetInstances/Island/Island_UE5Preset5_Inst.Island_UE5Preset5_Inst"),
		TEXT("/Game/M4/Materials/MasterMaterials/Functions/UE58Compatibility/MF_LandscapeLayerBlend_UE58safe.MF_LandscapeLayerBlend_UE58safe"),
		TEXT("/Game/M4/Materials/MasterMaterials/Functions/SplatmapDetection.SplatmapDetection"),
		TEXT("/Game/M4/Materials/MasterMaterials/Functions/FlatlandA.FlatlandA"),
		TEXT("/Game/M4/Materials/MasterMaterials/Functions/Cliff.Cliff"),
		TEXT("/Game/M4/Materials/PaintLayers/GrassLayer.GrassLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/DirtLayer.DirtLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/SandLayer.SandLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/MudLayer.MudLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/ForestLayer.ForestLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/StonesLayer.StonesLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/JaggedRockLayer.JaggedRockLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/FlatIceLayer.FlatIceLayer"),
		TEXT("/Game/M4/Materials/PaintLayers/Custom01Layer.Custom01Layer"),
		TEXT("/Game/M4/Materials/PaintLayers/Custom02Layer.Custom02Layer"),
		TEXT("/Game/M4/Materials/MasterMaterials/Functions/Output-RVT.Output-RVT")
	};

	static UMaterialInterface* LoadMaterial(const TCHAR* Path)
	{
		return LoadObject<UMaterialInterface>(nullptr, Path);
	}
}

void ATerraDyneGardenDemoHost::AppendM4MaterialDiagnostics(FString& OutLog, UMaterialInterface* Material) const
{
	auto Line = [&OutLog](const FString& Text)
	{
		OutLog += Text;
		OutLog += TEXT("\n");
		UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("%s"), *Text);
	};

	if (!Material)
	{
		Line(TEXT("DIAG material=null"));
		return;
	}

	Line(FString::Printf(TEXT("DIAG asset=%s class=%s"), *Material->GetPathName(), *Material->GetClass()->GetName()));
	if (UMaterial* Base = Material->GetMaterial())
	{
		Line(FString::Printf(TEXT("DIAG base=%s domain=%d blend=%d twoSided=%s"),
			*Base->GetPathName(),
			static_cast<int32>(Base->MaterialDomain),
			static_cast<int32>(Base->BlendMode),
			Base->IsTwoSided() ? TEXT("yes") : TEXT("no")));
	}

	TArray<FMaterialParameterInfo> SwitchInfos;
	TArray<FGuid> SwitchIds;
	Material->GetAllParameterInfoOfType(EMaterialParameterType::StaticSwitch, SwitchInfos, SwitchIds);
	for (const FMaterialParameterInfo& Info : SwitchInfos)
	{
		const FString NameStr = Info.Name.ToString();
		if (!(NameStr.Contains(TEXT("Voxel"))
			|| NameStr.Contains(TEXT("World Position"))
			|| NameStr.Contains(TEXT("RVT"))
			|| NameStr.Contains(TEXT("Splatmap"))
			|| NameStr.Contains(TEXT("UE 5.8"))))
		{
			continue;
		}
#if WITH_EDITORONLY_DATA
		bool bValue = false;
		FGuid Unused;
		Material->GetStaticSwitchParameterValue(FHashedMaterialParameterInfo(Info.Name), bValue, Unused);
		Line(FString::Printf(TEXT("DIAG switch '%s' = %s"), *NameStr, bValue ? TEXT("true") : TEXT("false")));
#else
		Line(FString::Printf(TEXT("DIAG switch '%s' present (value editor-only)"), *NameStr));
#endif
	}

	TArray<FMaterialParameterInfo> TexInfos;
	TArray<FGuid> TexIds;
	Material->GetAllTextureParameterInfo(TexInfos, TexIds);
	bool bDeclaresHeightMap = false;
	bool bDeclaresWeightMap = false;
	for (const FMaterialParameterInfo& Info : TexInfos)
	{
		if (Info.Name == TEXT("HeightMap"))
		{
			bDeclaresHeightMap = true;
		}
		if (Info.Name == TEXT("WeightMap"))
		{
			bDeclaresWeightMap = true;
		}
	}
	UTexture* HeightMap = nullptr;
	UTexture* WeightMap = nullptr;
	Material->GetTextureParameterValue(FHashedMaterialParameterInfo(TEXT("HeightMap")), HeightMap);
	Material->GetTextureParameterValue(FHashedMaterialParameterInfo(TEXT("WeightMap")), WeightMap);
	Line(FString::Printf(
		TEXT("DIAG HeightMap declared=%s value=%s | WeightMap declared=%s value=%s (SetMaterial TerraDyne pousse ces noms ; le graphe M4 ne les consomme pas s'ils ne sont pas déclarés)"),
		bDeclaresHeightMap ? TEXT("yes") : TEXT("no"),
		HeightMap ? *HeightMap->GetName() : TEXT("null"),
		bDeclaresWeightMap ? TEXT("yes") : TEXT("no"),
		WeightMap ? *WeightMap->GetName() : TEXT("null")));

#if WITH_EDITOR
	if (UMaterial* Base = Material->GetMaterial())
	{
		int32 LandscapeExpr = 0;
		for (UMaterialExpression* Expr : Base->GetExpressions())
		{
			if (!Expr)
			{
				continue;
			}
			const FString ClassName = Expr->GetClass()->GetName();
			if (ClassName.Contains(TEXT("Landscape")))
			{
				++LandscapeExpr;
				Line(FString::Printf(TEXT("DIAG editor-expr %s"), *ClassName));
			}
		}
		Line(FString::Printf(TEXT("DIAG editor Landscape-named expressions on master (direct, not functions)=%d"), LandscapeExpr));
	}
#endif
}

FString ATerraDyneGardenDemoHost::TryApplyM4Material()
{
	ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk)
	{
		M4Verdict = TEXT("incompatibilité / non vérifié — pas de chunk");
		return M4Verdict;
	}

	bM4Tried = true;

	UMaterialInterface* OrphanEval = TerraDyneM4Compat::LoadMaterial(TerraDyneM4Compat::OrphanEvalMic);
	UMaterialInterface* ObsoleteShared = TerraDyneM4Compat::LoadMaterial(TerraDyneM4Compat::ObsoleteSharedMic);
	UMaterialInterface* Master = TerraDyneM4Compat::LoadMaterial(TerraDyneM4Compat::RealMaster);
	UMaterialInterface* IslandMic = TerraDyneM4Compat::LoadMaterial(TerraDyneM4Compat::RealIslandMic);

	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("M4 load orphanEval=%s obsoleteShared=%s master=%s islandMic=%s"),
		OrphanEval ? TEXT("ok") : TEXT("FAIL"),
		ObsoleteShared ? TEXT("ok") : TEXT("FAIL"),
		Master ? TEXT("ok") : TEXT("FAIL"),
		IslandMic ? TEXT("ok") : TEXT("FAIL"));

	UMaterialInterface* M4 = IslandMic;
	if (!M4)
	{
		M4Verdict = TEXT("incompatibilité démontrée (chargement) — MIC réel Island_UE5Preset5_Inst introuvable sous /Game/M4/Materials/PresetInstances/Island/");
		ApplyFallbackMaterial();
		bUseNativeM4Material = false;
		return M4Verdict;
	}

	if (!Master)
	{
		M4Verdict = TEXT("compatibilité partielle — MIC chargé mais M4UE5MasterMat introuvable ; ne pas conclure au graphe maître");
	}

	LiveChunk->SetMaterial(M4);
	AppliedMeshMaterial = M4;
	bUseNativeM4Material = true;
	if (LiveChunk->DynamicMeshComp)
	{
		LiveChunk->DynamicMeshComp->SetColorOverrideMode(EDynamicMeshComponentColorOverrideMode::None);
	}

	FString Diag;
	AppendM4MaterialDiagnostics(Diag, M4);

	M4Verdict = TEXT("MIC réel Island_UE5Preset5_Inst appliqué (graphe maître non modifié). Overlay vertex-color désactivé. Peinture LandscapeLayerBlend ≠ WeightMap TerraDyne. Validation visuelle humaine requise.");
	return M4Verdict;
}

FString ATerraDyneGardenDemoHost::TryApplyTerraDyneMasterMaterial()
{
	static const TCHAR* TerraDyneMasterPath =
		TEXT("/TerraDyne/Materials/VHFM/M_TerraDyne_Master.M_TerraDyne_Master");

	ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk)
	{
		PathBVerdict = TEXT("Path B FAIL — pas de chunk");
		LastOperation = PathBVerdict;
		return PathBVerdict;
	}

	UMaterialInterface* Master = LoadObject<UMaterialInterface>(nullptr, TerraDyneMasterPath);
	if (!Master)
	{
		PathBVerdict = TEXT("Path B BLOCKED — M_TerraDyne_Master introuvable sous /TerraDyne/Materials/VHFM/");
		LastOperation = PathBVerdict;
		UE_LOG(LogTerraDyneGardenDemo, Error, TEXT("%s"), *PathBVerdict);
		return PathBVerdict;
	}

	bool bDeclaresHeightMap = false;
	bool bDeclaresWeightMap = false;
	TArray<FMaterialParameterInfo> TexInfos;
	TArray<FGuid> TexIds;
	Master->GetAllTextureParameterInfo(TexInfos, TexIds);
	for (const FMaterialParameterInfo& Info : TexInfos)
	{
		if (Info.Name == TEXT("HeightMap"))
		{
			bDeclaresHeightMap = true;
		}
		if (Info.Name == TEXT("WeightMap"))
		{
			bDeclaresWeightMap = true;
		}
	}

	if (!bDeclaresHeightMap || !bDeclaresWeightMap)
	{
		PathBVerdict = FString::Printf(
			TEXT("Path B BLOCKED — M_TerraDyne_Master chargeable mais paramètres manquants (HeightMap=%s WeightMap=%s). Aucun remplacement improvisé."),
			bDeclaresHeightMap ? TEXT("yes") : TEXT("no"),
			bDeclaresWeightMap ? TEXT("yes") : TEXT("no"));
		LastOperation = PathBVerdict;
		UE_LOG(LogTerraDyneGardenDemo, Error, TEXT("%s"), *PathBVerdict);
		return PathBVerdict;
	}

	LiveChunk->SetMaterial(Master);
	AppliedMeshMaterial = Master;
	// Reuse overlay-suppress flag so RecolorMeshFromPaint does not fight the TD master.
	bUseNativeM4Material = true;
	if (LiveChunk->DynamicMeshComp)
	{
		LiveChunk->DynamicMeshComp->SetColorOverrideMode(EDynamicMeshComponentColorOverrideMode::None);
	}

	UTexture* BoundHeight = nullptr;
	UTexture* BoundWeight = nullptr;
	FString SlotMatName = TEXT("null");
	if (LiveChunk->DynamicMeshComp)
	{
		if (UMaterialInterface* Slot0 = LiveChunk->DynamicMeshComp->GetMaterial(0))
		{
			SlotMatName = Slot0->GetName();
			Slot0->GetTextureParameterValue(FHashedMaterialParameterInfo(TEXT("HeightMap")), BoundHeight);
			Slot0->GetTextureParameterValue(FHashedMaterialParameterInfo(TEXT("WeightMap")), BoundWeight);
		}
	}

	const bool bHeightBound = BoundHeight != nullptr;
	const bool bWeightBound = BoundWeight != nullptr;
	PathBVerdict = FString::Printf(
		TEXT("Path B APPLY — SetMaterial(%s) slot0=%s HeightMap=%s (%s) WeightMap=%s (%s). Overlay off. Validation visuelle humaine requise (FEAS §8)."),
		TerraDyneMasterPath,
		*SlotMatName,
		bHeightBound ? TEXT("bound") : TEXT("NULL"),
		BoundHeight ? *BoundHeight->GetName() : TEXT("-"),
		bWeightBound ? TEXT("bound") : TEXT("NULL"),
		BoundWeight ? *BoundWeight->GetName() : TEXT("-"));
	LastOperation = PathBVerdict;
	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("%s"), *PathBVerdict);

	if (!bHeightBound || !bWeightBound)
	{
		UE_LOG(LogTerraDyneGardenDemo, Warning,
			TEXT("Path B: SetMaterial ran but HeightMap/WeightMap readback incomplete (SetupGPU/HeightRT may be pending)."));
	}

	return PathBVerdict;
}

FString ATerraDyneGardenDemoHost::RestoreDemoOverlayMaterial()
{
	ApplyFallbackMaterial();
	bForceRecolor = true;
	PathBVerdict = TEXT("Path B restored — vertex-color overlay demo material via ApplyFallbackMaterial");
	LastOperation = PathBVerdict;
	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("%s"), *PathBVerdict);
	return PathBVerdict;
}

void ATerraDyneGardenDemoHost::RunM4CompatSelfTest()
{
	TArray<FString> Lines;
	auto Add = [&Lines](const FString& Text)
	{
		Lines.Add(Text);
		UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("%s"), *Text);
	};

	Add(TEXT("M4CompatSelfTest begin"));
	Add(TEXT("plugin=TerraDyne OSS 0.4 host-only adapter; master graph not edited; plugin source not edited"));

	int32 Loaded = 0;
	int32 Missing = 0;
	for (const TCHAR* Path : TerraDyneM4Compat::RequiredAssets)
	{
		UObject* Obj = StaticLoadObject(UObject::StaticClass(), nullptr, Path);
		if (Obj)
		{
			++Loaded;
			Add(FString::Printf(TEXT("ASSET ok %s class=%s"), Path, *Obj->GetClass()->GetName()));
		}
		else
		{
			++Missing;
			Add(FString::Printf(TEXT("ASSET FAIL %s"), Path));
		}
	}
	Add(FString::Printf(TEXT("ASSET summary loaded=%d missing=%d"), Loaded, Missing));

	const FString ApplyResult = TryApplyM4Material();
	Add(FString::Printf(TEXT("APPLY %s"), *ApplyResult));

	ATerraDyneChunk* LiveChunk = Chunk.Get();
	UMaterialInterface* Slot0 = (LiveChunk && LiveChunk->DynamicMeshComp) ? LiveChunk->DynamicMeshComp->GetMaterial(0) : nullptr;
	Add(FString::Printf(TEXT("SLOT0 %s"), Slot0 ? *Slot0->GetPathName() : TEXT("null")));

	FString Diag;
	AppendM4MaterialDiagnostics(Diag, Slot0);
	TArray<FString> DiagLines;
	Diag.ParseIntoArray(DiagLines, TEXT("\n"), true);
	Lines.Append(DiagLines);

	const FVector Probe = FVector(200.f, 200.f, 0.f);
	const float Before = SampleHeight(Probe);
	const bool bRaised = ApplyRaise(Probe);
	const float AfterRaise = SampleHeight(Probe);
	const bool bLowered = ApplyLower(Probe);
	const float AfterLower = SampleHeight(Probe);
	Add(FString::Printf(
		TEXT("HEIGHT before=%.3f raise=%s afterRaise=%.3f lower=%s afterLower=%.3f"),
		Before, bRaised ? TEXT("ok") : TEXT("FAIL"), AfterRaise,
		bLowered ? TEXT("ok") : TEXT("FAIL"), AfterLower));

	const bool bHeightOk = bRaised && AfterRaise > Before + 1.f && bLowered && AfterLower < AfterRaise - 1.f;
	Add(FString::Printf(TEXT("HEIGHT_RELIEF %s (data plane TerraDyne, independent of M4 graph)"),
		bHeightOk ? TEXT("PASS") : TEXT("FAIL")));

	Add(TEXT("VISUAL_M4 non vérifié par ce self-test (éditeur / humain)"));
	Add(TEXT("PAINT_M4_LAYERS non vérifié — LandscapeLayerBlend n'est pas alimenté par WeightMap OSS 0.4"));
	Add(TEXT("RVT non vérifié — producteur Landscape absent sur DynamicMesh"));
	Add(TEXT("COLLISION non vérifié ici (debounce)"));
	Add(TEXT("SAVE non vérifié — SaveWorld host non câblé"));
	Add(TEXT("PACKAGING non vérifié par ce run"));

	const FString ReportPath = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectDir(), TEXT("Results"), TEXT("m4_compat_selftest.log")));
	Add(FString::Printf(TEXT("WROTE %s"), *ReportPath));
	Add(TEXT("M4CompatSelfTest end"));
	FFileHelper::SaveStringArrayToFile(Lines, *ReportPath);
	FPlatformMisc::RequestExit(false);
}

FString ATerraDyneGardenDemoHost::GetStatusLine() const
{
	const UTerraDyneSettings* Settings = GetDefault<UTerraDyneSettings>();
	return FString::Printf(
		TEXT("%s | GPU=%s debounce=%.2fs | M4: %s | PathB: %s"),
		*LastOperation,
		(Settings && Settings->bEnableGPUBrushes) ? TEXT("on") : TEXT("off"),
		Settings ? Settings->CollisionDebounceTime : 0.2f,
		*M4Verdict,
		*PathBVerdict);
}

void ATerraDyneGardenDemoHost::RunLowerSelfTest()
{
	ATerraDyneChunk* LiveChunk = Chunk.Get();
	if (!LiveChunk || !IsTerrainReady())
	{
		UE_LOG(LogTerraDyneGardenDemo, Error, TEXT("LowerSelfTest FAIL: terrain not ready"));
		return;
	}

	auto SampleBuffers = [LiveChunk](const FVector& WorldLocation, float& OutBase, float& OutSculpt, float& OutHeight)
	{
		OutBase = OutSculpt = OutHeight = 0.f;
		if (LiveChunk->Resolution < 2 || LiveChunk->HeightBuffer.Num() != LiveChunk->Resolution * LiveChunk->Resolution)
		{
			return;
		}
		const FVector Local = WorldLocation - LiveChunk->GetActorLocation();
		const float Half = LiveChunk->WorldSize * 0.5f;
		const float Step = LiveChunk->WorldSize / static_cast<float>(LiveChunk->Resolution - 1);
		const int32 X = FMath::Clamp(FMath::RoundToInt((Local.X + Half) / Step), 0, LiveChunk->Resolution - 1);
		const int32 Y = FMath::Clamp(FMath::RoundToInt((Local.Y + Half) / Step), 0, LiveChunk->Resolution - 1);
		const int32 Index = Y * LiveChunk->Resolution + X;
		if (LiveChunk->BaseBuffer.IsValidIndex(Index))
		{
			OutBase = LiveChunk->BaseBuffer[Index];
		}
		if (LiveChunk->SculptBuffer.IsValidIndex(Index))
		{
			OutSculpt = LiveChunk->SculptBuffer[Index];
		}
		if (LiveChunk->HeightBuffer.IsValidIndex(Index))
		{
			OutHeight = LiveChunk->HeightBuffer[Index];
		}
	};

	auto Line = [](FString& Out, const TCHAR* Name, bool bPass, const FString& Detail)
	{
		Out += FString::Printf(TEXT("%s %s  %s\n"), bPass ? TEXT("PASS") : TEXT("FAIL"), Name, *Detail);
	};

	FString Report;
	const FVector Origin = LiveChunk->GetActorLocation();
	float Base = 0.f, Sculpt = 0.f, Height = 0.f;
	SampleBuffers(Origin, Base, Sculpt, Height);
	const float Z0 = SampleHeight(Origin);
	Report += FString::Printf(
		TEXT("T0 center Base=%.6f Sculpt=%.6f Height=%.6f WorldZ=%.3f ZScale=%.1f\n"),
		Base, Sculpt, Height, Z0, LiveChunk->ZScale);

	// A: Lower x3 on intact terrain, offset so it does not share B/C.
	const FVector PointA = Origin + FVector(-2500.f, 0.f, 0.f);
	float A0 = SampleHeight(PointA);
	ApplyLower(PointA);
	ApplyLower(PointA);
	ApplyLower(PointA);
	float AH = 0.f, AS = 0.f, AB = 0.f;
	SampleBuffers(PointA, AB, AS, AH);
	const float AZ = SampleHeight(PointA);
	Line(Report, TEXT("A Lower x3 intact"),
		AH < -0.01f && AZ < A0 - 1.f,
		FString::Printf(TEXT("WorldZ %.3f -> %.3f  Height=%.6f Sculpt=%.6f"), A0, AZ, AH, AS));

	// B: Raise then Lower past the initial surface.
	const FVector PointB = Origin + FVector(0.f, 2500.f, 0.f);
	const float B0 = SampleHeight(PointB);
	ApplyRaise(PointB);
	const float BRaised = SampleHeight(PointB);
	ApplyLower(PointB);
	ApplyLower(PointB);
	ApplyLower(PointB);
	float BH = 0.f, BS = 0.f, BB = 0.f;
	SampleBuffers(PointB, BB, BS, BH);
	const float BZ = SampleHeight(PointB);
	Line(Report, TEXT("B Raise then Lower past T0"),
		BRaised > B0 + 1.f && BH < -0.01f && BZ < B0 - 1.f,
		FString::Printf(TEXT("T0=%.3f raised=%.3f after3Lower=%.3f Height=%.6f"), B0, BRaised, BZ, BH));

	// C: Lower then Raise back above T0.
	const FVector PointC = Origin + FVector(2500.f, 0.f, 0.f);
	const float C0 = SampleHeight(PointC);
	ApplyLower(PointC);
	const float CLow = SampleHeight(PointC);
	ApplyRaise(PointC);
	ApplyRaise(PointC);
	float CH = 0.f, CS = 0.f, CB = 0.f;
	SampleBuffers(PointC, CB, CS, CH);
	const float CZ = SampleHeight(PointC);
	Line(Report, TEXT("C Lower then Raise above T0"),
		CLow < C0 - 1.f && CZ > C0 + 1.f,
		FString::Printf(TEXT("T0=%.3f lowered=%.3f raised=%.3f Height=%.6f"), C0, CLow, CZ, CH));

	// D: repeated Lower is monotonic and stays below T0.
	const FVector PointD = Origin + FVector(0.f, -2500.f, 0.f);
	const float D0 = SampleHeight(PointD);
	ApplyLower(PointD);
	const float D1 = SampleHeight(PointD);
	ApplyLower(PointD);
	const float D2 = SampleHeight(PointD);
	ApplyLower(PointD);
	const float D3 = SampleHeight(PointD);
	Line(Report, TEXT("D repeated Lower deepens"),
		D1 < D0 - 1.f && D2 < D1 - 1.f && D3 < D2 - 1.f,
		FString::Printf(TEXT("%.3f -> %.3f -> %.3f -> %.3f"), D0, D1, D2, D3));

	// E: another intact location.
	const FVector PointE = Origin + FVector(1800.f, 1800.f, 0.f);
	const float E0 = SampleHeight(PointE);
	ApplyLower(PointE);
	float EH = 0.f, ES = 0.f, EB = 0.f;
	SampleBuffers(PointE, EB, ES, EH);
	const float EZ = SampleHeight(PointE);
	Line(Report, TEXT("E Lower other intact point"),
		EH < -0.01f && EZ < E0 - 1.f,
		FString::Printf(TEXT("WorldZ %.3f -> %.3f Height=%.6f"), E0, EZ, EH));

	const bool bAll = Report.Contains(TEXT("FAIL")) == false;
	LastOperation = bAll ? TEXT("LowerSelfTest PASS") : TEXT("LowerSelfTest FAIL");
	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("LowerSelfTest\n%s"), *Report);

	const FString SavedPath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("lower_selftest.txt"));
	FFileHelper::SaveStringToFile(Report, *SavedPath);
	const FString ResultsPath = FPaths::Combine(FPaths::ProjectDir(), TEXT("Results"), TEXT("lower_selftest.txt"));
	FFileHelper::SaveStringToFile(Report, *ResultsPath);
}

void ATerraDyneGardenDemoHost::RunPaintSelfTest()
{
	if (!IsTerrainReady() || !PaintStore)
	{
		UE_LOG(LogTerraDyneGardenDemo, Error, TEXT("PaintSelfTest FAIL: terrain not ready"));
		return;
	}

	int32 LayersOk = 0;
	for (int32 Layer = 0; Layer < TerraDyneDemoLayers::Count(); ++Layer)
	{
		const FVector Point((Layer - 5) * 1800.f, 400.f, 0.f);
		const bool bPainted = ApplyPaint(Point, Layer);
		float U = 0.f, V = 0.f;
		WorldToNormalized(Point, U, V);
		const uint8 StoreW = PaintStore->SampleWeight(Layer, U, V);
		const FLinearColor StoreC = PaintStore->SampleColor(U, V);
		const FLinearColor Expect = TerraDyneDemoLayers::Color(Layer);

		FVector4f OverlayC = FVector4f::Zero();
		bool bOverlay = false;
		if (ATerraDyneChunk* LiveChunk = Chunk.Get())
		{
			if (UDynamicMesh* Dyn = LiveChunk->DynamicMeshComp ? LiveChunk->DynamicMeshComp->GetDynamicMesh() : nullptr)
			{
				Dyn->ProcessMesh([&](const UE::Geometry::FDynamicMesh3& Mesh)
				{
					if (!Mesh.HasAttributes() || !Mesh.Attributes()->HasPrimaryColors())
					{
						return;
					}
					const UE::Geometry::FDynamicMeshColorOverlay* Colors = Mesh.Attributes()->PrimaryColors();
					if (!Colors || Colors->ElementCount() <= 0)
					{
						return;
					}
					const FVector Local = Point - LiveChunk->GetActorLocation();
					int32 BestVid = INDEX_NONE;
					double BestD = TNumericLimits<double>::Max();
					for (const int32 Vid : Mesh.VertexIndicesItr())
					{
						const FVector3d P = Mesh.GetVertex(Vid);
						const double D = FMath::Square(P.X - Local.X) + FMath::Square(P.Y - Local.Y);
						if (D < BestD)
						{
							BestD = D;
							BestVid = Vid;
						}
					}
					if (BestVid != INDEX_NONE)
					{
						for (const int32 Tid : Mesh.TriangleIndicesItr())
						{
							const UE::Geometry::FIndex3i Tri = Mesh.GetTriangle(Tid);
							const UE::Geometry::FIndex3i Cols = Colors->GetTriangle(Tid);
							if (Tri.A == BestVid) { OverlayC = Colors->GetElement(Cols.A); bOverlay = true; break; }
							if (Tri.B == BestVid) { OverlayC = Colors->GetElement(Cols.B); bOverlay = true; break; }
							if (Tri.C == BestVid) { OverlayC = Colors->GetElement(Cols.C); bOverlay = true; break; }
						}
					}
				});
			}
		}

		const bool bStore = bPainted && StoreW >= 200;
		const bool bColorNear = FMath::Abs(StoreC.R - Expect.R) < 0.2f
			&& FMath::Abs(StoreC.G - Expect.G) < 0.2f
			&& FMath::Abs(StoreC.B - Expect.B) < 0.2f;
		const bool bOverlayNear = bOverlay
			&& FMath::Abs(OverlayC.X - Expect.R) < 0.25f
			&& FMath::Abs(OverlayC.Y - Expect.G) < 0.25f
			&& FMath::Abs(OverlayC.Z - Expect.B) < 0.25f;
		const bool bOk = bStore && bColorNear && bOverlayNear;
		LayersOk += bOk ? 1 : 0;
		UE_LOG(LogTerraDyneGardenDemo, Log,
			TEXT("PaintSelfTest layer=%d %s storeW=%u storeRGB=(%.2f,%.2f,%.2f) overlay=%d overlayRGB=(%.2f,%.2f,%.2f) %s"),
			Layer, *TerraDyneDemoLayers::Names()[Layer].ToString(), StoreW,
			StoreC.R, StoreC.G, StoreC.B, bOverlay ? 1 : 0, OverlayC.X, OverlayC.Y, OverlayC.Z,
			bOk ? TEXT("PASS") : TEXT("FAIL"));
	}

	const FVector RaisePoint(-7200.f, 400.f, 0.f);
	ApplyRaise(RaisePoint);
	ApplyLower(RaisePoint);
	bForceRecolor = true;
	RecolorMeshFromPaint();
	UE_LOG(LogTerraDyneGardenDemo, Log, TEXT("PaintSelfTest layers_pass=%d/%d after Raise/Lower"),
		LayersOk, TerraDyneDemoLayers::Count());
}

static FAutoConsoleCommand GDemoPaintSelfTestCmd(
	TEXT("Demo.PaintSelfTest"),
	TEXT("Paint all 11 demo layers at spaced points and log store vs overlay colors."),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		if (!GEngine)
		{
			return;
		}
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (UWorld* World = Context.World())
			{
				for (TActorIterator<ATerraDyneGardenDemoHost> It(World); It; ++It)
				{
					It->RunPaintSelfTest();
					return;
				}
			}
		}
	}));

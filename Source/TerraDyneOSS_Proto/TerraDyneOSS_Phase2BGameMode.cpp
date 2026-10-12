#include "TerraDyneOSS_Phase2BGameMode.h"

#include "CollisionQueryParams.h"
#include "Core/TerraDyneManager.h"
#include "Core/TerraDyneSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Settings/TerraDyneSettings.h"
#include "World/TerraDyneChunk.h"
#include "World/TerraDyneTileData.h"

DEFINE_LOG_CATEGORY_STATIC(LogTerraDyneOSS_Proto, Log, All);

namespace
{
	constexpr float WaitAbortSeconds = 8.0f;
	constexpr float TraceStartZ = 10000.0f;
	constexpr float TraceEndZ = -10000.0f;
	constexpr float StrengthUU = 80.0f;

	struct FConfigDef
	{
		const TCHAR* Name;
		float WorldSize;
		int32 Resolution;
	};

	struct FCaseDef
	{
		const TCHAR* Id;
		float Radius;
		bool bOnVertex;
	};

	const FConfigDef Configs[3] = {
		{ TEXT("A"), 1000.0f, 41 },
		{ TEXT("B"), 1000.0f, 40 },
		{ TEXT("C"), 13545.0f, 127 },
	};

	const FCaseDef Cases[4] = {
		{ TEXT("25cm_vertex"), 12.5f, true },
		{ TEXT("25cm_quad"), 12.5f, false },
		{ TEXT("2m_vertex"), 100.0f, true },
		{ TEXT("2m_quad"), 100.0f, false },
	};
}

ATerraDyneOSS_Phase2BGameMode::ATerraDyneOSS_Phase2BGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ATerraDyneOSS_Phase2BGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!SpawnManager())
	{
		return;
	}

	SequenceStartSeconds = FPlatformTime::Seconds();
	StageStartSeconds = SequenceStartSeconds;
	ConfigIndex = 0;
	CaseIndex = 0;
	Stage = EPhase2Stage::PrepareConfig;
	SetActorTickEnabled(true);

	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("PHASE2B START 12 cases A/B/C Strength=%.3f GPU path off streaming paused"), StrengthUU);
}

bool ATerraDyneOSS_Phase2BGameMode::SpawnManager()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTerraDyneOSS_Proto, Error, TEXT("STEP FAIL world: GetWorld() is null"));
		return false;
	}

	FActorSpawnParameters ManagerParams;
	ManagerParams.bDeferConstruction = true;
	ManagerParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATerraDyneManager* Manager = World->SpawnActor<ATerraDyneManager>(
		ATerraDyneManager::StaticClass(),
		FTransform::Identity,
		ManagerParams);
	if (!Manager)
	{
		UE_LOG(LogTerraDyneOSS_Proto, Error, TEXT("STEP FAIL manager: SpawnActor returned null"));
		return false;
	}

	Manager->GlobalChunkSize = Configs[0].WorldSize;
	Manager->bSpawnDefaultChunksOnBeginPlay = false;
	Manager->bStreamingPaused = true;
	Manager->ProceduralWorldSettings.bEnableSeededOutskirts = false;
	Manager->FinishSpawning(FTransform::Identity);

	UTerraDyneSubsystem* Subsystem = World->GetSubsystem<UTerraDyneSubsystem>();
	ATerraDyneManager* Registered = Subsystem ? Subsystem->GetTerrainManager() : nullptr;
	if (Registered != Manager)
	{
		UE_LOG(LogTerraDyneOSS_Proto, Error,
			TEXT("STEP FAIL manager: subsystem registry mismatch (registered=%s)"),
			Registered ? *Registered->GetName() : TEXT("null"));
		return false;
	}

	const UTerraDyneSettings* Settings = GetDefault<UTerraDyneSettings>();
	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("STEP OK manager: %s streamingPaused=%s bEnableGPUBrushes=%s CanEverRender=%s CollisionDebounceTime=%.3f"),
		*Manager->GetName(),
		Manager->bStreamingPaused ? TEXT("true") : TEXT("false"),
		(Settings && Settings->bEnableGPUBrushes) ? TEXT("true") : TEXT("false"),
		FApp::CanEverRender() ? TEXT("true") : TEXT("false"),
		Settings ? Settings->CollisionDebounceTime : -1.0f);

	TerrainManager = Manager;
	return true;
}

void ATerraDyneOSS_Phase2BGameMode::DestroyCurrentChunk()
{
	ATerraDyneManager* Manager = TerrainManager.Get();
	ATerraDyneChunk* Chunk = TerrainChunk.Get();
	if (Manager && Chunk)
	{
		Manager->UnregisterChunk(Chunk);
	}
	if (Chunk)
	{
		Chunk->Destroy();
	}
	TerrainChunk.Reset();
	BaselineBuffer.Reset();
}

bool ATerraDyneOSS_Phase2BGameMode::SpawnChunkForCurrentConfig()
{
	ATerraDyneManager* Manager = TerrainManager.Get();
	UWorld* World = GetWorld();
	if (!Manager || !World)
	{
		UE_LOG(LogTerraDyneOSS_Proto, Error, TEXT("STEP FAIL config: manager/world invalid"));
		return false;
	}

	DestroyCurrentChunk();

	const FConfigDef& Cfg = Configs[ConfigIndex];
	const FIntPoint Coord(0, 0);
	const int32 ExpectedSamples = Cfg.Resolution * Cfg.Resolution;

	Manager->GlobalChunkSize = Cfg.WorldSize;

	FActorSpawnParameters ChunkParams;
	ChunkParams.bDeferConstruction = true;
	ChunkParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector ChunkLocation = Manager->GetActorLocation()
		+ FVector(Coord.X * Cfg.WorldSize, Coord.Y * Cfg.WorldSize, 0.0f);

	ATerraDyneChunk* Chunk = World->SpawnActor<ATerraDyneChunk>(
		ATerraDyneChunk::StaticClass(),
		FTransform(ChunkLocation),
		ChunkParams);
	if (!Chunk)
	{
		UE_LOG(LogTerraDyneOSS_Proto, Error, TEXT("STEP FAIL chunk: SpawnActor returned null for config %s"), Cfg.Name);
		return false;
	}

	Chunk->GridCoordinate = Coord;
	Chunk->WorldSize = Cfg.WorldSize;
	Chunk->ChunkSizeWorldUnits = Cfg.WorldSize;
	Chunk->Resolution = Cfg.Resolution;
	Chunk->FinishSpawning(FTransform(ChunkLocation));
	Chunk->InitializeChunk(Coord, Cfg.WorldSize, Cfg.Resolution, nullptr, nullptr);

	if (Manager->GetChunkAtCoord(Coord) != Chunk)
	{
		Manager->RegisterChunk(Chunk);
	}

	const bool bRegistered = Manager->GetChunkAtCoord(Coord) == Chunk;
	const int32 HeightSamples = Chunk->HeightBuffer.Num();
	const bool bGpu = Chunk->IsUsingGPU();
	const bool bMeshPresent = Chunk->DynamicMeshComp.Get() != nullptr;
	CurrentStep = (Chunk->Resolution > 1) ? (Chunk->WorldSize / static_cast<float>(Chunk->Resolution - 1)) : 0.0f;

	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("PHASE2B CONFIG %s requestedRes=%d effectiveRes=%d WorldSize=%.6f Step=%.9f HeightBuffer=%d expected=%d GPU=%s MeshComp=%s ZScale=%.3f registered=%s ActiveChunks=%d"),
		Cfg.Name,
		Cfg.Resolution,
		Chunk->Resolution,
		Chunk->WorldSize,
		CurrentStep,
		HeightSamples,
		ExpectedSamples,
		bGpu ? TEXT("true") : TEXT("false"),
		bMeshPresent ? TEXT("true") : TEXT("false"),
		Chunk->ZScale,
		bRegistered ? TEXT("true") : TEXT("false"),
		Manager->GetActiveChunkCount());

	if (!bRegistered || HeightSamples != ExpectedSamples || Chunk->Resolution != Cfg.Resolution
		|| !FMath::IsNearlyEqual(Chunk->WorldSize, Cfg.WorldSize) || bGpu || !bMeshPresent)
	{
		UE_LOG(LogTerraDyneOSS_Proto, Error,
			TEXT("STEP FAIL config %s: effective values rejected (res %d vs %d size %.6f vs %.6f samples %d gpu=%s mesh=%s registered=%s)"),
			Cfg.Name, Chunk->Resolution, Cfg.Resolution, Chunk->WorldSize, Cfg.WorldSize, HeightSamples,
			bGpu ? TEXT("true") : TEXT("false"),
			bMeshPresent ? TEXT("true") : TEXT("false"),
			bRegistered ? TEXT("true") : TEXT("false"));
		Chunk->Destroy();
		TerrainChunk.Reset();
		return false;
	}

	TerrainChunk = Chunk;
	return true;
}

FVector ATerraDyneOSS_Phase2BGameMode::VertexWorld(int32 X, int32 Y) const
{
	ATerraDyneChunk* Chunk = TerrainChunk.Get();
	if (!Chunk || Chunk->Resolution < 2)
	{
		return FVector::ZeroVector;
	}
	const float HalfSize = Chunk->WorldSize * 0.5f;
	const float Step = Chunk->WorldSize / static_cast<float>(Chunk->Resolution - 1);
	const FVector Local(-HalfSize + X * Step, -HalfSize + Y * Step, 0.0f);
	return Chunk->GetActorLocation() + Local;
}

void ATerraDyneOSS_Phase2BGameMode::ComputeCasePoints()
{
	ATerraDyneChunk* Chunk = TerrainChunk.Get();
	const FCaseDef& Case = Cases[CaseIndex];
	BrushRadius = Case.Radius;
	BrushStrength = StrengthUU;

	const int32 Res = Chunk ? Chunk->Resolution : 0;
	const int32 Mid = (Res > 1) ? ((Res - 1) / 2) : 0;
	const int32 VertexX = (Res % 2 == 0) ? (Mid + 1) : Mid;
	const int32 VertexY = VertexX;

	NearestVertexWorld = VertexWorld(VertexX, VertexY);
	NearestVertexIndex = VertexY * Res + VertexX;

	if (Case.bOnVertex)
	{
		BrushWorld = NearestVertexWorld;
	}
	else
	{
		const float HalfStep = CurrentStep * 0.5f;
		if (Res % 2 == 0)
		{
			BrushWorld = Chunk ? Chunk->GetActorLocation() : FVector::ZeroVector;
		}
		else
		{
			BrushWorld = NearestVertexWorld + FVector(HalfStep, HalfStep, 0.0f);
		}
	}

	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("PHASE2B CASE %s_%s onVertex=%s Brush=%s Radius=%.6f Strength=%.6f NearestV=%s Idx=%d Step=%.9f"),
		Configs[ConfigIndex].Name,
		Case.Id,
		Case.bOnVertex ? TEXT("true") : TEXT("false"),
		*BrushWorld.ToString(),
		BrushRadius,
		BrushStrength,
		*NearestVertexWorld.ToString(),
		NearestVertexIndex,
		CurrentStep);
}

void ATerraDyneOSS_Phase2BGameMode::CaptureSurface(FSurfaceSample& OutSample, const FVector& TraceWorld) const
{
	OutSample = FSurfaceSample();
	OutSample.Seconds = FPlatformTime::Seconds() - SequenceStartSeconds;

	ATerraDyneChunk* Chunk = TerrainChunk.Get();
	ATerraDyneManager* Manager = TerrainManager.Get();
	UWorld* World = GetWorld();
	if (!Chunk || !Manager || !World)
	{
		return;
	}

	const FVector LocalBrush = TraceWorld - Chunk->GetActorLocation();
	const FVector LocalNearest = NearestVertexWorld - Chunk->GetActorLocation();
	OutSample.ApiLocalBrush = Chunk->GetHeightAtLocation(LocalBrush);
	OutSample.ApiWorldBrush = Manager->GetWorldHeightAtLocation(TraceWorld);
	OutSample.ApiLocalNearest = Chunk->GetHeightAtLocation(LocalNearest);

	UDynamicMeshComponent* MeshComp = Chunk->DynamicMeshComp.Get();
	OutSample.bMeshComp = MeshComp != nullptr;
	if (MeshComp)
	{
		const FBoxSphereBounds LocalBounds = MeshComp->GetLocalBounds();
		OutSample.MeshBoundsMinZ = LocalBounds.Origin.Z - LocalBounds.BoxExtent.Z;
		OutSample.MeshBoundsMaxZ = LocalBounds.Origin.Z + LocalBounds.BoxExtent.Z;
		OutSample.MeshBoundsExtentX = LocalBounds.BoxExtent.X;
		OutSample.bCollisionEnabled = MeshComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision;

		const FVector CompStart(TraceWorld.X, TraceWorld.Y, TraceWorld.Z + TraceStartZ);
		const FVector CompEnd(TraceWorld.X, TraceWorld.Y, TraceWorld.Z + TraceEndZ);
		FHitResult CompHit;
		FCollisionQueryParams CompParams(FName(TEXT("Phase2BComponentTrace")), true);
		OutSample.bComponentTraceHit = MeshComp->LineTraceComponent(CompHit, CompStart, CompEnd, CompParams);
		if (OutSample.bComponentTraceHit)
		{
			OutSample.ComponentTraceZ = CompHit.ImpactPoint.Z;
			OutSample.ComponentHitName = CompHit.GetComponent() ? CompHit.GetComponent()->GetName() : TEXT("none");
		}
	}

	const FVector WorldStart(TraceWorld.X, TraceWorld.Y, TraceWorld.Z + TraceStartZ);
	const FVector WorldEnd(TraceWorld.X, TraceWorld.Y, TraceWorld.Z + TraceEndZ);
	FHitResult WorldHit;
	FCollisionQueryParams WorldParams(FName(TEXT("Phase2BWorldTrace")), true);
	WorldParams.AddIgnoredActor(this);
	OutSample.bWorldTraceHit = World->LineTraceSingleByChannel(WorldHit, WorldStart, WorldEnd, ECC_Visibility, WorldParams);
	if (OutSample.bWorldTraceHit)
	{
		OutSample.WorldTraceZ = WorldHit.ImpactPoint.Z;
		OutSample.WorldHitName = WorldHit.GetActor() ? WorldHit.GetActor()->GetName() : TEXT("none");
	}
}

void ATerraDyneOSS_Phase2BGameMode::LogSurface(const TCHAR* Label, const FSurfaceSample& Sample) const
{
	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("PHASE2B %s t=%.4fs ApiBrushL=%.6f ApiBrushW=%.6f ApiNearL=%.6f MeshZ=[%.6f,%.6f] ExtentX=%.3f CompHit=%s CompZ=%.6f WorldHit=%s WorldZ=%.6f WorldActor=%s"),
		Label,
		Sample.Seconds,
		Sample.ApiLocalBrush,
		Sample.ApiWorldBrush,
		Sample.ApiLocalNearest,
		Sample.MeshBoundsMinZ,
		Sample.MeshBoundsMaxZ,
		Sample.MeshBoundsExtentX,
		Sample.bComponentTraceHit ? TEXT("true") : TEXT("false"),
		Sample.ComponentTraceZ,
		Sample.bWorldTraceHit ? TEXT("true") : TEXT("false"),
		Sample.WorldTraceZ,
		*Sample.WorldHitName);
}

ATerraDyneOSS_Phase2BGameMode::FBufferStats ATerraDyneOSS_Phase2BGameMode::ComputeBufferStats(const TArray<float>& Reference) const
{
	FBufferStats Stats;
	ATerraDyneChunk* Chunk = TerrainChunk.Get();
	if (!Chunk)
	{
		return Stats;
	}

	const int32 Num = FMath::Min(Chunk->HeightBuffer.Num(), Reference.Num());
	for (int32 i = 0; i < Num; ++i)
	{
		const float Delta = Chunk->HeightBuffer[i] - Reference[i];
		const float AbsVal = FMath::Abs(Chunk->HeightBuffer[i]);
		if (AbsVal > Stats.MaxAbsValue)
		{
			Stats.MaxAbsValue = AbsVal;
			Stats.MaxAbsValueIndex = i;
		}
		if (FMath::Abs(Delta) > KINDA_SMALL_NUMBER)
		{
			if (Stats.ChangedCount == 0)
			{
				Stats.MinDelta = Delta;
				Stats.MaxDelta = Delta;
				Stats.FirstChangedIndex = i;
			}
			Stats.ChangedCount++;
			Stats.LastChangedIndex = i;
			Stats.MinDelta = FMath::Min(Stats.MinDelta, Delta);
			Stats.MaxDelta = FMath::Max(Stats.MaxDelta, Delta);
			Stats.SumDelta += Delta;
			if (FMath::Abs(Delta) > Stats.AbsMaxDelta)
			{
				Stats.AbsMaxDelta = FMath::Abs(Delta);
				Stats.MaxAbsIndex = i;
			}
		}
	}

	if (Chunk->HeightBuffer.Num() > Reference.Num())
	{
		for (int32 i = Reference.Num(); i < Chunk->HeightBuffer.Num(); ++i)
		{
			if (FMath::Abs(Chunk->HeightBuffer[i]) > KINDA_SMALL_NUMBER)
			{
				Stats.ChangedCount++;
			}
		}
	}
	return Stats;
}

void ATerraDyneOSS_Phase2BGameMode::LogBufferStats(const TCHAR* Label, const FBufferStats& Stats) const
{
	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("PHASE2B %s changed=%d firstIdx=%d lastIdx=%d maxAbsIdx=%d minD=%.8f maxD=%.8f absMaxD=%.8f sumD=%.8f maxAbsValue=%.8f maxAbsValueIdx=%d"),
		Label,
		Stats.ChangedCount,
		Stats.FirstChangedIndex,
		Stats.LastChangedIndex,
		Stats.MaxAbsIndex,
		Stats.MinDelta,
		Stats.MaxDelta,
		Stats.AbsMaxDelta,
		Stats.SumDelta,
		Stats.MaxAbsValue,
		Stats.MaxAbsValueIndex);

	ATerraDyneChunk* Chunk = TerrainChunk.Get();
	if (!Chunk || Stats.ChangedCount <= 0)
	{
		return;
	}

	const int32 MaxList = 16;
	int32 Listed = 0;
	FString IndexList;
	for (int32 i = 0; i < Chunk->HeightBuffer.Num() && Listed < MaxList; ++i)
	{
		const float Ref = BaselineBuffer.IsValidIndex(i) ? BaselineBuffer[i] : 0.0f;
		if (FMath::Abs(Chunk->HeightBuffer[i] - Ref) > KINDA_SMALL_NUMBER)
		{
			IndexList += FString::Printf(TEXT("%d:%.8f "), i, Chunk->HeightBuffer[i] - Ref);
			Listed++;
		}
	}
	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("PHASE2B %s indices(first%d)=%s"), Label, Listed, *IndexList);
}

bool ATerraDyneOSS_Phase2BGameMode::HasMeshBoundsDelta(const FSurfaceSample& Before, const FSurfaceSample& After) const
{
	return !FMath::IsNearlyEqual(Before.MeshBoundsMaxZ, After.MeshBoundsMaxZ, KINDA_SMALL_NUMBER)
		|| !FMath::IsNearlyEqual(Before.MeshBoundsMinZ, After.MeshBoundsMinZ, KINDA_SMALL_NUMBER);
}

bool ATerraDyneOSS_Phase2BGameMode::HasComponentTraceDelta(const FSurfaceSample& Before, const FSurfaceSample& After) const
{
	if (!Before.bComponentTraceHit || !After.bComponentTraceHit)
	{
		return false;
	}
	return !FMath::IsNearlyEqual(Before.ComponentTraceZ, After.ComponentTraceZ, KINDA_SMALL_NUMBER);
}

void ATerraDyneOSS_Phase2BGameMode::AbortCurrentConfig(const TCHAR* Reason)
{
	const int32 Remaining = CasesPerConfig - CaseIndex;
	CasesAborted += Remaining;
	UE_LOG(LogTerraDyneOSS_Proto, Error,
		TEXT("PHASE2B ABORT config=%s at case=%d remainingInConfig=%d reason=%s"),
		Configs[ConfigIndex].Name, CaseIndex, Remaining, Reason);
	CaseIndex = CasesPerConfig;
	Stage = EPhase2Stage::Advance;
	StageStartSeconds = FPlatformTime::Seconds();
}

void ATerraDyneOSS_Phase2BGameMode::FinishSequence(const TCHAR* StatusLine)
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;
	Stage = EPhase2Stage::Finished;
	SetActorTickEnabled(false);
	DestroyCurrentChunk();

	UE_LOG(LogTerraDyneOSS_Proto, Log,
		TEXT("PHASE2B SUMMARY executed=%d aborted=%d"), CasesExecuted, CasesAborted);
	UE_LOG(LogTerraDyneOSS_Proto, Log, TEXT("PHASE2B STATUS %s"), StatusLine);
	UE_LOG(LogTerraDyneOSS_Proto, Log, TEXT("PHASE2B DONE"));

	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("QUIT"));
	}
}

void ATerraDyneOSS_Phase2BGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFinished || Stage == EPhase2Stage::Idle || Stage == EPhase2Stage::Finished)
	{
		return;
	}

	const double Now = FPlatformTime::Seconds();
	const double StageElapsed = Now - StageStartSeconds;

	switch (Stage)
	{
	case EPhase2Stage::PrepareConfig:
	{
		if (ConfigIndex >= ConfigCount)
		{
			FinishSequence(TEXT("SEQUENCE COMPLETE"));
			return;
		}
		if (!SpawnChunkForCurrentConfig())
		{
			AbortCurrentConfig(TEXT("config init rejected"));
			return;
		}
		CaseIndex = 0;
		Stage = EPhase2Stage::WaitInitialSurface;
		StageStartSeconds = Now;
		break;
	}
	case EPhase2Stage::WaitInitialSurface:
	{
		FSurfaceSample Probe;
		CaptureSurface(Probe, TerrainChunk.IsValid() ? TerrainChunk->GetActorLocation() : FVector::ZeroVector);
		const bool bReady = Probe.bMeshComp && Probe.MeshBoundsExtentX > 1.0f && Probe.bComponentTraceHit;
		if (bReady || StageElapsed >= WaitAbortSeconds)
		{
			UE_LOG(LogTerraDyneOSS_Proto, Log,
				TEXT("PHASE2B SURFACE config=%s ready=%s elapsed=%.3f ExtentX=%.3f CompHit=%s"),
				Configs[ConfigIndex].Name,
				bReady ? TEXT("true") : TEXT("false"),
				StageElapsed,
				Probe.MeshBoundsExtentX,
				Probe.bComponentTraceHit ? TEXT("true") : TEXT("false"));
			if (!bReady)
			{
				AbortCurrentConfig(TEXT("initial mesh/collision not ready"));
				return;
			}
			Stage = EPhase2Stage::VerifyBaseline;
			StageStartSeconds = Now;
		}
		break;
	}
	case EPhase2Stage::VerifyBaseline:
	{
		if (CaseIndex >= CasesPerConfig)
		{
			Stage = EPhase2Stage::Advance;
			break;
		}

		ATerraDyneChunk* Chunk = TerrainChunk.Get();
		if (!Chunk)
		{
			AbortCurrentConfig(TEXT("chunk lost before case"));
			return;
		}

		ComputeCasePoints();
		BaselineBuffer = Chunk->HeightBuffer;
		float MaxAbs = 0.0f;
		int32 MaxAbsIdx = INDEX_NONE;
		for (int32 i = 0; i < Chunk->HeightBuffer.Num(); ++i)
		{
			const float AbsVal = FMath::Abs(Chunk->HeightBuffer[i]);
			if (AbsVal > MaxAbs)
			{
				MaxAbs = AbsVal;
				MaxAbsIdx = i;
			}
		}
		UE_LOG(LogTerraDyneOSS_Proto, Log,
			TEXT("PHASE2B BASELINE %s_%s samples=%d maxAbs=%.8f idx=%d"),
			Configs[ConfigIndex].Name, Cases[CaseIndex].Id, Chunk->HeightBuffer.Num(), MaxAbs, MaxAbsIdx);

		if (MaxAbs > KINDA_SMALL_NUMBER)
		{
			AbortCurrentConfig(TEXT("height residual before case"));
			return;
		}

		CaptureSurface(BaselineSurface, BrushWorld);
		LogSurface(TEXT("BASELINE_SURFACE"), BaselineSurface);
		Stage = EPhase2Stage::ApplyRaise;
		break;
	}
	case EPhase2Stage::ApplyRaise:
	{
		ATerraDyneManager* Manager = TerrainManager.Get();
		ATerraDyneChunk* Chunk = TerrainChunk.Get();
		if (!Manager || !Chunk)
		{
			AbortCurrentConfig(TEXT("lost actors before raise"));
			return;
		}

		UE_LOG(LogTerraDyneOSS_Proto, Log,
			TEXT("PHASE2B RAISE %s_%s ApplyGlobalBrush void World=%s Radius=%.6f Strength=%.6f GPU=%s"),
			Configs[ConfigIndex].Name,
			Cases[CaseIndex].Id,
			*BrushWorld.ToString(),
			BrushRadius,
			BrushStrength,
			Chunk->IsUsingGPU() ? TEXT("true") : TEXT("false"));

		Manager->ApplyGlobalBrush(
			BrushWorld,
			BrushRadius,
			BrushStrength,
			ETerraDyneBrushMode::Raise,
			ETerraDyneLayer::Sculpt,
			0,
			0.0f);

		RaiseStats = ComputeBufferStats(BaselineBuffer);
		LogBufferStats(TEXT("RAISE_CPU"), RaiseStats);
		CaptureSurface(RaiseImmediate, BrushWorld);
		LogSurface(TEXT("RAISE_IMMEDIATE"), RaiseImmediate);
		bRaiseCpuChanged = RaiseStats.ChangedCount > 0;
		bRaiseMeshObserved = false;
		bRaiseCollisionObserved = false;

		UE_LOG(LogTerraDyneOSS_Proto, Log,
			TEXT("PHASE2B RAISE_RESULT %s_%s cpuChanged=%s changed=%d absMaxD=%.8f apiBrush=%.6f"),
			Configs[ConfigIndex].Name,
			Cases[CaseIndex].Id,
			bRaiseCpuChanged ? TEXT("true") : TEXT("false"),
			RaiseStats.ChangedCount,
			RaiseStats.AbsMaxDelta,
			RaiseImmediate.ApiWorldBrush);

		Stage = EPhase2Stage::WaitRaiseSurface;
		StageStartSeconds = Now;
		break;
	}
	case EPhase2Stage::WaitRaiseSurface:
	{
		FSurfaceSample Probe;
		CaptureSurface(Probe, BrushWorld);
		if (bRaiseCpuChanged)
		{
			if (!bRaiseMeshObserved && HasMeshBoundsDelta(BaselineSurface, Probe))
			{
				bRaiseMeshObserved = true;
				UE_LOG(LogTerraDyneOSS_Proto, Log,
					TEXT("PHASE2B RAISE_MESH %s_%s t=%.4fs MaxZ %.6f -> %.6f"),
					Configs[ConfigIndex].Name, Cases[CaseIndex].Id, Probe.Seconds,
					BaselineSurface.MeshBoundsMaxZ, Probe.MeshBoundsMaxZ);
			}
			if (!bRaiseCollisionObserved && HasComponentTraceDelta(BaselineSurface, Probe))
			{
				bRaiseCollisionObserved = true;
				UE_LOG(LogTerraDyneOSS_Proto, Log,
					TEXT("PHASE2B RAISE_COLLISION %s_%s t=%.4fs CompZ %.6f -> %.6f"),
					Configs[ConfigIndex].Name, Cases[CaseIndex].Id, Probe.Seconds,
					BaselineSurface.ComponentTraceZ, Probe.ComponentTraceZ);
			}
			if ((bRaiseMeshObserved && bRaiseCollisionObserved) || StageElapsed >= WaitAbortSeconds)
			{
				RaiseDeferred = Probe;
				LogSurface(TEXT("RAISE_DEFERRED"), RaiseDeferred);
				if (!(bRaiseMeshObserved && bRaiseCollisionObserved))
				{
					UE_LOG(LogTerraDyneOSS_Proto, Warning,
						TEXT("PHASE2B RAISE_WAIT_ABORT %s_%s mesh=%s collision=%s elapsed=%.3f"),
						Configs[ConfigIndex].Name, Cases[CaseIndex].Id,
						bRaiseMeshObserved ? TEXT("true") : TEXT("false"),
						bRaiseCollisionObserved ? TEXT("true") : TEXT("false"),
						StageElapsed);
				}
				Stage = EPhase2Stage::ApplyLower;
			}
		}
		else
		{
			if (StageElapsed >= 0.25f || StageElapsed >= WaitAbortSeconds)
			{
				RaiseDeferred = Probe;
				LogSurface(TEXT("RAISE_DEFERRED_NOCPU"), RaiseDeferred);
				UE_LOG(LogTerraDyneOSS_Proto, Log,
					TEXT("PHASE2B SKIP_LOWER %s_%s no CPU delta — Lower not applied"),
					Configs[ConfigIndex].Name, Cases[CaseIndex].Id);
				CasesExecuted++;
				CaseIndex++;
				Stage = EPhase2Stage::VerifyBaseline;
			}
		}
		break;
	}
	case EPhase2Stage::ApplyLower:
	{
		ATerraDyneManager* Manager = TerrainManager.Get();
		ATerraDyneChunk* Chunk = TerrainChunk.Get();
		if (!Manager || !Chunk)
		{
			AbortCurrentConfig(TEXT("lost actors before lower"));
			return;
		}

		UE_LOG(LogTerraDyneOSS_Proto, Log,
			TEXT("PHASE2B LOWER %s_%s ApplyGlobalBrush void World=%s Radius=%.6f Strength=%.6f"),
			Configs[ConfigIndex].Name, Cases[CaseIndex].Id, *BrushWorld.ToString(), BrushRadius, BrushStrength);

		Manager->ApplyGlobalBrush(
			BrushWorld,
			BrushRadius,
			BrushStrength,
			ETerraDyneBrushMode::Lower,
			ETerraDyneLayer::Sculpt,
			0,
			0.0f);

		const FBufferStats LowerStats = ComputeBufferStats(BaselineBuffer);
		LogBufferStats(TEXT("LOWER_VS_BASELINE"), LowerStats);
		CaptureSurface(LowerImmediate, BrushWorld);
		LogSurface(TEXT("LOWER_IMMEDIATE"), LowerImmediate);
		bLowerMeshObserved = false;
		bLowerCollisionObserved = false;
		Stage = EPhase2Stage::WaitLowerSurface;
		StageStartSeconds = Now;
		break;
	}
	case EPhase2Stage::WaitLowerSurface:
	{
		FSurfaceSample Probe;
		CaptureSurface(Probe, BrushWorld);
		if (!bLowerMeshObserved && HasMeshBoundsDelta(RaiseDeferred, Probe))
		{
			bLowerMeshObserved = true;
			UE_LOG(LogTerraDyneOSS_Proto, Log,
				TEXT("PHASE2B LOWER_MESH %s_%s t=%.4fs MaxZ %.6f -> %.6f"),
				Configs[ConfigIndex].Name, Cases[CaseIndex].Id, Probe.Seconds,
				RaiseDeferred.MeshBoundsMaxZ, Probe.MeshBoundsMaxZ);
		}
		if (!bLowerCollisionObserved && HasComponentTraceDelta(RaiseDeferred, Probe))
		{
			bLowerCollisionObserved = true;
			UE_LOG(LogTerraDyneOSS_Proto, Log,
				TEXT("PHASE2B LOWER_COLLISION %s_%s t=%.4fs CompZ %.6f -> %.6f"),
				Configs[ConfigIndex].Name, Cases[CaseIndex].Id, Probe.Seconds,
				RaiseDeferred.ComponentTraceZ, Probe.ComponentTraceZ);
		}
		if ((bLowerMeshObserved && bLowerCollisionObserved) || StageElapsed >= WaitAbortSeconds)
		{
			LowerDeferred = Probe;
			LogSurface(TEXT("LOWER_DEFERRED"), LowerDeferred);
			ATerraDyneChunk* Chunk = TerrainChunk.Get();
			float MaxAbs = 0.0f;
			if (Chunk)
			{
				for (float Sample : Chunk->HeightBuffer)
				{
					MaxAbs = FMath::Max(MaxAbs, FMath::Abs(Sample));
				}
			}
			UE_LOG(LogTerraDyneOSS_Proto, Log,
				TEXT("PHASE2B RESET %s_%s maxAbsAfterLower=%.8f mesh=%s collision=%s"),
				Configs[ConfigIndex].Name, Cases[CaseIndex].Id, MaxAbs,
				bLowerMeshObserved ? TEXT("true") : TEXT("false"),
				bLowerCollisionObserved ? TEXT("true") : TEXT("false"));

			if (MaxAbs > KINDA_SMALL_NUMBER)
			{
				AbortCurrentConfig(TEXT("residual after Lower"));
				return;
			}

			CasesExecuted++;
			CaseIndex++;
			Stage = EPhase2Stage::VerifyBaseline;
		}
		break;
	}
	case EPhase2Stage::Advance:
	{
		DestroyCurrentChunk();
		ConfigIndex++;
		CaseIndex = 0;
		Stage = EPhase2Stage::PrepareConfig;
		break;
	}
	default:
		break;
	}
}

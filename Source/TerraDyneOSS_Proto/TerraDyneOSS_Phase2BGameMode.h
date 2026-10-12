#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TerraDyneOSS_Phase2BGameMode.generated.h"

class ATerraDyneManager;
class ATerraDyneChunk;

UCLASS()
class TERRADYNEOSS_PROTO_API ATerraDyneOSS_Phase2BGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATerraDyneOSS_Phase2BGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	enum class EPhase2Stage : uint8
	{
		Idle,
		PrepareConfig,
		WaitInitialSurface,
		VerifyBaseline,
		ApplyRaise,
		WaitRaiseSurface,
		ApplyLower,
		WaitLowerSurface,
		Advance,
		Finished
	};

	struct FSurfaceSample
	{
		float ApiLocalBrush = 0.0f;
		float ApiWorldBrush = 0.0f;
		float ApiLocalNearest = 0.0f;
		float MeshBoundsMinZ = 0.0f;
		float MeshBoundsMaxZ = 0.0f;
		float MeshBoundsExtentX = 0.0f;
		bool bMeshComp = false;
		bool bCollisionEnabled = false;
		bool bComponentTraceHit = false;
		float ComponentTraceZ = 0.0f;
		FString ComponentHitName;
		bool bWorldTraceHit = false;
		float WorldTraceZ = 0.0f;
		FString WorldHitName;
		double Seconds = 0.0;
	};

	struct FBufferStats
	{
		int32 ChangedCount = 0;
		int32 FirstChangedIndex = INDEX_NONE;
		int32 LastChangedIndex = INDEX_NONE;
		int32 MaxAbsIndex = INDEX_NONE;
		float MinDelta = 0.0f;
		float MaxDelta = 0.0f;
		float AbsMaxDelta = 0.0f;
		float SumDelta = 0.0f;
		float MaxAbsValue = 0.0f;
		int32 MaxAbsValueIndex = INDEX_NONE;
	};

	bool SpawnManager();
	bool SpawnChunkForCurrentConfig();
	void DestroyCurrentChunk();
	void ComputeCasePoints();
	FVector VertexWorld(int32 X, int32 Y) const;
	void CaptureSurface(FSurfaceSample& OutSample, const FVector& TraceWorld) const;
	void LogSurface(const TCHAR* Label, const FSurfaceSample& Sample) const;
	FBufferStats ComputeBufferStats(const TArray<float>& Reference) const;
	void LogBufferStats(const TCHAR* Label, const FBufferStats& Stats) const;
	bool HasMeshBoundsDelta(const FSurfaceSample& Before, const FSurfaceSample& After) const;
	bool HasComponentTraceDelta(const FSurfaceSample& Before, const FSurfaceSample& After) const;
	void FinishSequence(const TCHAR* StatusLine);
	void AbortCurrentConfig(const TCHAR* Reason);

	static constexpr int32 ConfigCount = 3;
	static constexpr int32 CasesPerConfig = 4;

	TWeakObjectPtr<ATerraDyneManager> TerrainManager;
	TWeakObjectPtr<ATerraDyneChunk> TerrainChunk;
	EPhase2Stage Stage = EPhase2Stage::Idle;
	int32 ConfigIndex = 0;
	int32 CaseIndex = 0;
	FVector BrushWorld = FVector::ZeroVector;
	FVector NearestVertexWorld = FVector::ZeroVector;
	int32 NearestVertexIndex = INDEX_NONE;
	float CurrentStep = 0.0f;
	float BrushRadius = 0.0f;
	float BrushStrength = 80.0f;
	double SequenceStartSeconds = 0.0;
	double StageStartSeconds = 0.0;
	TArray<float> BaselineBuffer;
	FSurfaceSample BaselineSurface;
	FSurfaceSample RaiseImmediate;
	FSurfaceSample RaiseDeferred;
	FSurfaceSample LowerImmediate;
	FSurfaceSample LowerDeferred;
	FBufferStats RaiseStats;
	bool bRaiseCpuChanged = false;
	bool bRaiseMeshObserved = false;
	bool bRaiseCollisionObserved = false;
	bool bLowerMeshObserved = false;
	bool bLowerCollisionObserved = false;
	bool bFinished = false;
	int32 CasesExecuted = 0;
	int32 CasesAborted = 0;
};

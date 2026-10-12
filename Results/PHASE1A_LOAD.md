# Phase 1A — plugin load after EngineVersion 5.8.0

Does not overwrite PHASE1_RUNTIME.md or phase1_init.log.

## Résultat

Plugin TerraDyne **chargé**. Aucun manager, aucun chunk (attendu : carte OpenWorld + GameModeBase).

## .uplugin

Isolated copy only:
- Before (saved as PHASE1A_TerraDyne.uplugin.before.json): EngineVersion 5.7.0
- After: EngineVersion 5.8.0
- Public clone C:\Users\sahel\AppData\Local\Temp\TerraDyne_audit still 5.7.0
- GardenFervor has no TerraDyne plugin

This is a load-metadata change, not a compatibility validation.

## Compilation

Not rerun. Descriptor-only change. Binaries remain 2026-10-11 18:45.

## Load proof (phase1a_load.log)

Absent: "Skipping load of 'TerraDyne'", "is Incompatible", "requires engine version '5.7.0'".

Present:
- LogPluginManager: Mounting Project plugin TerraDyne
- InternalLoadLibrary: TerraDyne → Plugins/TerraDyne/Binaries/Win64/UnrealEditor-TerraDyne.dll
- LogTerraDyne: TerraDyne Runtime Module Started.
- LogTerraDyne: TerraDyneSubsystem: Initializing for World OpenWorld
- Load map complete /Engine/Maps/Templates/OpenWorld
- Game class is GameModeBase

Warnings/errors that did not prevent load:
- CDO TerraDyneEditController: missing /TerraDyne/Materials/M_TerraDyne_BrushPreview (stripped OSS asset)
- LogTerraDyne: shader directory was not found (expected CPU-only OSS)

No ATerraDyneManager / Spawned chunk / RegisterChunk in the log.

## Host plan (not implemented, Raise/Lower not run)

Do not rely on spawning the manager alone.
- bSpawnDefaultChunksOnBeginPlay defaults false.
- Manager BeginPlay only calls SpawnDefaultSandboxChunk if that flag is true.
- SpawnConfiguredChunk + chunk BeginPlay call Initialize(), which allocates BaseBuffer/SculptBuffer only.
- HeightBuffer, DetailBuffer, mesh/collision dirty flags are set in InitializeChunk() (public).
- ApplyGlobalBrush only edits chunks already in ActiveChunkMap (RegisterChunk).

Proposed host-only files (next authorization):
1. Source/TerraDyneOSS_Proto/TerraDyneOSS_ProtoGameMode.h/.cpp
2. Config/DefaultEngine.ini: GlobalDefaultGameMode=/Script/TerraDyneOSS_Proto.TerraDyneOSS_ProtoGameMode

GameMode BeginPlay (no plugin edits):
1. Deferred-spawn ATerraDyneManager; GlobalChunkSize=1000; bSpawnDefaultChunksOnBeginPlay=false; FinishSpawning.
2. Spawn ATerraDyneChunk at manager location.
3. Chunk->InitializeChunk(FIntPoint(0,0), 1000.f, 32, nullptr, nullptr).
4. Confirm HeightBuffer.Num()==32*32, Resolution, WorldSize, IsUsingGPU()==false.
5. Log GetHeightAtLocation at a known local point. Do not call ApplyGlobalBrush in the first host compile unless a later mission authorizes Raise.

Launch (same as Phase 1A):
UnrealEditor-Cmd.exe TerraDyneOSS_Proto.uproject /Engine/Maps/Templates/OpenWorld -game -unattended -nopause -nosplash -nullrhi -AbsLog=Results/phase1b_host.log

Later Raise (not this phase):
Manager->ApplyGlobalBrush(WorldCenter, RadiusUU, StrengthUU, ETerraDyneBrushMode::Raise)
then GetWorldHeightAtLocation / HeightBuffer / traces.

## Files this step

Modified: Plugins/TerraDyne/TerraDyne.uplugin (EngineVersion only)
Added: Results/PHASE1A_TerraDyne.uplugin.before.json, PHASE1A_COMPILE.md, PHASE1A_LOAD.md, phase1a_load.log
Not created: GameMode / test actor
GardenFervor: intact (c101261)

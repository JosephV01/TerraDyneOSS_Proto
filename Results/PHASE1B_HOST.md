# Phase 1B — host GameMode and chunk init

Does not overwrite PHASE1A_* or earlier reports.

## Statut

`PASS — CHUNK INITIALISÉ, TERRAFORMATION NON TESTÉE`

No Raise/Lower. Mesh/collision components present; geometry and traces not validated.

## Fichiers

Added (host only):
- Source/TerraDyneOSS_Proto/TerraDyneOSS_ProtoGameMode.h
- Source/TerraDyneOSS_Proto/TerraDyneOSS_ProtoGameMode.cpp

Modified:
- Config/DefaultEngine.ini — GlobalDefaultGameMode=/Script/TerraDyneOSS_Proto.TerraDyneOSS_ProtoGameMode

Plugin C++ unchanged. GardenFervor unchanged (HEAD c101261).

## API / préconditions

- SpawnConfiguredChunk is private. Host cannot call it.
- ActiveChunkMap is private. Verified via GetChunkAtCoord / GetActiveChunkCount.
- Manager BeginPlay registers with UTerraDyneSubsystem. bSpawnDefaultChunksOnBeginPlay defaults false; left false.
- Chunk BeginPlay calls Initialize() only (Base/Sculpt). HeightBuffer filled by InitializeChunk.
- InitializeChunk clamps resolution 32–256, allocates Resolution² samples, SetupGPU (off if !CanEverRender or !bEnableGPUBrushes), sets bInitialized.
- RegisterChunk (public) / chunk BeginPlay after manager exists adds ActiveChunkMap.
- ApplyGlobalBrush not called.

## Paramètres effectifs

- GlobalChunkSize / WorldSize = 1000 UU
- Resolution = 32 → expected HeightBuffer.Num() = 1024
- Coord (0,0) at manager origin
- ProceduralWorldSettings.bEnableSeededOutskirts = false (instance only, predictable zeros)
- bEnableGPUBrushes=false (DefaultGame.ini)
- Launch used -nullrhi → CanEverRender=false

## Ordre

1. Deferred spawn manager; set size/flags; FinishSpawning
2. Confirm subsystem GetTerrainManager()
3. Deferred spawn chunk at Coord*(size); set GridCoordinate; FinishSpawning
4. InitializeChunk((0,0), 1000, 32, null, null)
5. Confirm GetChunkAtCoord, HeightBuffer.Num(), IsUsingGPU(), DynamicMeshComp

## Compilation

- Game Win64 Development: Succeeded
- Editor Win64 Development: required for UnrealEditor-Cmd -game; first Editor link failed (UDynamicMesh::GetTriangleCount). Removed that call (host-only). Editor then Succeeded. Game rebuilt Succeeded.

## Exécution (phase1b_host.log)

- Mounting / InternalLoadLibrary TerraDyne; Runtime Module Started
- Game class is TerraDyneOSS_ProtoGameMode
- STEP OK manager: TerraDyneManager_0 GlobalChunkSize=1000.0 ActiveChunks=0
- bEnableGPUBrushes=false CanEverRender=false
- STEP OK chunk spawn: HeightBuffer=0 before InitializeChunk
- STEP RESULT registered=true ActiveChunks=1 Resolution=32 WorldSize=1000.0 HeightBuffer=1024 expected=1024 GPU=false MeshComp=true CollisionEnabled=true HeightAtOrigin=0.0000
- STEP OK init: chunk initialized, terraform not tested
- No STEP FAIL, no ApplyGlobalBrush

## Limites

- HeightBuffer zeros are init state, not a terraform proof
- MeshComp/CollisionEnabled ≠ rebuilt mesh or Chaos hit
- GPU off due to settings and -nullrhi
- Missing BrushPreview material / shader dir warnings remain
- OpenWorld + one 10 m chunk only

## Étape suivante

Authorize Raise then Lower on this initialized chunk, with T0/T1/T2 CPU / GetHeightAtLocation / traces. Do not treat this PASS as terrain-edit validation.

## Design Gate

Design Gate : aucun statut modifié. F1 reste INVALIDÉ et 08.7 reste À DÉFINIR. Aucun contrat ni choix architectural n’est validé par cette campagne.

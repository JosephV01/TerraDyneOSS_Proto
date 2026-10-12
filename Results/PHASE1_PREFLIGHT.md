# Phase 1 preflight — before launch

Date: 2026-10-11
Project: C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311
OSS commit marker: 270c31117e20e491ef5333dd7e2f9dc133fcdc38
GardenFervor HEAD observed: c101261 (not launched, not modified)

## Config used at runtime
- Config/DefaultGame.ini [/Script/TerraDyne.TerraDyneSettings] bEnableGPUBrushes=false
- Config/DefaultEngine.ini GameDefaultMap=/Engine/Maps/Templates/OpenWorld
- No GlobalDefaultGameMode
- Host module is IMPLEMENT_PRIMARY_GAME_MODULE only (no GameMode, no Exec, no test actor)

## Binary
- TerraDyneOSS_Proto.exe LastWriteTime 2026-10-11 18:45:43 (matches COMPILE_AFTER_ADAPT Game success)

## Map / trigger
- No .umap in the isolated project
- Plugin has BP_TerraDyneManager / BP_TerraDyneChunk, no plugin map
- ATerraDyneManager::bSpawnDefaultChunksOnBeginPlay defaults to false
- ApplyGlobalBrush exists on the manager; nothing in the host calls it
- No host console command
- Plugin Editor automation TerraDyne.Functional.Deformation exists (WITH_EDITOR only); not used as a substitute for this game campaign

Phase A launch uses the isolated .uproject and the engine OpenWorld template to observe actual initialization.

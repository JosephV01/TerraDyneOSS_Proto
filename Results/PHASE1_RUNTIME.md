# TerraDyne OSS Phase 1 runtime — 2026-10-11

Does not overwrite REPORT.md / ADAPTATIONS.md / COMPILE_AFTER_ADAPT.md.

## Résultat

`BLOQUÉ — INITIALISATION`

The isolated game started. The TerraDyne plugin did not load. No chunks, no CPU brush, no mesh/collision measurements.

## Conditions de test

- Engine: 5.8.3-58210709 (log: enginereleaseversion=5.8.3)
- Project: TerraDyneOSS_Proto.uproject (isolated)
- Command:
  UnrealEditor-Cmd.exe TerraDyneOSS_Proto.uproject /Engine/Maps/Templates/OpenWorld -game -unattended -nopause -nosplash -nullrhi -AbsLog=Results/phase1_init.log
- Binary host DLL loaded: UnrealEditor-TerraDyneOSS_Proto.dll
- Game exe from last compile: TerraDyneOSS_Proto.exe 2026-10-11 18:45:43
- Map: /Engine/Maps/Templates/OpenWorld
- Game class: GameModeBase
- Config on disk: [/Script/TerraDyne.TerraDyneSettings] bEnableGPUBrushes=false
- GPU path: not observable — plugin skipped before settings apply
- Brush: not attempted (mandatory stop)

## Phase A observations

Verified:
- Launch of isolated project: yes
- Map load complete: yes (0.39 s)
- TerraDyne world/chunks: no
- Mesh/collision components: none (plugin not loaded)
- CPU path selected: not demonstrated (module never started)

Exact plugin-manager lines from phase1_init.log:

```
LogPluginManager: Warning: Plugin 'TerraDyne' requires engine version '5.7.0' and may not be compatible with the current current engine version '5.8.0-55116800+++UE5+Release-5.8'
Message dialog closed, result: No, title: 'TerraDyne' is Incompatible, text: The 'TerraDyne' plugin was designed for build 5.7.0. Attempt to load it anyway?
LogPluginManager: Display: Skipping load of 'TerraDyne'.
```

Unattended answered No. UnrealEditor-TerraDyne.dll was not loaded.

## Phase B–D

Not started.

## Mesures

None. No HeightBuffer, no GetWorldHeightAtLocation, no traces.

## Logs

See Results/phase1_init.log.
Relevant: EngineVersion mismatch 5.7.0 vs 5.8; plugin skipped.
No LogTerraDyne Error (logger not loaded).
No Fatal.

## Trigger inventory (preflight)

- Host has no GameMode, Exec, or test actor.
- No project .umap.
- ATerraDyneManager::ApplyGlobalBrush exists but is unreachable without a spawned manager.
- bSpawnDefaultChunksOnBeginPlay defaults to false.
- Plugin Editor test TerraDyne.Functional.Deformation exists; unused here (Editor-only, not this campaign).

## Modifications

No plugin or host source changed in this campaign.
GardenFervor not modified.

New files only:
- Results/PHASE1_PREFLIGHT.md
- Results/PHASE1_RUNTIME.md
- Results/phase1_init.log

## Conclusion

Compilation ≠ load. The 5.7.0 EngineVersion in TerraDyne.uplugin blocks runtime load on 5.8.3 when unattended refuses the compatibility dialog.

Even after a load fix, this host still has no TerraDyne map and no brush caller. That is a second, separate gap.

Proposed later corrections (not applied):
1. Isolated copy only: set TerraDyne.uplugin EngineVersion to 5.8.0 (or empty) and relaunch Phase A.
2. After the plugin actually loads: add a host-only GameMode that spawns ATerraDyneManager, initializes one chunk, calls ApplyGlobalBrush Raise/Lower, writes T0/T1/T2 JSON. Do not change plugin behavior.

## Design Gate

Design Gate : aucun statut modifié. F1 reste INVALIDÉ et 08.7 reste À DÉFINIR. Aucun contrat ni choix architectural n’est validé par cette campagne.

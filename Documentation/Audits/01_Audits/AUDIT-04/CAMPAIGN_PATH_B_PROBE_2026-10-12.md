# Campaign Path B Probe — TerraDyne native master on DynamicMesh

| Field | Value |
|---|---|
| Date | **2026-10-12** |
| Id | `EXT-TERRADYNE-A04-PATHB` |
| Feasibility | [`FEASIBILITY_M4_DYNAMICMESH_2026-10-12.md`](FEASIBILITY_M4_DYNAMICMESH_2026-10-12.md) §8 |
| Parent | [`REPORT.md`](REPORT.md) |
| Execution (visual) | **`NOT_RUN`** — awaiting human GPU PIE |
| Code / plugin TerraDyne / M4 / GardenFervor | Host demo **only** modified; plugin & `Content/M4/` untouched |

**Design Gate : Path B Probe uniquement ; essai technique limité et réversible autorisé. Aucune validation de production ni intégration GardenFervor autorisée.**

**Prompt vs §8:** No divergence. This campaign implements the §8 Path B probe (apply `M_TerraDyne_Master` via `SetMaterial`, human accept/reject criteria unchanged).

---

## A. Preparation

### A.1 Initial state

| Item | Value |
|---|---|
| Tip at start | `14e4894` (`origin/main` synced) |
| Untracked | `Results/*.json` (left untouched) |
| Material asset | `Plugins/TerraDyne/Content/Materials/VHFM/M_TerraDyne_Master.uasset` **present** |
| Default plugin path | `UTerraDyneSettings::MasterMaterialPath` → `/TerraDyne/Materials/VHFM/M_TerraDyne_Master` |
| Height RT / Weight | `ATerraDyneChunk::SetupGPU` → `HeightRT`; weight via `WeightTexture` + `UploadWeightTexture` |
| Parameter bus | `ApplyTerrainMaterialParameters` sets `HeightMap`, `WeightMap`, `WorldSize`, `ZScale`, `TexelStep` |

### A.2 References inspected

- Feasibility §8 acceptance criteria  
- `TryApplyM4Material`, `SetMaterial`, `ApplyTerrainMaterialParameters`  
- Host fallback already listed `M_TerraDyne_Master` as tertiary load path  
- C-A04-VIS (Path A grid outcome) — not re-run here  

### A.3 Minimal host changes

| File | Change |
|---|---|
| `TerraDyneGardenDemoHost.*` | `TryApplyTerraDyneMasterMaterial`, `RestoreDemoOverlayMaterial`, `PathBVerdict` |
| `TerraDyneDemoPlayerController.*` | `RequestPathBTest`, `RequestRestoreDemoMaterial` |
| `TerraDyneDemoHUD.*` | Buttons **Test TD Master**, **Restore overlay** |

**Unchanged:** `TryApplyM4Material` behaviour; plugin sources; M4 assets.

### A.4 Effective apply path

```text
HUD "Test TD Master"
  → OnPathB → RequestPathBTest
  → TryApplyTerraDyneMasterMaterial
       LoadObject(/TerraDyne/Materials/VHFM/M_TerraDyne_Master)
       Verify texture params HeightMap + WeightMap declared (else BLOCKED, no substitute)
       LiveChunk->SetMaterial(Master)   // creates MID + ApplyTerrainMaterialParameters
       Suppress host paint overlay (bUseNativeM4Material=true; ColorOverrideMode=None)
       Read back HeightMap/WeightMap from slot 0 MID → PathBVerdict log
```

**Rollback**

1. HUD **Restore overlay** → `ApplyFallbackMaterial()` (VertexColor demo path).  
2. Or **Test M4 MIC** (returns to Path A experiment).  
3. Or stop PIE / relaunch (chunk respawns with default fallback).

---

## B. Compilation

| Field | Value |
|---|---|
| Target | `TerraDyneOSS_ProtoEditor` Win64 Development (UE 5.8) |
| Result | **BLOCKED** — UBT exit 6: *Unable to build while Live Coding is active* |
| Lock owner (observed) | `UnrealEditor.exe` + `LiveCodingConsole` on **this** `TerraDyneOSS_Proto.uproject` |
| Human unblock | In the open editor: **Ctrl+Alt+F11** (Live Coding compile), **or** close the editor and re-run Build.bat |
| Log | `Results/compile_pathb_probe.log` (local) |

Host sources are staged for human compile; no claim that binaries already include Path B until that step succeeds.

---

## C. Human PIE procedure (GPU, not `-nullrhi`)

1. Open `TerraDyneOSS_Proto.uproject` (not GardenFervor).  
2. Map `L_TerraDyneGardenDemo`.  
3. PIE with normal GPU viewport.  
4. Note baseline look (vertex-color overlay).  
5. Click **Test TD Master**.  
6. Status line / Output Log: expect `Path B APPLY` with `HeightMap=bound` and `WeightMap=bound`.  
7. Details → `TerraDyneChunk` → `DynamicMeshComp` Element 0: MID from `M_TerraDyne_Master` (not Island MIC, not `MI_ProcGrid`).  
8. Observe shading vs C-A04-VIS enlarged grid.  
9. **Raise** then **Lower** (or vice versa); note whether material appearance responds to height.  
10. Wait ≥ **30 s**.  
11. Optional: **Restore overlay** to confirm reversibility.  
12. Report observations (do not treat this doc as visual PASS until then).

### §8 criteria checklist (human)

| Criterion | Pending human |
|---|---|
| Element 0 = TD master / MID | ☐ |
| HeightMap + WeightMap bound to real resources | ☐ (log assists) |
| Look ≠ C-A04-VIS enlarged-grid-only | ☐ |
| Visible height response | ☐ |
| Stable ≥ 30 s | ☐ |

**Accept Path B baseline** only if all §8 accept bullets hold.  
**Reject** if grid/error/pink/invisible with params bound.  
Do **not** claim M4 parity, RVT, or GF integration.

---

## D. Execution results

| Field | Value |
|---|---|
| Visual state | **`NOT_RUN`** |
| Parameter bind (code/log) | Pending first human PIE log paste |
| Accept / Reject | **Pending human** |

---

## E. Limits

- Probe only — not production material choice.  
- `bUseNativeM4Material` flag reused to suppress paint overlay (name historical).  
- Full M4 automaterial / RVT / 08.7 / GardenFervor: **out of scope**.  

`Design Gate : Path B Probe uniquement ; essai technique limité et réversible autorisé. Aucune validation de production ni intégration GardenFervor autorisée.`

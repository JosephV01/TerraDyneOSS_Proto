# Feasibility — M4 rendering architectures on TerraDyne DynamicMesh

| Field | Value |
|---|---|
| Date | **2026-10-12** |
| Id | `EXT-TERRADYNE-A04-FEAS` |
| Parent | [`REPORT.md`](REPORT.md) · [`CAMPAIGN_C-A04-VIS_2026-10-12.md`](CAMPAIGN_C-A04-VIS_2026-10-12.md) |
| Nature | Feasibility / diagnostic — **no implementation** |
| Proto | `TerraDyneOSS_Proto_270c311` · plugin OSS **0.4** `270c311` |
| Code / M4 / GardenFervor | **Unchanged** by this mission |

**Design Gate : audit de faisabilité uniquement ; aucune implémentation ni intégration autorisée.**

Certainty labels used below: **démontré** · **observé** · **hypothèse**.

---

## 1. Blocking summary

| Fact | Source | Certainty |
|---|---|---|
| Real MIC `Island_UE5Preset5_Inst` assigns to `DynamicMeshComp` Element 0 | C-A04-VIS human PIE | **démontré** |
| Appearance changes (enlarged **grid**); stable ≥ 30 s | C-A04-VIS | **démontré** |
| Expected Island M4 textures / automaterial **not** identified | C-A04-VIS | **démontré** (negative) |
| Cause of enlarged grid | C-A04-VIS §C | **INDÉTERMINÉE** |
| Master has 25 direct Landscape-named expressions | A04 self-test / VIS log | **démontré** |
| MIC does **not** declare `HeightMap` / `WeightMap` | A04 / VIS log | **observé** |
| TerraDyne `SetMaterial` pushes those names on a MID | `TerraDyneChunk.cpp` | **démontré** |

**Block:** native Island MIC on DynamicMesh is assignable and stable, but does **not** deliver the expected M4 look. Continuing without an architecture choice risks more dead-end MIC experiments.

---

## 2. Observed render chain (code)

```text
HUD "Test M4 MIC"
  → UTerraDyneDemoHUD::OnM4
  → ATerraDyneDemoPlayerController::RequestM4Test
  → ATerraDyneGardenDemoHost::TryApplyM4Material
       loads /Game/M4/.../Island_UE5Preset5_Inst
       LiveChunk->SetMaterial(M4)
       bUseNativeM4Material=true; vertex-color overlay off
  → ATerraDyneChunk::SetMaterial
       requires FApp::CanEverRender()
       MID = Create(InMaterial)
       ApplyTerrainMaterialParameters(MID, HeightRT, WeightTexture, WorldSize, ZScale, Resolution)
       DynamicMeshComp->SetMaterial(0, MID)
  → Mesh: UDynamicMeshComponent (grid Resolution×Resolution)
       UV0: U,V ∈ [0, WorldSize/1000]  (demo WorldSize ≈ 13652.5 → UVTileScale ≈ 13.65)
```

| Step | File / symbol | Observation | Render consequence | Certainty |
|---|---|---|---|---|
| Geometry | `ATerraDyneChunk::InitializeTerrainMeshTopology` | Heightfield grid; `ChunkResolution=128` (host) | Dense mesh; independent of M4 | **démontré** |
| UV scale | same; `UVTileScale = WorldSize / 1000.f` | UV span ≈ **13.65** across chunk (not 0–1) | Any UV-tiled sampler repeats ~14×; may look like a coarse grid | **observé** (math); grid **cause** still **hypothèse** |
| Vertex colors | Host `RecolorMeshFromPaint` when `!bUseNativeM4Material` | Overlay off after M4 apply | Baseline paint stripes removed after Test M4 | **démontré** |
| Material slot | `SetMaterial` → slot **0** | Matches human Element 0 | Assignment path correct | **démontré** |
| Parameters pushed | `ApplyTerrainMaterialParameters` | `HeightMap`, `WeightMap`, `WorldSize`, `ZScale`, `TexelStep` | **No effect** on M4 if params undeclared | **démontré** (push) + **observé** (undeclared on MIC) |
| Default host look | `ApplyFallbackMaterial` → `VertexColorViewMode_ColorOnly` | Pre-M4 | Not M4 | **démontré** |
| Template floor | `HideTemplateFloor` hides actors named Floor / `Template_Map_Floor` | Engine template floor | Distinct from DynamicMesh | **démontré** |
| `MI_ProcGrid` | Referenced in `L_TerraDyneGardenDemo.umap` (Engine OpenWorldTemplate) | Map asset reference | **Must not** be confused with chunk material; not the SetMaterial target | **observé** |

Plugin adoption guard (import path only): `UsesLandscapeOnlyMaterialExpressions` rejects materials containing Landscape LayerBlend / LayerSample / GrassOutput / … (`TerraDyneManager.cpp`). Same expression classes are present on `M4UE5MasterMat` (A04). Consequence: OSS would **refuse** adopting this master as Landscape→chunk master material. Certainty: **démontré** for the guard; VIS already applied MIC via host anyway.

---

## 3. Material graph / inputs

| Input expected by TerraDyne runtime | Consumed by Island MIC / M4UE5MasterMat? | Certainty |
|---|---|---|
| Texture `HeightMap` | **No** (undeclared) | **observé** |
| Texture `WeightMap` | **No** (undeclared) | **observé** |
| Scalars `WorldSize`, `ZScale`, `TexelStep` | Not shown as used by M4 diagnostics | **hypothèse** (not sampled in A04) |
| Landscape layer weights / LayerBlend | Master contains LayerBlend + LayerSample×N | **démontré** (expression inventory) |
| DynamicMesh UV0 / world position | Depends on static switches (World Position UVs = **false** on Island MIC) | **observé** (switch) |

**Consequence:** the data TerraDyne actually maintains (height RT, 4-channel weight texture, mesh UVs) is on a **different bus** than the Landscape-oriented M4 master. HeightMap/WeightMap mismatch does **not** by itself prove the enlarged-grid cause (**hypothèse** forbidden as demonstrated cause — C-A04-VIS).

Official automaterial layers (Flatland / Cliff / Beach) are documented for **Landscape / demo map** height-based look ([Automaterial layers](https://aaronneal.online/docs/m4/using-the-automaterials/automaterial-layers/)). Paint is documented as **Landscape Paint** ([Painting on the Terrain](https://aaronneal.online/docs/m4/getting-started/painting-on-the-terrain/)). Neither document describes TerraDyne `UDynamicMeshComponent`.

---

## 4. Official M4 documentation findings

Index: [https://aaronneal.online/docs/m4/](https://aaronneal.online/docs/m4/)

| Topic | Official statement | Relevance | Certainty |
|---|---|---|---|
| Paint | Landscape mode, 18 paint layers | Host RGB paint ≠ M4 paint | **démontré** (doc) |
| Automaterial | Height/slope layers on demo terrains | Assumes Landscape-like height context | **démontré** (doc intent) |
| Voxel path | Install Voxel Plugin → Voxel World → Voxel Landscape Material Collection → assign Island preset → toggle **Enable Voxel Plugin Support** → Multi Index | Official **non-Landscape** route is **Voxel World**, not DynamicMesh | **démontré** (doc) |
| Voxel ≠ TerraDyne | Docs never name TerraDyne / DynamicMesh | Do **not** equate Voxel-ready MIC with DynamicMesh compatibility | **démontré** (absence) |
| RVT / VHFM | Project VT support; RVT volume; edit Landscape MIC not VHFM mesh MIC | Separate stack; Island MIC RVT switch = false in A04 | **démontré** (doc + A04) |
| Splat bake | Uses RVT; troubleshooting for solid/strange tiling on Landscape | Not a DynamicMesh procedure | **démontré** (doc) |

Forums: **not used**.

---

## 5. Comparison A / B / C

### A — Keep current M4 master/MIC on DynamicMesh

| | |
|---|---|
| Data fit | Landscape expressions need Landscape (or Voxel Multi-Index) inputs; DynamicMesh supplies HeightRT/WeightTex/UVs that M4 does not declare | 
| Gaps | LayerBlend/LayerSample/Grass; HeightMap/WeightMap unused; Voxel & World Position switches **false** on Island MIC |
| Changes if pursued | New MIC with static switches; still **no** official DynamicMesh path; possible master rewrite (forbidden as “solution” without separate auth); UV scale experiment |
| Status after C-A04-VIS | Native Island MIC path **visually insufficient** for expected M4 look | 
| Verdict | **Not viable as-is** for expected M4 Island look on TerraDyne DynamicMesh. Residual switch experiments are secondary only. |

### B — TerraDyne-compatible (non-Landscape) material path

| | |
|---|---|
| Fit | Plugin already designs for `HeightMap` / `WeightMap` via `ApplyTerrainMaterialParameters` |
| Existing asset | `/TerraDyne/Materials/VHFM/M_TerraDyne_Master` (plugin Content) — intended master for this bus |
| M4 reuse | Textures / look can be **sampled** later into a TD-shaped material; **not** the unaltered Landscape master |
| Lost vs full M4 | Landscape paint layers, GrassOutput, official automaterial LayerBlend, RVT Landscape producer path — must be rebuilt or dropped |
| Not “grid substitute” | Goal = height/weight-driven terrain shading on DynamicMesh, not VertexColor / ProcGrid |
| Verdict | **Best aligned** with current TerraDyne runtime architecture; lowest dependency on foreign middleware |

### C — Official alternate terrain (Voxel Plugin)

| | |
|---|---|
| Conditions | Install Voxel Plugin; Voxel World; Material Collection; Enable Voxel Plugin Support on MIC |
| Limits | Replaces / sidesteps TerraDyne DynamicMesh as the rendered terrain; RVT/VHFM docs still Landscape-centric |
| Cost | New plugin, new world type, training, possible license; **out of scope** to install here |
| Verdict | Documented for M4↔Voxel — **not** a drop-in fix for TerraDyne chunks; do not confuse with Path A |

---

## 6. Risks, unknowns, discriminating tests

| Item | Type | Minimal discriminating test |
|---|---|---|
| Cause of enlarged grid | Unknown | Compare Element 0 material name + UV visualization (e.g. debug UV material) vs M4 MIC — **hypothesis** until run |
| UVTileScale ≈ 13.65 | Observed math | Temporary UV 0–1 mesh or material that draws UV checker — **not run** here |
| `MI_ProcGrid` / WorldGrid in map | Observed refs | Confirm Outliner: grid not from hidden Floor / leftover Landscape using ProcGrid while looking at chunk | 
| World Position UV MIC | Hypothesis | Editor: duplicate Island MIC under `Content/Demo/` (not `Content/M4/`), enable World Position UVs, assign to Element 0 — accept only if Island-like automaterial appears |
| Path B bus | Untested visually on this host after M4 work | Apply `M_TerraDyne_Master` **through `SetMaterial`** so HeightMap/WeightMap bind — see §7 |

---

## 7. Single recommendation (next mission)

**Pursue Path B first:** validate a **TerraDyne-native** material on `DynamicMeshComp` that consumes `HeightMap` / `WeightMap` (starting from `M_TerraDyne_Master`), **without** treating unaltered `Island_UE5Preset5_Inst` as the render solution.

**Why before A residual / C**

1. C-A04-VIS already showed Path A (native Island MIC) fails the expected look.  
2. Official docs offer Voxel (C), not DynamicMesh — C abandons the TerraDyne mesh under test.  
3. Path B matches the parameters the plugin already writes every `SetMaterial` call.

**Why not implement in this mission:** Design Gate forbids code/material changes here; Path B needs a **authorized** minimal host probe (or equivalent) so `SetMaterial` binds RTs.

**Path A / C:** keep as documented alternatives only after Path B accept/reject.

---

## 8. Acceptance criteria for the next mission (Path B probe)

Authorize a **narrow** follow-up (host-only or strictly manual equivalent):

1. Apply `/TerraDyne/Materials/VHFM/M_TerraDyne_Master` (or a Demo MIC child) via `ATerraDyneChunk::SetMaterial` so `ApplyTerrainMaterialParameters` runs.  
2. Confirm Details Element 0 is that material/MID (not Island MIC, not VertexColor-only, not `MI_ProcGrid`).  
3. **Accept Path B baseline** if, in GPU PIE:  
   - shading is **not** the enlarged-grid-only look of C-A04-VIS; **and**  
   - Raise/Lower (or height variation) produces a **visible** material response consistent with height/weight sampling; **and**  
   - session stable ≥ 30 s.  
4. **Reject Path B baseline** if still grid/error/pink with parameters bound, or mesh invisible.  
5. Explicitly **out of scope** for that mission: full M4 automaterial parity, RVT, GardenFervor, 08.7, publishing `Content/M4/`.

Optional later (only if Path B accepted): document how selected **M4 textures** could feed a TD-shaped material — still not the Landscape master as-is.

---

## 9. Register coherence

No register status change in this feasibility mission. `C-A04-VIS` remains `PARTIAL`. `C-A04-RVT` remains `NOT_DEMONSTRATED`.

`Design Gate : audit de faisabilité uniquement ; aucune implémentation ni intégration autorisée.`

# Campaign C-A04-VIS — Visual M4 on DynamicMesh (preparation)

| Field | Value |
|---|---|
| Date | **2026-10-12** |
| Id | `EXT-TERRADYNE-A04-VIS` |
| Parent audit | [`REPORT.md`](REPORT.md) (`EXT-TERRADYNE-A04`) |
| Register item | `C-A04-VIS` — status **unchanged**: `NOT_DEMONSTRATED` |
| Proto | `C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311` |
| Public remote | [JosephV01/TerraDyneOSS_Proto](https://github.com/JosephV01/TerraDyneOSS_Proto) |
| Code / plugin / M4 assets / GardenFervor | **Not modified** by this mission |
| Execution state | **`NOT_RUN`** (human PIE required) |
| RVT (`C-A04-RVT`) | **Out of scope** — remains `NOT_DEMONSTRATED` |

**Design Gate : validation visuelle uniquement ; aucune intégration technique, modification du code ou décision 08.7 autorisée.**

This document prepares a **manual** Unreal Editor + PIE check. It does **not** claim that PIE was executed.

---

## A. Preparation

### A.1 Files inspected (local; blob-matched to public where checked)

| Path | Role |
|---|---|
| `Documentation/Audits/README.md` | Entry + publication rules |
| `Documentation/Audits/PUBLICATION_CONTROL.md` | Exclusions (`Content/M4/`, binaries, Saved) |
| `Documentation/Audits/PROVENANCE.md` | Import provenance |
| `Documentation/Audits/01_Audits/AUDIT-04/REPORT.md` | A04 facts (load, Landscape nodes, nullrhi limit) |
| `Documentation/Audits/04_Conclusions_Register/REGISTER.md` | `C-A04-VIS = NOT_DEMONSTRATED` |
| `Documentation/Audits/03_Syntheses/FINAL_SYNTHESIS.md` | Cross-audit context (A01–A03 + M4 notes) |
| `Documentation/Audits/05_Documentary_History/HISTORY.md` | Publication ledger + inclusion rule |
| Commit `1d3d671` | Documentary closure only (README / HISTORY rule / mission report / METHODOLOGY) — **no** proto code |
| `Config/DefaultEngine.ini` | Default map + GameMode |
| `Source/.../TerraDyneGardenDemoHost.cpp` | `TryApplyM4Material`, MIC path, fallback |
| `Source/.../TerraDyneDemoHUD.cpp` | Button **Test M4 MIC** |
| `Source/.../TerraDyneDemoPlayerController.cpp` | `RequestM4Test` → `TryApplyM4Material` |
| `Results/m4_compat_selftest.log` | Prior automated run: load OK, `SLOT0 null`, `VISUAL_M4 non vérifié` |
| `Results/M4_NODE_COMPAT.md` | Host note |
| Disk | `Content/Demo/L_TerraDyneGardenDemo.umap` present; `Content/M4/.../Island_UE5Preset5_Inst.uasset` present; `M4UE5MasterMat.uasset` present |

Local ↔ public blob match for the seven priority docs above: **MATCH** at mission start (`main` @ `6edcb57`).

### A.2 Official M4 documentation consulted

Primary index: [https://aaronneal.online/docs/m4/](https://aaronneal.online/docs/m4/)

| Page | Relevant takeaway for this campaign |
|---|---|
| [Painting on the Terrain](https://aaronneal.online/docs/m4/getting-started/painting-on-the-terrain/) | Paint uses **Landscape** mode (Shift+2), **18 paint layers**. This is **not** the TerraDyne host paint overlay. |
| [Creating your own paint layer](https://aaronneal.online/docs/m4/paint-layer-customization/creating-your-own-paint-layer/) | Paint layers hook into the Master **Layer Blend**; painting is a Landscape workflow. |
| [Automaterial layers](https://aaronneal.online/docs/m4/using-the-automaterials/automaterial-layers/) | Flatland A/B, Foothills, Cliff, Beach, Wet Beach — height/slope-style automaterial look on the **demo maps** (Landscape context). |
| [Using materials with Voxel Plugin](https://aaronneal.online/docs/m4/voxel-plugin-easy-setup/using-the-materials-with-voxel-plugin/) | Official **non-Landscape** path requires Voxel Plugin + MIC switch **Enable Voxel Plugin Support**. That path is **not** TerraDyne DynamicMesh and is **not** enabled on `Island_UE5Preset5_Inst` per A04 (`false`). |

**Do not assume** a Landscape-oriented master works correctly on `UDynamicMeshComponent`. A04 already listed 25 direct Landscape expressions on `M4UE5MasterMat`. This campaign only asks whether a **real GPU render** appears after assignment.

Forums: **not consulted** (official docs answered the questions needed for the protocol).

### A.3 Identified map, actors, material, instance

| Item | Exact identity |
|---|---|
| Project file | `TerraDyneOSS_Proto.uproject` (**not** GardenFervor) |
| Map | `/Game/Demo/L_TerraDyneGardenDemo` (`Content/Demo/L_TerraDyneGardenDemo.umap`) |
| GameMode | `/Script/TerraDyneOSS_Proto.TerraDyneGardenDemoGameMode` |
| Host actor (class) | `ATerraDyneGardenDemoHost` (spawned by GameMode) |
| Terrain actors (runtime) | `ATerraDyneManager` + `ATerraDyneChunk` (spawned by host `SpawnTerrain`) |
| Mesh component | `ATerraDyneChunk::DynamicMeshComp` (`UDynamicMeshComponent`) |
| Target MIC | `/Game/M4/Materials/PresetInstances/Island/Island_UE5Preset5_Inst` |
| Master parent | `/Game/M4/Materials/MasterMaterials/M4UE5MasterMat` |
| Apply entry point | HUD button label **`Test M4 MIC`** → `UTerraDyneDemoHUD::OnM4` → `RequestM4Test` → `TryApplyM4Material` |
| Default look before M4 | Fallback `VertexColorViewMode_ColorOnly` + host paint vertex-color overlay (`bUseNativeM4Material=false`) |
| Orphan path (must **not** be the success target) | `/Game/Demo/M4_Eval/Island_UE5Preset5_Inst` (loads but incomplete vs real M4 tree) |

### A.4 Distinctions (do not conflate)

| Layer | What proves it | What does **not** prove it |
|---|---|---|
| Resource load | Content Browser / prior A04 17/17 | Visible terrain look |
| Material compile | Output Log: no fatal shader errors for the assigned MIC | Asset exists on disk |
| Assignment | Details: Element 0 = `Island_UE5Preset5_Inst` (or MID created from it) after **Test M4 MIC** | Log line alone / nullrhi `SLOT0 null` |
| Real visual render | Viewport (Editor and/or PIE) shows a **non-fallback** shaded surface | `-nullrhi`, Shipping screenshot of vertex-color paint demo |
| Layers / parameters | Recognisable Island automaterial / textured look **or** explicit failure modes (pink, black, flat grey, WorldGrid) | Host RGB paint buttons (those are **not** M4 Landscape layers) |
| PIE session stability | Mesh remains assigned and visible for ~30+ s without crash | Single frame flash |

### A.5 Manual procedure (human)

**Prerequisites**

1. Machine can run Unreal Editor with a real GPU renderer (**no** `-nullrhi`).
2. Open **`TerraDyneOSS_Proto.uproject`** from  
   `C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311`.
3. Confirm startup map is `L_TerraDyneGardenDemo` (DefaultEngine already sets this).
4. Do **not** open GardenFervor for this test.
5. Do **not** edit any M4 asset, plugin, or C++ file.

**Steps**

1. **Content check (Editor, not PIE)**  
   Content Browser → `M4/Materials/PresetInstances/Island/Island_UE5Preset5_Inst`.  
   Open it; confirm Parent = `M4UE5MasterMat`.  
   Optional: note Static Switches (A04: Voxel Support **false**, World Position UVs **false**, RVT Support **false**, UE 5.8 Safe Layer Blending **true**).

2. **Baseline viewport (Editor)**  
   Frame the demo chunk / host ground. Note the **pre-M4** look (typically vertex-color / flat overlay colours).  
   Optional screenshot: `baseline_before_M4`.

3. **Play In Editor**  
   Start **PIE** (Selected Viewport or New Editor Window). Wait until the HUD appears (Raise / Lower / Paint / **Test M4 MIC**).

4. **Assign real MIC**  
   Click **`Test M4 MIC`**.  
   Expected host behaviour (from code): loads real Island MIC, `SetMaterial` on chunk, disables vertex-color overlay (`bUseNativeM4Material=true`).

5. **Observe render**  
   Look at the DynamicMesh chunk **immediately** and after ~2–3 s (shader compile may hitch).  
   Record which of these best matches what you see (pick one primary):  
   - **V1** Textured Island-like automaterial (grass/dirt/rock variation).  
   - **V2** Solid / near-uniform colour (not vertex-colour paint stripes).  
   - **V3** Pink / error material.  
   - **V4** Black / unlit / invisible mesh.  
   - **V5** Unchanged vs baseline vertex-colour overlay (assignment likely failed).  
   - **V6** Other (describe).

6. **Confirm assignment (during PIE if possible)**  
   World Outliner → select the spawned `TerraDyneChunk` (or mesh component).  
   Details → Materials / Element 0: expect `Island_UE5Preset5_Inst` or a Dynamic instance whose parent chain reaches that MIC — **not** `VertexColorViewMode_ColorOnly`, **not** `M_TerraDyne_Master` only, **not** the orphan `Demo/M4_Eval/...` path.

7. **Output Log**  
   Filter `LogTerraDyneGardenDemo` / `Material` / `Shader`. Note compile errors mentioning Landscape or M4.  
   A hitch + successful textured draw can still be a visual PASS candidate; a persistent error material is not.

8. **Negative checks (must not be counted as success)**  
   - Host **Paint** colour buttons changing RGB overlay **before** M4 apply = demo paint, not M4 layers.  
   - Landscape Paint mode (Shift+2) on this DynamicMesh chunk = **not** a valid M4 paint proof here (official paint path is Landscape).  
   - Re-running `-M4CompatSelfTest` / `-nullrhi` = **not** this campaign.

9. **Optional geometry sanity (not required for C-A04-VIS)**  
   Raise then Lower once; confirm the **mesh** still deforms. Material verdict stays independent.

10. **Stability**  
    Remain in PIE ≥ 30 s. Note crash / material reset / mesh disappearance.

11. **Exit**  
    Stop PIE. Optional: repeat once for reproducibility.  
    Save screenshots under a **local** folder only (do **not** commit `Content/M4` or large PNGs unless separately authorized). Suggested names: `C-A04-VIS_before.png`, `C-A04-VIS_after.png`.

### A.6 Expected result if successful (for later closure only)

`C-A04-VIS` may move from `NOT_DEMONSTRATED` only if **all** hold:

1. Real MIC path above was assigned (Details / log consistent).  
2. Viewport shows a real shaded result that is **not** the vertex-colour fallback overlay.  
3. Observation is in Editor GPU and/or PIE **with renderer** (not nullrhi).  
4. Human records V-code + screenshots + brief notes.  
5. RVT is **not** claimed.

A textured automaterial look (V1) is the strongest positive. V2 may be `PARTIAL` depending on notes. V3–V5 are failures of **this** assignment path — **not** automatic proof that “M4 never works anywhere.”

### A.7 Prerequisites / blockages to report if stuck

| Blocker | What to write back |
|---|---|
| Project won’t open / compile | Exact error; stop — no visual verdict |
| Map empty / no chunk | Screenshot Outliner; stop |
| Button **Test M4 MIC** missing | Confirm GameMode / HUD; stop |
| MIC missing under `/Game/M4/...` | `Content/M4` incomplete locally; stop |
| Only nullrhi available | Cannot close C-A04-VIS |

---

## B. Execution

| Field | Value |
|---|---|
| State | **`NOT_RUN`** |
| Operator | *(pending human)* |
| Date/time PIE | — |
| Steps completed | — |
| Observed V-code | — |
| Assignment confirmed | — |
| Output Log notes | — |
| Screenshots | — |
| Stability ≥ 30 s | — |

No human-verifiable PIE result has been received for this campaign. Do **not** treat A04’s automated self-test as execution of C-A04-VIS.

---

## C. Conclusion (preparation only)

| Item | Status |
|---|---|
| `C-A04-VIS` | **`NOT_DEMONSTRATED`** (unchanged) |
| `C-A04-RVT` | **`NOT_DEMONSTRATED`** (not tested) |
| Procedure error vs material incompatibility | **Not decidable** until §B is filled |
| General “M4 incompatible forever” | **Not claimed** |

**Next action (human):** run §A.5, return observations (V-code + assignment check + optional screenshots). A follow-up documentary mission will update this §B/§C and, only if warranted, `REGISTER.md`.

`Design Gate : validation visuelle uniquement ; aucune intégration technique, modification du code ou décision 08.7 autorisée.`

# Conclusions register

Statuses: `DEMONSTRATED` · `PARTIAL` · `NOT_DEMONSTRATED` · `INCOMPATIBILITY_DEMONSTRATED` · `INVALIDATED`

Sources are historical audits + `Results/` evidence. **No new technical verdict is invented here.**

| Id | Topic | Status | Evidence / report | Notes |
|---|---|---|---|---|
| C-A01-RL | Raise/Lower visual HoleFix | `DEMONSTRATED` (manual user) | A01 · `Results/DEMO_TERRADYNE_M4.md` | Single-chunk demo |
| C-A01-COL | Collision after Lower after debounce | `DEMONSTRATED` (manual user) / intra-debounce `NOT_DEMONSTRATED` | A01 · DEMO | |
| C-A01-PAINT | Overlay paint HoleFix | `DEMONSTRATED` (manual) | A01 · DEMO | ≠ M4 paint layers |
| C-A01-SAVE | SaveWorld/LoadWorld host | `NOT_DEMONSTRATED` | A01 | API present, host not wired |
| C-A01-PERF | RTS island perf | `NOT_DEMONSTRATED` | A01 | |
| C-A02-AUTH | Option A (TD as height authority) | `INCOMPATIBILITY_DEMONSTRATED` (contractual vs GF texts) | A02 | Not a runtime impossibility claim |
| C-A02-B | Option B recommendation | Historical recommendation | A02 · synthesis | **≠** adopted 08.7 |
| C-A02-ADAPT | Store→`LoadFromData` adapter | `NOT_DEMONSTRATED` | A02 | Absent |
| C-A03-CONV | Landscape→chunks editor, 4 layers | `DEMONSTRATED` (code inspection) | A03 | Not exercised on GF island here |
| C-A03-ADOPT | Generic Landscape-expr adoption refuse | `INCOMPATIBILITY_DEMONSTRATED` (mechanism) | A03 | Option default off |
| C-A03-M4GRAPH | Master ∈ refuse set | Was `NOT_DEMONSTRATED` in A03 | A03 | **Clarified by A04** |
| C-A03-D | Campaign MIC incomplete → D | `INCOMPATIBILITY_DEMONSTRATED` (incomplete install path) | A03 · DEMO §8 | Distinct from graph listing |
| C-A04-LOAD | Real MIC+master+functions load | `DEMONSTRATED` | A04 · `m4_compat_selftest.log` | nullrhi SLOT0 null |
| C-A04-NODES | Landscape exprs on master | `INCOMPATIBILITY_DEMONSTRATED` vs DynamicMesh adoption path | A04 | 25 direct exprs listed |
| C-A04-PARAMS | HeightMap/WeightMap undeclared on M4 | `OBSERVED` / bus mismatch | A04 | |
| C-A04-VIS | Visual M4 on chunk | `PARTIAL` | A04 · [`CAMPAIGN_C-A04-VIS_2026-10-12.md`](../01_Audits/AUDIT-04/CAMPAIGN_C-A04-VIS_2026-10-12.md) | PIE GPU: MIC Element 0 assigned; visual change (enlarged grid) + ≥30 s stable; expected M4 textures/automaterial **not** identified; grid cause **indeterminate** |
| C-A04-RVT | RVT on DynamicMesh | `NOT_DEMONSTRATED` | A04 · A03 | |
| C-PUB-M4ASSET | Publish `Content/M4` | Human decision open | PUBLICATION_CONTROL | Still local-only |

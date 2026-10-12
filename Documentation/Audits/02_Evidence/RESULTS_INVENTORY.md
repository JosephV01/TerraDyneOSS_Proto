# Inventory — `Results/` (classification)

Files remain in `Results/` unless noted. Categories by content.

## Reports / campaign notes (keep in Results; linked from audits)

| File | Category | Subject |
|---|---|---|
| `DEMO_TERRADYNE_M4.md` | Campaign report | HoleFix + M4 D |
| `M4_NODE_COMPAT.md` | Host note | Real MIC path / self-test |
| `ADAPTATIONS.md` | Technical note | Isolated GPU stub |
| `PHASE1_*.md`, `PHASE1A_*`, `PHASE1B_*`, `PHASE1C_*` | Phase reports | Compile/load/host/raise-lower |
| `PHASE2_TEST_PLAN.md`, `PHASE2B_PRECISION_RESULTS.md` | Test plan / results | Precision |
| `PHASE3A_ARCHITECTURE_COMPARISON.md` | Working architecture note | Pre-A02 |
| `COMPILE.md`, `COMPILE_AFTER_ADAPT.md`, `compile_game_demo.md` | Compile notes | Build |
| `REPORT.md` | Stub / misc | Not an audit REPORT |
| `ENV_PREFLIGHT.txt`, `TERRADYNE_COMMIT.txt` | Baseline stamps | Versions |

## Raw logs / UAT / cook (evidence; not reports)

`*.log`, `Cook-*.txt`, `Log.txt`, `*UAT*`, `UnrealPak*`, `Manifest_*`, `FinalCopy*`, `PrePak*` (dupes removed), `UBA-*`, `IoStoreCommands.txt`, `PakCommands.txt`

## Scripts

`*.bat`, `symbolicate_shipping.py`

## Captures

`shipping_*_play.png`, `shipping_play_shot1.png`

## Crash / JSON (often untracked until explicit add)

`shipping_crash_*.xml`, `Log.json`, `*.json` under Results

## Action policy

- **Conserve** all unique evidence in `Results/`.
- **Reference** from `02_Evidence/INDEX.md` — do not mass-copy into Documentation.
- **Do not** promote logs to audit REPORT status.

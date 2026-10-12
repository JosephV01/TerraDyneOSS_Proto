# Evidence index — pointers into `Results/`

Evidence stays under [`../../../Results/`](../../../Results/) unless noted. This index does **not** duplicate large logs.

## Campaign / reliability (AUDIT-01)

| Evidence | Path | Role |
|---|---|---|
| Campaign report | [`Results/DEMO_TERRADYNE_M4.md`](../../../Results/DEMO_TERRADYNE_M4.md) | HoleFix validations; M4 campaign D |
| Commit stamp | [`Results/TERRADYNE_COMMIT.txt`](../../../Results/TERRADYNE_COMMIT.txt) | Plugin `270c311` |
| Env preflight | [`Results/ENV_PREFLIGHT.txt`](../../../Results/ENV_PREFLIGHT.txt) | Clone / UE versions |
| Lower API | [`Results/lower_selftest.txt`](../../../Results/lower_selftest.txt) | Canonical (exact twin removed — see DEDUPE) |
| Shipping shots | `Results/shipping_holefix_play.png`, `shipping_fix_play.png`, `shipping_play_shot1.png` | Visual evidence |
| Adaptations | [`Results/ADAPTATIONS.md`](../../../Results/ADAPTATIONS.md) | GPU stub / isolated patches |
| Compile notes | `Results/COMPILE.md`, `COMPILE_AFTER_ADAPT.md`, `PHASE1*.md` | Build history |

## Architecture (AUDIT-02)

| Evidence | Path | Role |
|---|---|---|
| Proto PHASE3A note | [`Results/PHASE3A_ARCHITECTURE_COMPARISON.md`](../../../Results/PHASE3A_ARCHITECTURE_COMPARISON.md) | Working note; **not** canonical vs A02 REPORT |
| Precision plan/results | `Results/PHASE2_TEST_PLAN.md`, `PHASE2B_PRECISION_RESULTS.md` | Measurement campaigns |

## M4 / materials (AUDIT-03 / AUDIT-04)

| Evidence | Path | Role |
|---|---|---|
| Host M4 note | [`Results/M4_NODE_COMPAT.md`](../../../Results/M4_NODE_COMPAT.md) | Path fix summary |
| Self-test | [`Results/m4_compat_selftest.log`](../../../Results/m4_compat_selftest.log) | Asset load + height PASS |
| Findings / tests (imported) | [`FINDINGS.md`](FINDINGS.md), [`TESTS.md`](TESTS.md) | Cross-audit |

## Packaging / crash (supporting)

| Evidence | Path | Role |
|---|---|---|
| Crash XML | `Results/shipping_crash_*.xml` | Historical Shipping crash (may be untracked until added) |
| UAT / cook lists | `Results/*UFSFiles*.txt`, `Cook-*.txt` | Cook/pak inventories |

## Not published

- `Packaged/Win64ShippingHoleFix/...` — binaries (PUBLICATION_CONTROL)
- `Content/M4/` — commercial (PUBLICATION_CONTROL)
- `Saved/` — credentials / local

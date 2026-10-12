# Exact duplicate removals (2026-10-12)

Verified with SHA-256. Canonical file kept; redundant tracked copy removed from Git (and deleted from working tree only when identical).

| Canonical kept | Exact duplicate removed | SHA-256 prefix | Reason |
|---|---|---|---|
| `Results/FinalCopyWin64_NonUFSFiles.txt` | `Results/PrePak_Windows_NonUFSFiles.txt` | `379D6F1BAADE` | Identical NonUFS file list |
| `Results/FinalCopyWin64_NonUFSFilesDebug.txt` | `Results/PrePak_Windows_NonUFSFilesDebug.txt` | `4B0686D7C9B5` | Identical debug NonUFS list |
| `Results/lower_selftest.txt` | `Results/lower_selftest_shipping.txt` | `C6824ACA68ED` | Identical LowerSelfTest output |

**Not removed (semantic / historical, not exact dupes):**

| Pair | Why kept |
|---|---|
| `COMPILE.md` vs `COMPILE_AFTER_ADAPT.md` | Different stages |
| `demo_paint_selftest.log` vs `demo_paint_selftest2.log` | Sequential runs |
| `phase1c_raise_lower.log` vs `…_premature_lower.log` | Different runs |
| `DEMO_TERRADYNE_M4.md` vs AUDIT reports | Campaign vs formal audits |
| `PHASE3A_ARCHITECTURE_COMPARISON.md` vs AUDIT-02 | Working note vs canonical |
| `M4_NODE_COMPAT.md` vs AUDIT-04 | Short host note vs full audit |
| `Results/REPORT.md` | Tiny stub; kept as historical label (not an audit) |

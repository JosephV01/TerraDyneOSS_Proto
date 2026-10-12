# Provenance of imported historical documents

**Import date:** 2026-10-12  
**Importer host:** `TerraDyneOSS_Proto_270c311`  
**GardenFervor:** read-only source — **not modified** by this import.

## Online vs local

Attempted public URLs under `https://github.com/JosephV01/GardenFervor/main/ExternalTechnologyAudits/01_TerraDyne/...` returned **HTTP 404** on 2026-10-12 (files not on `main` remote).

Imports were taken from the **local GardenFervor working tree**:

`C:\Users\sahel\Documents\Unreal Projects\GardenFervor\ExternalTechnologyAudits\01_TerraDyne\`

SHA-256 of each imported file matches the local GF source byte-for-byte at import time (verified).

| Destination in this repo | Local GF source | SHA-256 (prefix) |
|---|---|---|
| `01_Audits/AUDIT-01/REPORT.md` | `01_Audits/AUDIT-01/REPORT.md` | `ED6CD8F0B4A48380…` |
| `01_Audits/AUDIT-02/REPORT.md` | `01_Audits/AUDIT-02/REPORT.md` | `F33BA2B380E62246…` |
| `01_Audits/AUDIT-03/REPORT.md` | `01_Audits/AUDIT-03/REPORT.md` | `0ADD2DD9858A4F1D…` |
| `01_Audits/AUDIT-04/REPORT.md` | `01_Audits/AUDIT-04/REPORT.md` | `74DC7E88EF7199CB…` |
| `03_Syntheses/FINAL_SYNTHESIS.md` | `06_Final_Synthesis/FINAL_SYNTHESIS.md` | `E02CC22DD94261C2…` |
| `02_Evidence/FINDINGS.md` | `03_Technical_Findings/FINDINGS.md` | *(copied)* |
| `02_Evidence/TESTS.md` | `05_Follow_Up_Tests/TESTS.md` | *(copied)* |
| `00_Index_And_Methodology/SCOPE.md` | `00_Scope_And_Baseline/SCOPE.md` | *(copied)* |

## Integrity rules applied

- Report bodies were **not** rewritten.
- Companion `README.md` files in each `AUDIT-NN/` are **new metadata wrappers** for this repo; GF originals are stored as `README_IMPORTED.md`.
- Relative links inside imported reports that pointed at GF paths may be stale; see each audit’s `LINKS.md`.

# Methodology — TerraDyne proto audits

## 1. Identity of an audit

Each audit document must declare:

| Field | Requirement |
|---|---|
| Stable id | e.g. `EXT-TERRADYNE-A04` |
| Objective | One sentence |
| Scope / out-of-scope | Explicit |
| Date | ISO `YYYY-MM-DD` |
| Environment | UE version, plugin version/commit, host map/build |
| Status | Document status ≠ technical verdict |

## 2. Separation inside a report

Reports must keep these layers distinct:

1. **Observed facts** — paths, versions, log lines, UI states  
2. **Tests executed** — procedure, command, build  
3. **Results** — pass/fail/measurements  
4. **Interpretation** — what the results mean  
5. **Remaining hypotheses** — untested ideas  

Do not promote an interpretation to a demonstration without citing evidence.

## 3. Evidence

- Important conclusions **must** reference evidence (relative paths under this repo, or immutable external URLs).
- Prefer keeping large binaries out of Git; store a pointer + hash/size/date in `02_Evidence/` when needed.
- Environment limits (nullrhi, Shipping vs Development, single chunk, GPU stub) must be stated next to results.

## 4. Historical integrity

- Do **not** silently rewrite a published report so a later synthesis looks consistent.
- Corrections: append a dated note or add `CORRECTION.md` / changelog entry; leave original text readable.
- Syntheses link sources with **relative** repo paths.

## 5. Validation ceiling

No document may claim a higher validation level than available evidence (e.g. no “100% M4 compatible” from a load-only test).

## 6. Relation to GardenFervor

This methodology applies to the **isolated proto**. GardenFervor contracts and Design Gate are external context only; they are not amended here.

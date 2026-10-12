# TerraDyne OSS Proto — Audit documentation repository

**Role.** Single entry point for audits, investigations, test evidence, syntheses, and conclusion status for the isolated demonstration host:

`TerraDyneOSS_Proto_270c311`

Public remote: [JosephV01/TerraDyneOSS_Proto](https://github.com/JosephV01/TerraDyneOSS_Proto)

This tree does **not** replace Unreal assets or the TerraDyne plugin sources.

**Out of scope.** GardenFervor contracts / Design Gate / gameplay. GF was a **read-only** provenance source for historical reports (see [`PROVENANCE.md`](PROVENANCE.md)).

## Layout

| Path | Purpose |
|---|---|
| [`00_Index_And_Methodology/`](00_Index_And_Methodology/) | Conventions, scope, methodology |
| [`01_Audits/`](01_Audits/) | AUDIT-01 … AUDIT-04 canonical reports |
| [`02_Evidence/`](02_Evidence/) | Indexes into `Results/`, findings, tests, dedupe log |
| [`03_Syntheses/`](03_Syntheses/) | Cross-audit synthesis (1–3) |
| [`04_Conclusions_Register/`](04_Conclusions_Register/) | Living conclusions register |
| [`05_Documentary_History/`](05_Documentary_History/) | Publication history + dated mission reports |
| [`PUBLICATION_CONTROL.md`](PUBLICATION_CONTROL.md) | Public push exclusions |
| [`PROVENANCE.md`](PROVENANCE.md) | Import provenance (GF local → this repo) |

Campaign notes remain under [`../../Results/`](../../Results/) and are **classified / linked**, not mass-relocated.

## Canonical entry points

| Document | URL path |
|---|---|
| AUDIT-01 | [`01_Audits/AUDIT-01/REPORT.md`](01_Audits/AUDIT-01/REPORT.md) |
| AUDIT-02 | [`01_Audits/AUDIT-02/REPORT.md`](01_Audits/AUDIT-02/REPORT.md) |
| AUDIT-03 | [`01_Audits/AUDIT-03/REPORT.md`](01_Audits/AUDIT-03/REPORT.md) |
| AUDIT-04 | [`01_Audits/AUDIT-04/REPORT.md`](01_Audits/AUDIT-04/REPORT.md) |
| Synthesis 1–3 | [`03_Syntheses/FINAL_SYNTHESIS.md`](03_Syntheses/FINAL_SYNTHESIS.md) |
| Conclusions | [`04_Conclusions_Register/REGISTER.md`](04_Conclusions_Register/REGISTER.md) |

## Naming & proof statuses

See [`00_Index_And_Methodology/METHODOLOGY.md`](00_Index_And_Methodology/METHODOLOGY.md).

## Publication rules

1. Update files under `Documentation/Audits/` (+ evidence as needed).  
2. Dedicated documentary commit.  
3. Push to `origin/main` (no force-push).  
4. Record **substantive** publication events in [`05_Documentary_History/HISTORY.md`](05_Documentary_History/HISTORY.md) (corpus / material public content).  
5. **Do not** add a HISTORY table row for commits that only edit `HISTORY.md` (HISTORY maintenance) — that creates an infinite self-reference loop. Recover those commits with `git log -- Documentation/Audits/05_Documentary_History/HISTORY.md`. See the inclusion rule in `HISTORY.md`.

## Design Gate note

Documentary consolidation of the TerraDyne prototype host only. No prototype design/tech change. GardenFervor unmodified.

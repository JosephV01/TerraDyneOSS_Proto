# TerraDyne OSS Proto — Audit documentation repository

**Role.** Single entry point for audits, investigations, test evidence, syntheses, and conclusion status for the isolated demonstration host:

`TerraDyneOSS_Proto_270c311`

This tree is meant to be published on the project’s **public** GitHub repository so reports can be read online with stable URLs. It does **not** replace Unreal assets or the TerraDyne plugin sources.

**Out of scope.** GardenFervor (contracts, Design Gate, gameplay). Do not copy GF authority decisions into this tree as if they were TerraDyne proto facts.

## Layout

| Path | Purpose |
|---|---|
| [`00_Index_And_Methodology/`](00_Index_And_Methodology/) | Conventions, proof statuses, publication rules |
| [`01_Audits/`](01_Audits/) | One folder per audit (`AUDIT-NN/`) |
| [`02_Evidence/`](02_Evidence/) | Pointers / curated evidence (prefer links to `Results/` when already in-repo) |
| [`03_Syntheses/`](03_Syntheses/) | Cross-audit syntheses with relative links to sources |
| [`04_Conclusions_Register/`](04_Conclusions_Register/) | Living register of demonstrated / partial / undemonstrated / invalidated conclusions |
| [`05_Documentary_History/`](05_Documentary_History/) | Changelog of this documentation repository |
| [`PUBLICATION_CONTROL.md`](PUBLICATION_CONTROL.md) | What may / may not be pushed to a public remote |

Historical campaign notes already live under repo-root [`Results/`](../../Results/). **This mission does not relocate or rewrite them.** Classification and deduplication are a later step.

## Naming

- Audit id: `EXT-TERRADYNE-A01`, `A02`, … (stable) or short `AUDIT-01`
- Folder: `01_Audits/AUDIT-01/`
- Canonical report: `REPORT.md`
- Optional companion: `README.md` (status table only)

## Proof statuses (mandatory vocabulary)

| Status | Meaning |
|---|---|
| `OBSERVED` | Fact seen in assets/logs/UI under stated conditions |
| `EXECUTED_TEST` | A named procedure was run; result recorded |
| `DEMONSTRATED` | Conclusion supported by cited evidence |
| `PARTIAL` | Some aspects demonstrated; others open |
| `NOT_DEMONSTRATED` | Insufficient evidence (≠ compatibility, ≠ incompatibility) |
| `INCOMPATIBILITY_DEMONSTRATED` | Failure reproduced and attributed with evidence |
| `INVALIDATED` | Prior conclusion withdrawn; cite superseding note |

**Absence of evidence is never “compatible” and never “incompatible.”**

## Publication rules (for every future audit mission)

1. Write or update files under `Documentation/Audits/` (and evidence as needed).
2. Commit with an explicit documentary message.
3. Push to the public remote (no force-push).
4. Record the commit hash and **exact public URLs** of the published files in the mission report and in `05_Documentary_History/HISTORY.md`.

See also [`00_Index_And_Methodology/METHODOLOGY.md`](00_Index_And_Methodology/METHODOLOGY.md).

## Design Gate note

Creation of this documentary frame and (when authorized) the public GitHub host only. No prototype gameplay/design change. No GardenFervor modification.

# Documentary history

## Inclusion rule (effective 2026-10-12)

`HISTORY.md` records **substantive** publication events — commits that add or change the documentary corpus (audits, evidence indexes, syntheses, conclusions register, methodology/scope, provenance, publication control, mission reports) or other material public content under this repo’s publication control.

**Excluded from this table** (to avoid an infinite self-reference loop): commits whose **only** change is to update `HISTORY.md` itself (HISTORY maintenance). Those commits remain fully recoverable via:

```text
git log -- Documentation/Audits/05_Documentary_History/HISTORY.md
```

Example of an excluded HISTORY-maintenance commit: [`95b447a`](https://github.com/JosephV01/TerraDyneOSS_Proto/commit/95b447a5f773661d2bc1f611fa1e5398073d7f3b) (added the `690892a` row; not listed below).

Rows already present for past HISTORY-only commits (`e0a3596`, `690892a`) are **retained** as historical record and are **not** rewritten. Going forward, new HISTORY-only maintenance commits are not added to the table.

When a substantive commit is published, a follow-up HISTORY-only commit **may** fill in its hash/URL in this table; that follow-up is excluded by this rule.

| Date | Event | Commit / URL |
|---|---|---|
| 2026-10-12 | Public repo `JosephV01/TerraDyneOSS_Proto` (PUBLIC). First safe commit: docs framework, `.gitignore`, host Source/Config/Demo, Results notes, TerraDyne OSS sources. Excluded: `Content/M4/`, `Saved/`, Packaged/Binaries/Intermediate. | [`a190a84`](https://github.com/JosephV01/TerraDyneOSS_Proto/commit/a190a84d6d49305f2fe851f35758691abd306b2a) · [README](https://github.com/JosephV01/TerraDyneOSS_Proto/blob/main/Documentation/Audits/README.md) |
| 2026-10-12 | Recorded first public commit hash in this HISTORY file (history completeness fix). | [`e0a3596`](https://github.com/JosephV01/TerraDyneOSS_Proto/commit/e0a3596e83a727cb9c57cac1658b939a99c13e5c) — verified: only `HISTORY.md` (+2/−2) |
| 2026-10-12 | Import historical AUDIT-01…04 + FINAL_SYNTHESIS (+ FINDINGS/TESTS/SCOPE) from GardenFervor **local** tree (GitHub GF paths 404). Evidence indexes, conclusions register, Results classification, exact-dupe removal of three identical `Results/` twins. | [`16cc121`](https://github.com/JosephV01/TerraDyneOSS_Proto/commit/16cc121e6c0556e2e3483ea21ddf3388b49ce1a6) |
| 2026-10-12 | Recorded the `16cc121` import hash into this HISTORY file (replaced the post-push placeholder). Diff: only `HISTORY.md` (+1/−1). Message: `docs: record historical-audits import commit in HISTORY`. | [`690892a`](https://github.com/JosephV01/TerraDyneOSS_Proto/commit/690892afba594982bb92b10d770a108f42ca5f86) |
| 2026-10-12 | Documentary closure: verify local↔public baseline; adopt HISTORY inclusion rule (exclude HISTORY-only maintenance); publish mission report. | [`1d3d671`](https://github.com/JosephV01/TerraDyneOSS_Proto/commit/1d3d67138b113389a9bdb9b796dc9f4797017d5a) · [mission report](MISSION_2026-10-12_DOCUMENTARY_CLOSURE.md) |
| 2026-10-12 | C-A04-VIS preparation: PIE protocol for real M4 MIC on DynamicMesh; register status unchanged (`NOT_DEMONSTRATED`); execution `NOT_RUN`. | [`1e17b5f`](https://github.com/JosephV01/TerraDyneOSS_Proto/commit/1e17b5f2e1aea0a3cfa8436e6eca34f5c4ddef41) · [campaign](../01_Audits/AUDIT-04/CAMPAIGN_C-A04-VIS_2026-10-12.md) |
| 2026-10-12 | C-A04-VIS closure: human PIE GPU observations recorded; `C-A04-VIS` → `PARTIAL` (MIC assigned + visual change; expected M4 look not demonstrated). | *(hash after substantive commit — filled by excluded maintenance)* · [campaign](../01_Audits/AUDIT-04/CAMPAIGN_C-A04-VIS_2026-10-12.md) |

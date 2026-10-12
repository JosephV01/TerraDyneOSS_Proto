# Publication control — public GitHub

**Goal.** Allow online reading of audit docs without leaking secrets, build trash, or non-redistributable content.

## Must NOT publish (excluded via `.gitignore`)

| Path / pattern | Reason |
|---|---|
| `Binaries/`, `Intermediate/`, `DerivedDataCache/`, `Build/` | Generated; large; machine-local |
| `Saved/` | Local config, crash clients, cooked Zen store |
| `Saved/Cooked/Windows/ue.projectstore` | Contains **password-type authentication material** for Zen/storage — treat as credential; do not push; rotate if ever exposed |
| `Packaged/` | Multi-GB Shipping archives / paks |
| `Plugins/**/Binaries/`, `Plugins/**/Intermediate/` | Built plugin artefacts |
| `Content/M4/` | Third-party **Magic Map Material** commercial content (~4 GB; individual maps >100 MB GitHub limit). Redistribution on a **public** repo is **not authorized** until human confirms license allows it |

## Candidates for first public commit (after remote exists)

| Path | Why |
|---|---|
| `Documentation/` | Audit framework |
| `.gitignore` | Safety net |
| `Source/` | Host C++ (demo only) |
| `Config/` | Project defaults (reviewed: no secret patterns) |
| `TerraDyneOSS_Proto.uproject` | Project descriptor |
| `Content/Demo/` | Small host demo content (non-M4) |
| `Results/*.md` (+ small text logs if useful) | Campaign / investigation notes already local |
| `Plugins/TerraDyne/Source/`, `Content/`, `*.uplugin`, docs under plugin | OSS plugin **sources** (exclude Binaries/Intermediate) |

## Human decisions still required

1. **Authorize** creating/publicizing `TerraDyneOSS_Proto` under the authenticated GitHub account.  
2. **Authorize or forbid** any future inclusion of `Content/M4/` (license + LFS).  
3. **Rotate** the Zen/`ue.projectstore` credential if it may have been exposed outside this machine.  
4. Confirm whether packaged HoleFix binaries should be linked externally instead of Git.

## GitHub hard limits (relevant)

- Soft warning ~50 MB; hard block **100 MB** per file without LFS.  
- Several `Content/M4/Maps/*.umap` exceed 200 MB — another reason they stay ignored until LFS + license are decided.

# Setup — create the public remote (manual, required once)

`gh` is installed locally but **not authenticated**. Credential Manager knows a GitHub user `JosephV01` for Git, but GitHub CLI needs its own login.

## Exact steps (Windows PowerShell)

```powershell
cd "C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311"

# 1) Authenticate (browser or token)
gh auth login -h github.com -p https -w

# 2) Confirm identity
gh api user --jq .login

# 3) Ensure name is free under that account
gh repo view TerraDyneOSS_Proto

# 4) Create EMPTY public repo (no README / license / gitignore on remote)
gh repo create TerraDyneOSS_Proto --public --description "Isolated TerraDyne demonstration prototype and technical audit documentation." --source=. --remote=origin --push=false

# 5) Init + first safe commit (after reviewing PUBLICATION_CONTROL.md)
git init -b main
git add .gitignore Documentation Source Config TerraDyneOSS_Proto.uproject Content/Demo Results/*.md Plugins/TerraDyne/Source Plugins/TerraDyne/Content Plugins/TerraDyne/*.uplugin Plugins/TerraDyne/*.md 2>$null
git status
git commit -m "docs: initialize public audit documentation framework and safe project sources"
git remote add origin https://github.com/<LOGIN>/TerraDyneOSS_Proto.git
git push -u origin main

# 6) Verify
gh repo view TerraDyneOSS_Proto --web
```

**Do not** push `Content/M4/`, `Saved/`, `Packaged/`, or plugin Binaries/Intermediate.

**Do not** use `gregorik/TerraDyne` as this remote.

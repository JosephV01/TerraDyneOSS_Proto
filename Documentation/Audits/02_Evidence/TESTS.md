# Tests complémentaires — TerraDyne

« Proposé » n’est pas « exécuté ». Seules les lignes marquées **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** ont une confirmation humaine rapportée. Cursor n’a exécuté aucun de ces tests dans le cadre de la création de ce référentiel (2026-10-12).

| Id | Objectif | Procédure (rappel) | Build / version | Résultat | Confirmé par |
|---|---|---|---|---|---|
| FUT-01 | Collision après Lower pendant le délai de MAJ | Creuser, rester sur la zone pendant ~0,2 s+, marcher/tracer | HoleFix | Collision suit le trou après délai | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** |
| FUT-02 | Raise / Lower visuels HoleFix | Raise ; Lower sol intact ; les deux ordres | HoleFix | Visuel correct | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** |
| FUT-03 | Peintures HoleFix | Couches + aller-retour paint ↔ terraform | HoleFix | Peintures fonctionnelles | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** |
| FUT-04 | Ronde-trip SaveWorld/LoadWorld | Brancher l’API au host, sauver, relancer, recharger | Non exécuté | — | **NON EXÉCUTÉ** |
| FUT-05 | Collision pendant la fenêtre debounce (avant 0,2 s) | Tracer immédiatement après Lower | HoleFix ou suivant | — | **NON EXÉCUTÉ** |
| FUT-06 | Budget frame / île RTS | Mesure GPU/CPU à l’échelle cible GF | — | — | **NON EXÉCUTÉ** |
| FUT-07 | Pas 25 cm multi-chunks | Hors clamp 256 d’un chunk unique | — | — | **NON EXÉCUTÉ** |
| FUT-08 | Audit 2 comparaison 08.7 | Inspection code + contrats ; rapport canonique `AUDIT-02/REPORT.md` | OSS `270c311` + GF 2026-10-12 | Recommandation option B ; 08.7 non tranché | **AUDIT DOCUMENTAIRE EXÉCUTÉ** (pas un test runtime) |
| FUT-09 | Audit 3 M4 / versions / conversion | Inspection code + docs M4 officielles + sources développeur ; rapport `AUDIT-03/REPORT.md` | OSS 0.4 `270c311` | **D conservé** ; conversion éditeur 4 couches ; 0.7 hors proto | **AUDIT DOCUMENTAIRE EXÉCUTÉ** (pas un test runtime) |
| A03-V1 | Lister nœuds Landscape de `M4UE5MasterMat` | `GetExpressions()` via `-M4CompatSelfTest` (éditeur -game) | Proto `Content/M4` 2026-10-12 | 25 expr. Landscape directes (LayerBlend, GrassOutput, LayerSample×22, PhysicalMaterialOutput) | **EXÉCUTÉ (A04)** — listing code, pas un visuel Material Editor |
| FUT-10 | Self-test host M4 réel | `-M4CompatSelfTest` ; MIC `Island_UE5Preset5_Inst` | Proto Editor 5.8.3, `-game -nullrhi` | 17/17 assets ; HEIGHT PASS ; SLOT0 null (nullrhi) | **EXÉCUTÉ (automatisé A04)** ; visuel **NON VÉRIFIÉ** |
| A03-V2 | Binding 0.7 M4 avec et sans RVT | Uniquement si une édition 0.7 est un jour disponible ; ne pas l’acquérir depuis cet audit | Commercial annoncé | — | **NON EXÉCUTÉ** (édition absente) |
| A03-V3 | Conversion Authored World sur Landscape de test isolé | Wizard éditeur ; pas le projet GF | OSS 0.4 | — | **NON EXÉCUTÉ** |
| A03-V4 | Spécifier (documenter) un adaptateur Store→`FTerraDyneChunkData` | Conception seulement ; ne pas transférer D1 | GF + OSS 0.4 | — | **NON EXÉCUTÉ** |

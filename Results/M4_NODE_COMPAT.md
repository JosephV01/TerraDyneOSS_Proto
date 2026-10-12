# Host M4 — chargement du graphe réel (2026-10-12)

GardenFervor n’est pas modifié. Plugin TerraDyne non modifié. `Content/M4` graphe non édité.

Rapport canonique : `GardenFervor/ExternalTechnologyAudits/01_TerraDyne/01_Audits/AUDIT-04/REPORT.md`

## Changement host

`TryApplyM4Material` charge `/Game/M4/Materials/PresetInstances/Island/Island_UE5Preset5_Inst` (parent `M4UE5MasterMat`).  
Anciens chemins : `M4_Eval` (orphelin) et `Maps/Island_Demo_sharedassets` (inexistant).

Self-test : `-M4CompatSelfTest` (voir `m4_compat_selftest.log`).

Bouton HUD M4 : même chemin réel. Overlay vertex-color coupé si M4 appliqué.

## Preuve automatique

17/17 assets M4 requis chargés. HEIGHT Raise/Lower PASS.  
25 expressions Landscape listées sur le master. HeightMap/WeightMap absents du graphe M4.  
SLOT0 null sous `-nullrhi` (`SetMaterial` exige un renderer).

Visuel : validation humaine PIE requise.

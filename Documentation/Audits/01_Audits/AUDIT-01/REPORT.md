# AUDIT-01 — Fiabilité du terrain TerraDyne (prototype isolé)

| Champ | Valeur |
|---|---|
| Identifiant | `EXT-TERRADYNE-A01` |
| Date de constitution du rapport canonique | 2026-10-12 |
| Statut documentaire | `TECHNICAL_REVIEW_COMPLETE` |
| Verdict technique | **Favorable sous réserves** |
| Intégration GardenFervor | **Non autorisée** |
| Audit 2 | **Non exécuté** |

Ce rapport est le **document d’autorité** de l’audit 1 dans GardenFervor. Il synthétise une revue technique déjà réalisée (lecture de code + journaux proto + confirmations manuelles de l’utilisateur). Il **n’invente aucun test nouveau**. Il n’altère pas les conclusions des sources pour les rendre plus favorables.

## 1. Question et périmètre

**Question :** le prototype TerraDyne isolé tient-il, sur son build final, une hauteur éditable, une collision, une peinture, et quels blocages structurels restent pour GardenFervor ?

**Inclus :** hauteur (Raise/Lower), collision, peinture overlay, sauvegarde plugin vs host, résolution, GPU stub, crash Shipping historique.

**Exclus :** comparaison architecturale 08.7 (audit 2), compatibilité M4 approfondie (audit 3), intégration, perf île RTS, écologie.

## 2. Sources et versions

| Source | Rôle | Canonicalité |
|---|---|---|
| Prototype `TerraDyneOSS_Proto_270c311` | Code host + plugin, builds | Originaux **externes** — non copiés, non déplacés |
| `Results/DEMO_TERRADYNE_M4.md` (proto) | Campagne démo, cause du plancher, validations HoleFix | Source de campagne ; **pas** ce rapport |
| Journaux proto (`lower_selftest*.txt`, crash XML, rapports PHASE*) | Preuves de builds antérieurs ou self-tests | Voir `02_Evidence/INDEX.md` |
| Confirmations manuelles de l’utilisateur sur HoleFix | Raise, Lower, peintures, collision après délai | VALIDÉ MANUELLEMENT PAR L’UTILISATEUR |

Build final déclaré :  
`Packaged/Win64ShippingHoleFix/TerraDyneOSS_Proto.exe`

## 3. Faits établis (niveaux de preuve)

### 3.1 Crash graphique Shipping (historique)

Le premier package Shipping s’est arrêté sur un accès nul `GetDynamicMeshElements` lié au mode ColorOverride / matériau debug non cuit. Correctif host : override `None` + cook du matériau vertex color.  
**Niveau :** VALIDÉ — build antérieur (`Win64Shipping` témoin ; `Win64ShippingFixed` boot GPU).  
**Ne pas** présenter ce crash comme l’état du build HoleFix.

### 3.2 Creusement visuel

Un self-test API Lower a pu passer (`Win64ShippingLowerFix`) sans que l’utilisateur voie un trou : le plancher du template (`Floor_0` / `SM_Template_Map_Floor`, Z=0) masquait le mesh. Le clamp HeightBuffer `[-1,1]` n’est **pas** la cause racine du creusement invisible. HoleFix masque ce plancher.  
**Niveau :** VALIDÉ — campagne démo + confirmation utilisateur HoleFix.

### 3.3 Raise / Lower / peintures (build final)

L’utilisateur a confirmé manuellement, sur HoleFix : Raise, Lower (sol intact et ordres combinés), peintures, et aller-retour paint ↔ terraform.  
**Niveau :** VALIDÉ MANUELLEMENT PAR L’UTILISATEUR — build final.  
Ce n’est **pas** un test exécuté par Cursor.

### 3.4 Collision après creusement

L’utilisateur a confirmé manuellement que la collision suit le creusement **après le délai de mise à jour** (debounce documenté à 0,2 s dans le plugin, cook asynchrone, au plus une collision par frame).  
**Niveau :** VALIDÉ MANUELLEMENT PAR L’UTILISATEUR — build final.  
Cursor n’a pas reproduit ce scénario par un test automatisé sur HoleFix. La collision LOD lointaine (`NoCollision` au-delà du seuil inspecté) n’est pas une preuve RTS.

Inspection code (non runtime HoleFix) : `UpdateCollision()` → `DynamicMeshComp->UpdateCollision(true)` après apply mesh.

### 3.5 Sauvegarde persistante

`SaveWorld` / `LoadWorld` existent dans le plugin (SaveGame v2 + cache de chunks). Le host de démo **ne les appelle pas**. Aucune ronde-trip save/load n’est rapportée.  
**Niveau :** INSPECTÉ DANS LE CODE ; persist **NON VALIDÉ**.

### 3.6 Résolution / île ~25 cm

Chunk démo : taille 13652,5 / 128 steps ≈ **107,5 UU** (~107,5 cm si 1 UU = 1 cm). `InitializeChunk` clamp la résolution **32–256**. Une île entière à 25 cm de pas n’est **pas** démontrée et est structurellement limitée par ce clamp sur un seul chunk.  
**Niveau :** INSPECTÉ DANS LE CODE. Objectif 25 cm île : **NON VALIDÉ**.

### 3.7 GPU / perf RTS

`bEnableGPUBrushes=false` ; chemin GPU stub. Aucune mesure de budget frame à l’échelle île GardenFervor.  
**Niveau :** INSPECTÉ DANS LE CODE ; perf RTS **NON VALIDÉ**.

### 3.8 Peinture — double autorité

PaintStore (11 couches côté host GF) vs `WeightBuffers[4]` plugin. Overlay couleurs primaires historiquement effacé par `Mesh.Clear`. Peinture **jouable** sur HoleFix (utilisateur) ≠ peinture comme autorité GardenFervor / couches Landscape M4.  
**Niveau :** visuel HoleFix VALIDÉ MANUELLEMENT ; modèle d’autorité **NON VALIDÉ** pour GF.

### 3.9 M4

Verdict de campagne **D**. Les peintures TerraDyne ne sont pas des couches Landscape M4.  
**Niveau :** constat de campagne ; audit 3 **non exécuté**. Reste **D**.

### 3.10 Écologie

Aucune simulation écologique avancée n’a été exercée. **NON VALIDÉ.**

## 4. Verdict

**Favorable sous réserves** pour un **prototype isolé** de terraformation visuelle + collision après délai, sur le build HoleFix, aux échelles et modes testés.

Réserves bloquantes vis-à-vis d’une adoption GardenFervor :

1. Pas d’autorité de hauteur GardenFervor sans audit 2 / décision 08.7.
2. Persist host **non branchée**, **non validée**.
3. Pas de preuve de pas 25 cm à l’échelle île (clamp 256).
4. GPU stub ; perf RTS inconnue.
5. M4 = **D**.
6. Écologie non évaluée.

Ce verdict n’est **pas** une validation Design Gate, pas un PASS F1, et **n’autorise pas** l’intégration.

## 5. Vérifications restantes

Voir `05_Follow_Up_Tests/TESTS.md`. Aucun test listé là-bas n’est déclaré exécuté par Cursor. Les items déjà confirmés par l’utilisateur sont marqués comme tels.

## 6. Modifications hors périmètre

Lors de la constitution de ce dossier (2026-10-12) : aucun code GardenFervor, aucun fichier proto, aucun contrat, aucun statut Design Gate n’a été modifié pour cet audit.

# Synthèse finale — Évaluation TerraDyne pour GardenFervor

| Champ | Valeur |
|---|---|
| Identifiant | `EXT-TERRADYNE-SYN-01` |
| Date | **2026-10-12** |
| Nature | Synthèse transverse des audits 1–3 — **pas** un quatrième audit |
| Statut | Documentaire. **Aucune décision d’architecture. Aucune intégration.** |
| Prototype | `C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311` |
| Plugin | TerraDyne OSS **0.4**, commit **`270c31117e20e491ef5333dd7e2f9dc133fcdc38`** |

**Design Gate : aucun contrat ni statut modifié — synthèse documentaire des audits 1 à 3.**

Les trois rapports canoniques restent les sources détaillées :

- [`../01_Audits/AUDIT-01/REPORT.md`](../01_Audits/AUDIT-01/REPORT.md) — fiabilité
- [`../01_Audits/AUDIT-02/REPORT.md`](../01_Audits/AUDIT-02/REPORT.md) — architecture
- [`../01_Audits/AUDIT-03/REPORT.md`](../01_Audits/AUDIT-03/REPORT.md) — versions, conversion Landscape, M4

Cette synthèse **ne les remplace pas**, **ne les réécrit pas** et **n’autorise pas** une intégration.

---

## A. Résumé exécutif

Les trois audits établissent un **prototype isolé** TerraDyne OSS 0.4 capable, sur le build HoleFix, de Raise / Lower visuels, de peintures d’overlay et d’une collision qui suit le creusement **après** le délai de mise à jour. Ces confirmations sont des **validations manuelles de l’utilisateur**, pas des tests Cursor. Le plugin sait, **en éditeur**, convertir un Landscape Unreal chargé vers des chunks, avec **quatre** couches peintes. GardenFervor, de son côté, a déjà des contrats **VALIDÉ** qui nomment `RuntimeTerraformStore` comme unique autorité de hauteur après terraformation.

Ils ne permettent **pas** de conclure à une intégration. La persistance n’est pas câblée au host. Les performances RTS, le pas 25 cm à l’échelle île, les opérations concurrentes multi-unités, les sols, l’eau et les écosystèmes restent **NON VALIDÉ**. L’adaptateur `RuntimeTerraformStore` → `FTerraDyneChunkData` **n’existe pas**. M4 n’est **pas** démontré sur le DynamicMesh TerraDyne : le verdict de campagne **D** est conservé, avec une portée **restreinte** (essai MIC incomplet ; refus d’adoption **générique** ; graphe exact de `M4UE5MasterMat` **non listé**).

La recommandation d’architecture actuelle, issue d’AUDIT-02, est l’**option B** : GardenFervor conserve l’autorité du terrain. L’option A (TerraDyne comme autorité) est une **incompatibilité contractuelle** avec C-01 D1, 08.6, 00.5.S4, 00.6.A2 et C-02 F1 — pas une preuve que le plugin est inopérant. **08.7 reste `À DÉFINIR`.** Cette recommandation n’est **pas** une décision adoptée.

Une intégration n’est pas justifiée : les preuves de jouabilité portent sur **un chunk** de démo ; elles n’établissent ni l’autorité GardenFervor, ni G5 à l’échelle île, ni M4, ni C-19, ni une architecture 08.7. Transférer l’autorité pour « faciliter » TerraDyne violerait des contrats déjà VALIDÉ.

---

## B. Matrice des résultats consolidés

Niveaux : `DEMONTRÉ_MANUELLEMENT` (utilisateur) · `INSPECTÉ_DANS_LE_CODE` · `DOCUMENTÉ_OFFICIELLEMENT` · `ANNONCÉ_PAR_LE_DÉVELOPPEUR` · `NON VALIDÉ` / `INCONNU`.

| Domaine | Résultat consolidé | Niveau de preuve | Risque restant | Rapport source |
|---|---|---|---|---|
| Fiabilité Raise/Lower | Visuels HoleFix confirmés (y compris Lower sur sol intact et les deux ordres). Cause historique du creux invisible = plancher template, pas le clamp HeightBuffer. | `DEMONTRÉ_MANUELLEMENT` (utilisateur, HoleFix) ; cause plancher = campagne + utilisateur | Un chunk démo ; pas l’île GF ; GPU stub | A01 §3.2–3.3 ; TF-01 ; TF-02 |
| Collision | Suit le creusement **après** le délai (~0,2 s, cook async, 1 collision/frame). Collision **pendant** la fenêtre debounce : non testée. LOD lointain sans collision (code). | Après délai : `DEMONTRÉ_MANUELLEMENT`. Intra-debounce : `NON VALIDÉ` (FUT-05). | Unités RTS dans la fenêtre 0,2 s ; navmesh ; traces Landscape GF | A01 §3.4 ; TF-03 ; TESTS FUT-01 / FUT-05 |
| Peintures | Overlay jouable HoleFix (couleurs de remplacement). PaintStore host vs 4 `WeightBuffers` plugin. ≠ couches Landscape M4. | Visuel : `DEMONTRÉ_MANUELLEMENT`. Autorité GF/M4 : `NON VALIDÉ` | Double autorité peinture si mesh TD + Store | A01 §3.8 ; TF-07 ; A03 campagne D |
| Autorité des hauteurs | Contrats : `RuntimeTerraformStore`. Plugin : `HeightBuffer` (Base+Sculpt+Detail) + `ApplyGlobalBrush` void. Option B recommandée. | Contrats `DOCUMENTÉ` VALIDÉ ; plugin `INSPECTÉ_DANS_LE_CODE` | Option A = double vérité | A02 ; TF-09 ; C-01 D1 ; 08.6 |
| Opérations concurrentes | Pas de réservation sommet côté TerraDyne ; edits hors chunks actifs ignorées. Store GF additif game-thread. Multi-unités RTS non exercé des deux côtés. | `INSPECTÉ_DANS_LE_CODE` ; runtime `INCONNU` | Travaux autonomes hors streaming | A02 §7 |
| Conversion Landscape | Présente dans l’OSS 0.4 **éditeur** (`ImportFromLandscapeWithOptions`, baker). 4 couches ; composants **chargés**. ≠ conversion Store. Non exercée sur l’île GF. 0.7 (8 couches + binding) = annoncé, hors proto. | Code : `INSPECTÉ_DANS_LE_CODE`. Runtime conversion : `NON VALIDÉ`. 0.7 : `ANNONCÉ_PAR_LE_DÉVELOPPEUR` | World Partition / cellules non chargées ; confusion Store ↔ Landscape | A03 §5 ; TF-13 ; TF-14 |
| Matériaux M4 | Campagne **D** (MIC incomplet, `M4_Eval`). Scan d’adoption OSS 0.4 : refus **générique** de 7 expressions Landscape, option défaut **off**. Graphes `M4UE5MasterMat` vs ce scan : **non établi**. Dossier `Content/M4` désormais présent dans le proto : **présence seulement**, non testé. | D = campagne. Scan = `INSPECTÉ_DANS_LE_CODE`. Graphe M4 = `NON_DÉMONTRÉE_PREUVES_INSUFFISANTES`. `Content/M4` = inventaire 2026-10-12 | Confondre présence d’assets et compatibilité ; RVT même en 0.7 annoncé « not bound » | A03 (portée restreinte) ; §D ci-dessous |
| Persistance | `SaveWorld` / `LoadWorld` plugin (SaveGame v2) **non appelés** par le host. Store GF : flag persist OFF, politique C-19. Aucun round-trip VALIDÉ. | `INSPECTÉ_DANS_LE_CODE` ; runtime `NON VALIDÉ` | Deux formats distincts | A01 §3.5 ; TF-04 ; A02 ; FUT-04 |
| Performances RTS | `bEnableGPUBrushes=false`. Aucune mesure île. Clamp chunk 32–256 ; pas démo ≈ 107,5 UU ≠ pas Landscape GF. | `INSPECTÉ_DANS_LE_CODE` ; perf `NON VALIDÉ` | Budget frame ; multi-chunks ; 25 cm | A01 §3.6–3.7 ; TF-05 ; TF-06 ; TF-11 |
| Sols, eau et écosystèmes | Non exercés sur le proto. Contrats C-16 / C-15 / C-17 = conception. Poids TerraDyne ≠ sol. | `NON VALIDÉ` runtime | Prendre les 4 poids pour du C-16 | A01 §3.10 ; A02 §7 |
| Compatibilité contractuelle | Option A incompatible avec C-01 D1, D2, 08.6, 00.5.S4, 00.6.A2, C-02 F1. Option B alignée sur ces textes. 08.7 `À DÉFINIR`. | `INCOMPATIBILITÉ_DÉMONTRÉE` **contractuelle** (A) ; B = textes VALIDÉ | Recommandation lue comme décision 08.7 | A02 ; TF-09 |

Build final documenté dans A01 / SCOPE :  
`Packaged/Win64ShippingHoleFix/TerraDyneOSS_Proto.exe`  
(chemin relatif à la racine du proto). Le libellé `TerraDyne_OSS_Proto.exe` n’apparaît **pas** dans les rapports canoniques.

---

## C. Recommandation d’architecture

**GardenFervor conserve l’autorité du terrain**, représentée par `RuntimeTerraformStore`, conformément aux contrats applicables (C-01 D1 ; 08.6 VALIDÉ ; 00.5.S4 ; 00.6.A2 ; C-02 F1).

Cette orientation, recommandée par AUDIT-02, préserve :

- une seule vérité de hauteur pour le gameplay, distincte de la représentation (00.6.A2) ;
- la frontière C-01 / C-02 (le substrat spatial ne possède jamais la hauteur) ;
- la possibilité future des sols, de l’eau et des écosystèmes (C-16, C-15, C-17) sans les accrocher aux poids ou au `HeightBuffer` TerraDyne ;
- la politique de persistance C-19 sur des données dont C-01 est propriétaire, plutôt que sur un SaveGame plugin non câblé ;
- 08.7 comme choix de **présentation** (mesh, collision, rendu), encore ouvert.

TerraDyne OSS, dans cette orientation, n’est **pas** l’autorité. Un rôle éventuel de maillage / collision resterait une **candidate 08.7 non démontrée**, conditionnée à un adaptateur Store → `LoadFromData` **sans** `ApplyGlobalBrush`. Cet adaptateur **n’existe pas** (TF-10).

**Ceci n’est pas une décision 08.7.** DG-08.7 reste `À DÉFINIR` (`designGate.js`, lecture 2026-10-12). `04_Decision_Records/` est vide.

---

## D. Évaluation M4

### 1. Démontré pour TerraDyne OSS 0.4

- Rendu chunk = `UDynamicMeshComponent` ; `SetMaterial` crée un MID et pousse `HeightMap` / `WeightMap` (4 canaux). **Pas** de scan Landscape sur ce chemin.
- Adoption à la conversion : `UsesLandscapeOnlyMaterialExpressions` refuse sept types d’expressions Landscape **si** `bAdoptLandscapeMaterialAsMasterMaterial` est true (défaut **false**). Mécanisme **générique**, éditeur seulement.
- Campagne proto : `Content/Demo/M4_Eval/` = MIC + LayerInfos, **sans** master / plugin / textures ; soft-refs `/Game/M4/...` cassées → verdict **D** (`DEMO_TERRADYNE_M4.md` §8).
- Doc M4 officielle : peinture = Landscape Paint Layers ; chemin Voxel dédié ; **pas** d’équivalent TerraDyne.
- Éditions distinctes : OSS **0.4** / manuel **0.4.1** / forum **0.7.0** (8 couches + binding, RVT « not bound »). Le proto **n’est pas** 0.7.

### 2. Inconnu sur le matériau M4 exact

- Le graphe vivant de `M4UE5MasterMat` n’a **pas** été listé en éditeur (A03-V1 ouvert).
- Les noms sérialisés (`LandscapeLayerBlend`, `LayerSample`, `GrassOutput`) sont une **indication**, pas une preuve que le scan d’adoption refuserait ce master.
- Automaterial hauteur/pente, 18 couches peintes, RVT, trails : **non démontrés** sur DynamicMesh 0.4.
- Adaptation contrôlée : **non démontrée** ; pas un verdict positif.

### 3. Changement de contexte postérieur à AUDIT-03 — `Content/M4`

Inventaire **lecture seule**, 2026-10-12, **sans** éditeur ni test :

| Constat | Détail |
|---|---|
| Chemin | `TerraDyneOSS_Proto_270c311/Content/M4/` |
| Premier niveau | `Blueprints`, `Extras`, `Foliage`, `Maps`, `Materials`, `SoundSystem`, `Textures`, `TrailSystem` |
| Masters terrain | `Materials/MasterMaterials/M4UE5MasterMat.uasset`, `M4HeightfieldMasterMat.uasset`, `M4DistantMeshMasterMat.uasset`, `M4MobileMasterMat.uasset` |
| Volume | **1458** fichiers, dont **1450** `.uasset` |
| Campagne A03 | `Content/Demo/M4_Eval/` **toujours présent** (MIC orphelin + README) |

AUDIT-03 décrivait `M4_Eval` comme install **incomplète** et situait l’install réelle sous GardenFervor `Content/GardenFervor/Environment/`. `Content/M4` dans le proto est **postérieur** à cet état de rapport. La distinction chronologique est **conservée**. AUDIT-03 n’est **pas** réécrit.

**La présence du dossier ne prouve ni compilation, ni affichage, ni couches, ni RVT, ni compatibilité avec le scan d’adoption.** Toute évaluation réelle des matériaux est une **phase ultérieure**, non autorisée ici. Cette phase devra chercher la cause dans les assets et le code disponibles, **sans présumer** d’un correctif, d’une insolubilité définitive, ni d’une compatibilité.

---

## E. Risques et inconnues prioritaires

Pas un plan d’implémentation.

| Sujet | État | Suite éventuelle (non lancée) |
|---|---|---|
| Persistance / round-trip save-load | API plugin inspectée, host non câblé ; Store GF persist OFF | Campagne de test **distincte** (FUT-04), pas un nouvel audit documentaire |
| Cohérence données / maillage / collision | HoleFix : visuel + collision **après** délai, un chunk | Test intra-debounce (FUT-05) ; G5 île GF si 08.7 le demande |
| Modifications concurrentes | Code : pas de réservation sommet TD ; edits hors chunks perdues | Campagne RTS / tâches C-04–C-11 **si** un jour une représentation TD est étudiée |
| Échelle RTS, multi-chunks, résolutions | Clamp 256 ; GPU stub ; 107,5 UU = pas **démo**, pas île GF | Mesures (FUT-06, FUT-07) ; ne changent **pas** D1 |
| Conversion Store → représentation TD | Adaptateur **absent** (TF-10) | Spécification documentaire seulement (A03-V4 / A02-V1) ; **sans** transférer D1 |
| M4 exact / adaptations | D campagne ; scan générique ; graphe non listé ; `Content/M4` non testé | Phase matériaux **autorisée séparément** : listing graphe, essai contrôlé, pas d’hypothèse de fix |
| Sols, eau, écosystèmes | Contrats conception ; proto sans modèle C-15/C-16/C-17 | Rester hors TerraDyne comme autorité ; runtime écologique **pas** une preuve TD |

---

## F. Conditions de réévaluation

Aucun de ces travaux n’est lancé par cette synthèse. Une réévaluation ne serait justifiée que par des **preuves nouvelles**, par exemple :

1. Mesures représentatives (opérations cibles, résolution, budget frame) sur une configuration **nommée**, pas sur un seul chunk démo.
2. Round-trip save/load **validé** (plugin et/ou Store GF, formats distingués).
3. Démonstration de synchronisation Store autoritaire → maillage/collision **sans** `ApplyGlobalBrush` (adaptateur réel).
4. Tests **documentés** du matériau M4 **complet** (`Content/M4` ou install GF) sur le prototype, lorsque cette phase sera autorisée — y compris A03-V1 (listing graphe).
5. Preuve d’architecture **compatible** avec les contrats VALIDÉ (08.7 tranché **par l’humain**, pas par un audit).

Des gains de perf ou un dossier M4 plus complet **ne suffisent pas** à légitimer l’option A tant que C-01 D1 nomme le Store. Seul un **amendement explicite** de ces contrats par l’humain pourrait rouvrir l’autorité.

---

## G. Statut des décisions

| Élément | Statut |
|---|---|
| AUDIT-01 / 02 / 03 | Sources détaillées de référence ; **non réécrits** par cette synthèse |
| Recommandation option B | Recommandation d’audit — **≠** décision 08.7 |
| DG-08.7 | **`À DÉFINIR`** (lecture `designGate.js`, 2026-10-12) |
| Contrats VALIDÉ (C-01, C-02, 08.6, …) | **Inchangés** |
| `04_Decision_Records/` | **Aucune** décision enregistrée |
| Intégration TerraDyne | **Non autorisée** |
| `Content/M4` proto | Changement de contexte **non testé** ; n’invalide pas AUDIT-03 |

---

## H. Divergences documentaires constatées

Exposées, **non corrigées** dans les rapports 1–3 (hors périmètre).

1. **AUDIT-01** en-tête : « Audit 2 = Non exécuté » ; §3.9 : « audit 3 non exécuté ». Historique à la rédaction d’A01. Le registre et cette synthèse : A02 et A03 = `TECHNICAL_REVIEW_COMPLETE`.
2. **AUDIT-02** en-tête : « Audit 3 = Non exécuté ». Idem, snapshot A02. A03 a depuis précisé (et restreint) D.
3. **`AUDIT-03/README.md`** : libellé encore `INCOMPATIBILITÉ_TECHNIQUE_DÉMONTRÉE` (voie directe OSS 0.4), plus fort que le `REPORT.md` après vérification ciblée. Le **rapport** fait autorité.
4. **FINDINGS TF-08** : une ligne conserve « pas clôture d’audit 3 » alors qu’une autre cite la clôture A03. TF-13 formule l’écart 4 canaux vs paint M4 de façon plus affirmative que la portée restreinte d’A03 sur le graphe.
5. **Binaire** : rapports = `TerraDyneOSS_Proto.exe`. Ne pas substituer `TerraDyne_OSS_Proto.exe`.
6. **Collision « pendant le délai »** : les rapports valident la collision **après** le délai (FUT-01). FUT-05 (avant 0,2 s) = **NON EXÉCUTÉ**.
7. **`Content/M4`** : absent de l’état décrit par AUDIT-03 (`M4_Eval` incomplet). Présent au 2026-10-12. Chronologie à conserver.

---

## I. Périmètre de cette synthèse

**Créé :** ce fichier.

**Mis à jour pour référence :** `ExternalTechnologyAudits/01_TerraDyne/README.md` ; `ExternalTechnologyAudits/README.md`.

**Non modifié :** AUDIT-01 / 02 / 03, FINDINGS, TESTS, contrats, `designGate.js`, proto, code, assets, configuration.

`REGLES_PROJET.md` §1bis demanderait en principe une entrée `ETAT_PROJET.md` / `HISTORIQUE_MODIFICATIONS.md` pour toute modification validée. Ces fichiers **hors référentiel** n’ont **pas** été touchés, conformément au périmètre de mission.

`Design Gate : aucun contrat ni statut modifié — synthèse documentaire des audits 1 à 3.`

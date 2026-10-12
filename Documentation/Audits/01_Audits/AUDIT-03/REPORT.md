# AUDIT-03 — TerraDyne / M4 : versions, conversion Landscape et matériaux

| Champ | Valeur |
|---|---|
| Identifiant | `EXT-TERRADYNE-A03` |
| Date d’inspection | **2026-10-12** |
| Statut documentaire | `TECHNICAL_REVIEW_COMPLETE` |
| Verdict M4 (campagne, conservé) | **D — incompatibilité directe** (`DEMO_TERRADYNE_M4.md` §8 : MIC incomplet sur DynamicMesh) |
| Portée du refus d’adoption OSS 0.4 | **Mécanisme générique démontré** — pas une incompatibilité propre au graphe `M4UE5MasterMat` |
| Intégration | **Non autorisée** |
| 08.7 | **Non modifié** |

**Design Gate : aucun contrat ni statut modifié — audit M4, versions et compatibilité en lecture seule.**

Ce rapport est le **document d’autorité** de l’audit 3. Il ne réécrit pas AUDIT-01 ni AUDIT-02. Le verdict campagne M4 **D** (`DEMO_TERRADYNE_M4.md`) est **conservé**. Une vérification ciblée du **2026-10-12** en restreint la portée technique : le refus d’adoption inspecté dans le code OSS 0.4 est **générique** ; il ne démontre pas, à lui seul, que `M4UE5MasterMat` contient les expressions refusées.

---

## 1. Objet, périmètre, date et versions

**Questions :** quelle édition TerraDyne est réellement dans le proto ; quelles conversions Landscape existent dans *cette* édition ; ce que le manuel / Fab / forum annoncent d’autre ; si M4 peut lier ses couches et effets au rendu TerraDyne actuel ; quelles conditions seraient nécessaires sans transférer l’autorité de hauteur (AUDIT-02, option B).

**Inclus :** lecture seule du proto, du plugin, des assets M4 disponibles, documentation officielle M4, sources développeur TerraDyne.

**Exclus :** compilation, import, achat commercial, tests modifiant un projet, audit d’intégration, décision 08.7.

### Versions de travail (ne pas les fusionner)

| Édition | Identité constatée | Type de preuve |
|---|---|---|
| Prototype local | OSS **0.4** (`Version` 4 / `VersionName` `0.4`), commit enregistré **`270c31117e20e491ef5333dd7e2f9dc133fcdc38`**, `EngineVersion` **local** `5.8.0` | `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` |
| Dépôt OSS public `main` | **Même commit `270c311`** au 2026-10-12 ; `EngineVersion` **`5.7.0`** | `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` (GitHub) |
| Manuel développeur | **TerraDyne Manual v0.4.1** ; Fab 5.5 / 5.6 / 5.7 ; descripteur 401 / 0.4.1 | `DOCUMENTÉ_OFFICIELLEMENT` |
| Annonces commerciales / forum | **0.5.0** (2026-09-12), **0.5.1** (2026-09-15), **0.7.0** (2026-10-10) — 8 couches + *binding* matériaux Landscape | `ANNONCÉ_PAR_LE_DÉVELOPPEUR` |
| Fiche Fab | Titre mélange « TerraDyne (0.7.0) » et « current: 0.5.1 » | `ANNONCÉ_PAR_LE_DÉVELOPPEUR` ; code **non inspecté** |

Le proto **n’est pas** l’édition 0.7. Huit couches et le binding Landscape→chunk **n’y sont pas**.

---

## 2. Résumé exécutif

1. Le proto est TerraDyne **OSS 0.4** au commit **`270c311`**, compilé dans un host **UE 5.8** (`EngineAssociation` 5.8). Le même commit public déclare **UE 5.7.0**. L’écart vient d’une **modification locale** de `TerraDyne.uplugin` (`5.7.0` → `5.8.0`), plus des patches host/plugin déjà notés (GPU stub, recombine HeightBuffer).
2. **Authored World Conversion existe dans cet OSS** (wizard, `ImportFromLandscapeWithOptions`, baker `UTerraDyneBaker`). L’import *live* Landscape est **`#if WITH_EDITOR`**. Les poids importés sont **4 couches** ; le surplus est listé `UnmappedLayerNames`. L’adoption de matériau Landscape, **si** `bAdoptLandscapeMaterialAsMasterMaterial` est activé (défaut **false**), **refuse** tout graphe contenant au moins une des expressions listées §8.1. Ce refus est **générique** : il ne nomme pas M4.
3. Le **dépôt OSS public actuel est le même commit**. Il n’apporte pas les fonctions 0.7.
4. L’édition **0.7.0** (forum, 2026-10-10) annonce 8 couches et un *material binding* capable de faire tourner un matériau Landscape sur le mesh. **Code commercial non disponible.** Même cette annonce **exclut** RVT, `landscape.MobileWeightTextureArray`, Material Layers à nœuds Landscape.
5. M4 officiel : peinture = **Landscape Paint Layers** ; automaterial = couches hauteur/pente ; chemin Voxel = collection + flag dédiés. **Aucun chemin TerraDyne équivalent n’existe dans l’OSS 0.4.** `Content/Demo/M4_Eval/` n’est **pas** une install M4 (MIC + LayerInfos, soft-refs `/Game/M4/...` cassées) — c’est la **preuve de campagne D**, distincte du scan d’adoption.
6. GardenFervor possède une install M4 **réelle**, dont `M4UE5MasterMat.uasset`. Les noms sérialisés de cet asset **indiquent** `LandscapeLayerBlend` / `LayerSample` / `GrassOutput`, mais le graphe n’a **pas** été listé en éditeur (A03-V1). Cela **n’établit pas** que le scan `UsesLandscapeOnlyMaterialExpressions` refuserait ce master.
7. Conversion `Landscape UE → TerraDyne` ≠ `RuntimeTerraformStore → FTerraDyneChunkData`. Aucun adaptateur Store n’existe (AUDIT-02 TF-10).

**Recommandation d’audit :** ne pas fonder une voie M4+TerraDyne sur l’OSS du proto. Le 0.7 annoncé n’est pas transférable par une « petite rustine » OSS, et n’inclut pas le RVT dont GardenFervor se sert. L’autorité hauteur reste le Store (AUDIT-02). **Pas une décision 08.7.**

---

## 3. Inventaire de la version locale et provenance

### 3.1 Manifeste et commit

| Champ | Prototype local | GitHub `270c311` / `main` (2026-10-12) |
|---|---|---|
| Fichier | `Plugins/TerraDyne/TerraDyne.uplugin` | même chemin upstream |
| `Version` / `VersionName` | 4 / `0.4` | 4 / `0.4` |
| `EngineVersion` | **`5.8.0`** | **`5.7.0`** |
| Modules | TerraDyne Runtime + TerraDyneEditor | identique |
| Plugins déclarés | **GeometryScripting seulement** | identique |
| Git dans le proto | **Aucun** (ni racine ni plugin) | `main` = `270c311…` |
| Preuve commit | `Results/TERRADYNE_COMMIT.txt` | API GitHub commits/main |

**Écarts de libellés locaux :** README plugin « TerraDyne 0.4 » / exigences **UE 5.7** + **VirtualHeightfieldMesh** ; CHANGELOG dont la dernière section numérotée est encore **`[0.3] - 2026-04-05`** ; uplugin 0.4. Ni VHM dans l’uplugin inspecté.

### 3.2 Host Unreal

`TerraDyneOSS_Proto.uproject` : `"EngineAssociation": "5.8"`. Description : host isolé **270c311**, pas GardenFervor. Build de référence HoleFix empaqueté sur cette association (campagne AUDIT-01). **Compatibilité vendor OSS 0.4 ≠ 5.8.3** : l’exécution locale prouve un host 5.8, pas un support officiel OSS (README/CHANGELOG = 5.7). Le forum 0.7 mentionne des tests **5.8.3** — édition **différente**.

### 3.3 Modifications locales connues (hors Git)

- `EngineVersion` uplugin 5.8.0 (upstream 5.7.0 au même hash).
- Host démo + masquage floor + overlay paint (pas le plugin GitHub).
- Recombine `HeightBuffer` `[-1,1]` documenté AUDIT-01 (copie isolée).
- `bEnableGPUBrushes=false`.

Sans git plugin, un diff exhaustif vs `270c311` **ne peut pas** être produit ici. `INCONNU` pour tout fichier non recoupé.

### 3.4 Fonctions liées à la conversion / matériaux / persist (proto)

Présentes dans le code OSS local : `MigrateLandscapeProject`, `ImportFromLandscapeWithOptions`, `ResampleLandscapeData` (**éditeur**) ; `UTerraDyneBaker` ; `InitializeFromBakedLandscapeAssetSet` (runtime, asset préparé) ; `SaveWorld` / `LoadWorld` ; `SetMaterial` + MID `HeightMap`/`WeightMap` ; `NumWeightLayers = 4` ; `LoadFromData` / `ExportChunkStatePacket`.

Absentes du code examiné : `CreateTerrainMaterialBinding`, `WeightTexture1`, `ExtraWeightData`, 8 couches, `TerraDyneMaterialBinding`.

---

## 4. Matrice comparative des versions et fonctionnalités

Légende cellules : `PRÉSENT_ET_VÉRIFIÉ` · `ABSENT_DU_CODE_EXAMINÉ` · `DOCUMENTÉ_MAIS_NON_VÉRIFIÉ_LOCALEMENT` · `ANNONCÉ_PAR_LE_DÉVELOPPEUR` · `INCONNU` · `NON_APPLICABLE`.

| Fonctionnalité | Prototype local | Dépôt OSS public `270c311` | Manuel 0.4.1 | Commercial annoncé (surtout 0.7.0) | Preuve |
|---|---|---|---|---|---|
| Version exacte | 0.4 + uplugin 5.8.0 local | 0.4, uplugin 5.7.0 | 0.4.1 / 401 | Fab 0.5.1 et/ou 0.7.0 | uplugin, README, Fab, forum |
| UE 5.8.3 | Host 5.8 compilé/exécuté HoleFix ; **pas** déclaré dans README OSS | README 5.7 | Fab 5.5–5.7 | Forum 0.7 : tests 5.8.3 | uproject ; forum 2026-10-10 |
| Conversion Landscape existant | **PRÉSENT_ET_VÉRIFIÉ** (éditeur) | même code public | **DOCUMENTÉ** wizard Authored World | **ANNONCÉ** 8 couches + bind | Manager.h `#if WITH_EDITOR` ; manuel ; forum 0.7 |
| Import hauteurs | **PRÉSENT_ET_VÉRIFIÉ** (resample composants chargés) | idem | DOCUMENTÉ | ANNONCÉ | `ResampleLandscapeData` |
| Import couches peintes | **4 max** PRÉSENT | 4 | 4 (« four weight layers ») | **8** ANNONCÉ 0.7 | `NumWeightLayers` ; baker `< 4` |
| Conservation noms de couches | Mapping + `UnmappedLayerNames` PRÉSENT | idem | DOCUMENTÉ | ANNONCÉ + noms canaux | `FTerraDyneLandscapeLayerMapping` |
| Transfert végétation | PRÉSENT (ISM + actors) | idem | DOCUMENTÉ | ANNONCÉ | Manager/baker |
| Assets monde empaqueté | `UTerraDyneLandscapeAssetSet` + `InitializeFromBakedLandscapeAssetSet` PRÉSENT | idem | DOCUMENTÉ packaged sample | ANNONCÉ bake | Manager.h L325 |
| Adoption / liaison matériau Landscape | Adoption **directe** si **aucun** nœud Landscape ; sinon **refus** | idem | Adopt **off** par défaut ; refuse landscape-only | **Binding MI** ANNONCÉ 0.7 (on by default) | `UsesLandscapeOnlyMaterialExpressions` |
| Nombre couches peinture runtime | **4** | **4** | **4** | **8** (0.7) | Chunk.h |
| Save / load | API PRÉSENTE ; host démo **non câblé** | API | DOCUMENTÉ | ANNONCÉ schema 3 | SaveGame.h v2 local |
| API adaptateur custom | `LoadFromData`, packets, `SetMaterial` | idem | DOCUMENTÉ partiel | + library binding 0.7 | AUDIT-02 TF-10 |

**Limite d’accès :** l’édition commerciale n’a **pas** été inspectée en source. Tout 0.5/0.7 = documents développeur seulement.

---

## 5. Conversion d’un Landscape existant

### 5.1 Où c’est documenté / présent

- Manuel 0.4.1 : template wizard **Authored World Conversion** ; pipeline `ULandscapeInfo`, composants **chargés**, options (hide, clear, pause streaming, weights, mappings, grass, adopt material **off**, foliage).
- OSS 0.4 proto **et** GitHub `270c311` : **même famille de code** (`ImportFromLandscapeWithOptions`, baker).
- 0.7 forum : conversion **étendue** (8 couches, binding) — **pas dans le proto**.

### 5.2 Données réellement importées (OSS 0.4, code)

- Hauteurs resampled par composant Landscape **chargé**.
- Jusqu’à **4** allocations de weightmaps (hors visibility) → `WeightBuffers[0..3]` ; noms dans `LayerMappings` ; le reste → `UnmappedLayerNames`.
- Foliage ISM + actor foliage si options on.
- Métadonnées `FTerraDyneLandscapeMigrationState` sur le manager / asset set.
- Matériau : copie de `LandscapeMaterial` **seulement** si `bAdoptLandscapeMaterialAsMasterMaterial` est vrai **et** si le graphe n’a aucune des expressions §8.1 ; sinon le master TerraDyne est conservé. Défaut de l’option : **false**.

### 5.3 Pertes / limites (OSS 0.4)

- Composants **non chargés** (World Partition / proxies) : import échoue ou partiel — le code exige `SourceComponents.Num() > 0` et le notifie.
- Trous de visibilité : refus si `bRejectLandscapeVisibilityHoles`.
- Couches au-delà de 4 : **non importées** dans les buffers.
- Nœuds Landscape du matériau : s’ils appartiennent à la liste §8.1, l’adoption est **refusée** (le matériau source n’est pas copié comme master). Cela ne prouve pas que `M4UE5MasterMat` en contient.
- Import live : **éditeur seulement** (`#if WITH_EDITOR`). Runtime packaged : **asset set déjà baké**, pas un Landscape live.
- Conversion **non exercée** sur GardenFervor Island dans cette mission.

### 5.4 Save après import

Les buffers + migration state **peuvent** entrer dans `UTerraDyneSaveGame` v2 **si** `SaveWorld` est appelé. Host démo : **non appelé** (AUDIT-01). Ronde-trip **NON VALIDÉ**.

### 5.5 Deux conversions distinctes

| Opération | Statut |
|---|---|
| Landscape UE → chunks TerraDyne | Code OSS **présent** (éditeur) ; **non testé** ici sur l’île GF |
| `RuntimeTerraformStore` → `FTerraDyneChunkData` | **Aucun adaptateur.** Il faudrait : rééchantillonnage XY, cm→normalisé/`ZScale`, tuilage, 4 poids ≠ paint Store, **sans** `ApplyGlobalBrush`. AUDIT-02 TF-10. |

Ne pas transférer l’autorité de hauteur pour « faciliter » une conversion.

---

## 6. Documentation officielle M4 consultée

Date de consultation : **2026-10-12**. Source première : https://aaronneal.online/docs/m4/

| Page | URL | Constat utile |
|---|---|---|
| Accueil | `/docs/m4/` | Index ; maj affichée 26 Nov 2021 sur l’accueil fetché |
| Painting on the terrain | `/getting-started/painting-on-the-terrain/` | Peinture **comme un Landscape UE** ; **18 paint layers** ; mode Landscape → onglet Paint |
| UE 5.3+ landscape setup | `/unreal-engine-53-stuff/how-to-setup-a-landscape-in-ue53/` | Réglages **Landscape** (pas mesh générique) |
| Automaterial layers | `/using-the-automaterials/automaterial-layers/` | Flatland A/B, Foothills, Cliffs, Beach, Wet Beach — **hauteur / pente**, pas les 4 poids TerraDyne |
| Custom paint layers | `/paint-layer-customization/usingcustompaintlayers/` | Custom 01/02 via **Material Instance Landscape** |
| Creating paint layer | `/paint-layer-customization/creating-your-own-paint-layer/` | Duplicate Paint Layer function ; **Layer Blend** dans le **Master** ; `LB Height Blend` ; paint menu Landscape |
| Voxel Plugin | `/voxel-plugin-easy-setup/using-the-materials-with-voxel-plugin/` | Chemin **officiel distinct** : Voxel Landscape Material Collection + `Enable Voxel Plugin Support` + Multi Index. **Ne prouve pas TerraDyne.** |

**Pas d’équivalent TerraDyne** dans la doc M4. Communauté non requise pour ces faits.

Implication : M4 attend le **vertex factory Landscape** (poids de couches, coords Landscape). L’OSS TerraDyne 0.4 pousse un `UDynamicMeshComponent` + texture `WeightMap` RGBA (4 canaux) + params `HeightMap`/`WorldSize`/`ZScale`/`TexelStep`.

---

## 7. Assets M4 réellement disponibles

### 7.1 Prototype `Content/Demo/M4_Eval/` — **pas une install M4**

README local (inspecté) : copies **fichiers** depuis GF pour essai MIC ; **volontairement absents** plugin M4, master, PaintLayers, textures. Soft-refs `/Game/M4/...`.

Présents : `Island_UE5Preset5_Inst.uasset` + LayerInfos Auto, Grass, Dirt, Sand, Mud, Forest, Stones, Jagged_Rock, Flat_Ice, Custom01, Custom02.

### 7.2 GardenFervor — install M4 relocalisée

Chemins inspectés (existence fichiers, **pas** décompilation des graphes) :

- Masters : `Content/GardenFervor/Environment/Materials/MasterMaterials/M4UE5MasterMat.uasset`, `M4HeightfieldMasterMat.uasset`
- Preset île : `.../PresetInstances/Island/Island_UE5Preset5_Inst.uasset` (+ Preset1–6, Nanite)
- Helper : `.../Blueprints/MagicMapMaterialBP-V2.uasset` + `SystemResources/MagicMapMaterialBP/`
- Textures / trails / sound M4 présents sous Environment

Inspection **sans** ouverture éditeur ni décompilation de graphe (2026-10-12) :

| Asset | Constat | Limite |
|---|---|---|
| `M4UE5MasterMat.uasset` | Noms sérialisés : `LandscapeLayerBlend` (2), `LandscapeLayerSample` (1), `LandscapeGrassOutput` (1), `MaterialExpressionLandscape` (4), `RuntimeVirtualTexture` / `RVT` | Un nom dans la table **n’est pas** un nœud vivant confirmé. `HasAnyExpressionsInMaterialAndFunctionsOfType` n’a **pas** été exécuté. |
| `Functions/UE58Compatibility/MF_LandscapeLayerBlend_UE58safe.uasset` | Fonction **présente** ; noms `LandscapeLayerBlend` (8) | Le master **ne contient pas** la chaîne `MF_LandscapeLayerBlend_UE58safe` (0 occurrence). Usage par le master **non établi**. |
| `Functions/SplatmapDetection.uasset` | Noms `LandscapeLayerCoords`, `LandscapeLayerSample` | Idem : pas de preuve que le master l’appelle. |
| `Functions/Output-RVT.uasset` | Nom `LandscapeVisibilityMask` + RVT | Idem. |
| `Island_UE5Preset5_Inst.uasset` | Noms RVT ; **pas** des noms LayerBlend | Instance, pas le master. |

**Conclusion 7.2 :** on **ne dispose pas** d’une preuve que le graphe de `M4UE5MasterMat` contient une expression de la liste §8.1. Les noms sérialisés et la doc officielle (« Layer Blend dans le Master ») sont des **indications**, pas un listing d’expressions. A03-V1 reste ouvert. Ne pas traiter cette indication comme une incompatibilité démontrée du master.

### 7.3 Plugin M4 dans le proto

**Absent** des plugins du `.uproject` proto (GeometryScripting + TerraDyne seulement).

---

## 8. Mécanismes de rendu TerraDyne (OSS 0.4 proto)

- Mesh : `UDynamicMeshComponent` ; rebuild async ; positions + **normals** ; UV overlay à la création de topologie ; flags `UVs` + `NormalsTangents`.
- Matériau runtime : slot 0 ; `ATerraDyneChunk::SetMaterial` (`TerraDyneChunk.cpp` ~L1016) crée un **MID** et pousse `HeightMap`, `WeightMap`, `WorldSize`, `ZScale`, `TexelStep`. **Aucun** scan `UsesLandscapeOnlyMaterialExpressions`. Ce n’est **pas** le set de textures de poids Landscape.
- Paint : `WeightTexture` BGRA 4 couches ; upload CPU.
- Reconstruct : `Mesh.Clear` / topology change peut invalider les overlays (campagne : vertex colors).
- Interfaces GF : `LoadFromData`, `SetMaterial` — pas un bus M4.

### 8.1 Mécanisme d’adoption (éditeur seulement) — démontré

Fonction locale `UsesLandscapeOnlyMaterialExpressions` :

- `TerraDyneManager.cpp` L103–118, sous `#if WITH_EDITOR`.
- Duplicata baker : `TerraDyneBaker.cpp` L69–84 (module Editor).

Elle retourne vrai si le `UMaterial` résolu contient, **dans le matériau ou ses material functions**, au moins un :

`UMaterialExpressionLandscapeGrassOutput` · `LandscapeLayerBlend` · `LandscapeLayerCoords` · `LandscapeLayerSample` · `LandscapeLayerSwitch` · `LandscapeLayerWeight` · `LandscapeVisibilityMask`.

Conséquences, uniquement si `FTerraDyneLandscapeMigrationOptions::bAdoptLandscapeMaterialAsMasterMaterial` est **true** (défaut **false**, `TerraDyneManager.h` L89 ; tooltip wizard : *« Off by default; landscape materials usually do not work on dynamic meshes »*) :

- log `Skipping adoption… Landscape-only material expressions` ;
- notification éditeur ;
- `MasterMaterial` TerraDyne **conservé** ; `bAdoptedLandscapeMaterial` reste false.

Si l’option est false, le scan n’est **pas** utilisé : le matériau Landscape n’est simplement pas adopté.

Ce mécanisme **ne cite pas M4**. Il refuse une **catégorie** de graphes. Il n’est **pas** le chemin campagne (assignation MIC via `SetMaterial`).

Forum 0.7 (non présent ici) décrit UV0–2 coords Landscape, UV3 weights, tangentes +X — **autre layout** que l’OSS 0.4.

---

## 9. Comparaison des voies A–E

### A. Utilisation directe de M4 (OSS 0.4)

Deux mécanismes distincts :

1. **Campagne** (`DEMO_TERRADYNE_M4.md` §8) : copie MIC `Island_UE5Preset5_Inst` **sans** master / plugin / textures ; soft-refs `/Game/M4/...` cassées. Verdict **D**. Preuve = install incomplète + doc M4 (peinture = Landscape Paint Layers), **pas** le scan §8.1.
2. **Adoption à la conversion** : refuse les graphes §8.1 **si** l’option d’adoption est on. `M4UE5MasterMat` n’a **pas** été passé à ce scan. Appartenance du master à l’ensemble refusé : **non établie**.

Assignation ≠ compilation ≠ couches/effets. Ne pas conclure que toute fonctionnalité M4 est impossible après adaptation (voie C).

### B. Liaison matériau Landscape (0.7 annoncé)

**Absente du proto et de l’OSS `270c311`.** Exigences d’adoption (documents seulement) : édition **0.7.0**, baker/binding MI, 8 canaux, UE testée 5.8.3 *selon le forum*. Transfert vers OSS 0.4 = **pas une modification simple** (nouveau layout UV, 2 textures de poids, codec save 3, 28 tests annoncés). Limites **annoncées** : pas de RVT, pas MobileWeightTextureArray, pas Material Layers Landscape, GrassOutput ignoré, Lumen plus sombre. GardenFervor M4+RVT (F2) resterait **incomplet** même en 0.7. `ANNONCÉ_PAR_LE_DÉVELOPPEUR` / `INCONNU` runtime M4.

### C. Adaptation contrôlée de M4 au mesh

Hypothèse : extraire auto-couches pente/hauteur **sans** nœuds Landscape, alimenter 4 poids. **Non démontré.** Coût : fork du master M4, perte paint 18 couches / Custom / RVT / trails Landscape, maintenance à chaque update Aaron Neal. `POSSIBLE_SOUS_CONDITIONS` au mieux ; **pas** une compatibilité.

### D. GardenFervor autorité + représentation

Conforme AUDIT-02. Représentation TerraDyne depuis le Store : adaptateur **absent**. Conversion Landscape **n’est pas** une preuve Store. M4 resterait sur le **Landscape GF** (chemin actuel F2) si 08.7 le conserve.

### E. Abandon de la voie M4-sur-TerraDyne

Issue **cohérente** avec D + A : garder M4 sur Landscape (autorité visuelle actuelle 08.7) ; TerraDyne OSS 0.4 = proto d’évaluation géométrique, **pas** de rendu M4. Alternative 0.7 = achat + preuve RVT manquante + conflit d’autorité si mal câblé.

---

## 10. Matrice de compatibilité et niveaux de preuve

| Sujet | Verdict | Niveau |
|---|---|---|
| Conversion Landscape dans le proto OSS 0.4 | Présente **éditeur** ; 4 couches ; adopt matériau si graphe « mesh-safe » | `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` |
| Même conversion dans OSS public | Oui, **même commit** | `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` |
| Conversion 8 couches + bind 0.7 | Annoncée, code **inaccessible** | `ANNONCÉ_PAR_LE_DÉVELOPPEUR` |
| Assignation MIC M4 incomplet (campagne) | **D** | `DEMO_TERRADYNE_M4.md` §8 + `M4_Eval/README.md` |
| Scan d’adoption OSS 0.4 | Refuse les 7 types §8.1 ; option défaut **off** | `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` |
| `M4UE5MasterMat` ∈ ensemble refusé par ce scan | **Non établi** | Noms sérialisés = indication ; graphe éditeur **non listé** (A03-V1) |
| Liaison Landscape materials 0.4 | Adoption conditionnelle ; refus **générique** | `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` |
| Liaison 0.7 | Non dans le proto | `ANNONCÉ_PAR_LE_DÉVELOPPEUR` |
| Automaterial M4 (hauteur/pente) | Non démontré sur DynamicMesh 0.4 | `NON_DÉMONTRÉE_PREUVES_INSUFFISANTES` |
| Paint layers M4 + poids | Doc : 18 Landscape vs 4 RGBA plugin ; **pas** un graphe M4 inspecté | `DOCUMENTÉ_OFFICIELLEMENT` (M4) + `INSPECTÉ_DANS_LE_CODE` (4 couches) ; incompatibilité **directe des canaux** pour la voie native, sans preuve que le master utilise ces 18 nœuds |
| Rendu après rebuild | Overlay démo ≠ M4 ; topology dirty UV/normals | `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` |
| Adaptation M4 | Non faite | `POSSIBLE_SOUS_CONDITIONS` / non acquis |
| Autorité Store (AUDIT-02) | Conservée | Recommandation A02, **pas** 08.7 |
| Raise/Lower HoleFix | Utilisateur | `DEMONTRÉ_MANUELLEMENT` — **pas M4** |
| RVT M4 | Requis côté GF F2 ; **non bound** même en 0.7 annoncé | Doc GF + forum 0.7 Known Limitations |

---

## 11. Verdicts par capacité

1. Conversion Landscape **proto OSS** : **oui, éditeur, 4 couches**, non testée ici sur l’île GF.  
2. Conversion **OSS public** : **oui**, même `270c311`.  
3. Conversion **commerciale documentée** : 0.4.1 = 4 couches + refus landscape-only ; **0.7 = 8 couches + binding** (annonce).  
4. Compatibilité M4 / rendu TerraDyne **actuel** : verdict campagne **D conservé**. Portée = essai MIC **incomplet** + absence de chemin M4 officiel vers DynamicMesh. **Pas** `INCOMPATIBILITÉ_TECHNIQUE_DÉMONTRÉE` du graphe `M4UE5MasterMat`.  
5. Liaison matériaux Landscape OSS 0.4 : **refus générique** des 7 types §8.1, option défaut **off**. Appartenance de M4 à ce refus : **non établie**. 0.7 = annoncé hors proto.  
6. Couches auto M4 : **non démontrées** sur chunks 0.4.  
7. Couches peintes / poids : 18 (doc M4) vs 4 (code OSS) — écart de **canaux** inspecté ; fonctionnement réel des poids M4 sur chunk : **non démontré**.  
8. Conservation rendu après edit/rebuild : overlay TerraDyne utilisateur **oui** ; M4 **non**.  
9. Adaptation contrôlée M4 : **non démontrée** ; coût/maintenance élevés.  
10. Autorité GF : **conceptuellement séparable** ; conversion Landscape **≠** Store.

Choix global voie directe M4/TerraDyne OSS 0.4 : **D conservé** (campagne). Libellé technique : **`NON_DÉMONTRÉE_PREUVES_INSUFFISANTES`** pour l’incompatibilité **propre à `M4UE5MasterMat`** avec le scan d’adoption ; **`INCOMPATIBILITÉ_TECHNIQUE_DÉMONTRÉE`** uniquement pour (a) le **mécanisme générique** de refus des 7 expressions, (b) l’essai MIC **sans** master dans le proto.  
0.7 : **`NON_DÉMONTRÉE_PREUVES_INSUFFISANTES`** (plus RVT explicitement hors binding).  
Adaptation (voie C) : inchangée — **non démontrée**, pas un verdict positif.

---

## 12. Inconnues, risques, vérifications restantes

| Id | À démontrer plus tard (sans le faire maintenant) |
|---|---|
| A03-V1 | Ouvrir `M4UE5MasterMat` en éditeur et lister les expressions Landscape **vivantes** (y compris dans les material functions utilisées). Les noms sérialisés 2026-10-12 **ne remplacent pas** ce listing. |
| A03-V2 | Si un jour 0.7 est acquis : binding M4 **avec** et **sans** RVT ; mesurer l’écart Lumen annoncé |
| A03-V3 | Exercer Authored World Conversion **éditeur** sur un Landscape de test isolé (pas GF) — hauteurs + 4 poids |
| A03-V4 | Spécifier l’adaptateur Store→`FTerraDyneChunkData` (doc) sans changer D1 |
| FUT-04 | Save/Load TerraDyne après une conversion baker |

Risques : prendre Fab 0.7 pour l’OSS du proto ; croire que 8 couches = M4 18 couches + RVT ; convertir l’île Landscape en transférant l’autorité.

---

## 13. Conclusion générale

L’OSS du prototype (**0.4 / `270c311`**, host UE 5.8) **sait** convertir un Landscape **éditeur** vers des chunks, avec **4** peintures. L’adoption d’un matériau Landscape est **off** par défaut et **refuse** une liste générique d’expressions Landscape si on l’active. Cela **ne démontre pas** que `M4UE5MasterMat` appartient à cette liste. La campagne **D** démontre qu’un MIC M4 **incomplet** n’est pas un rendu M4 sur DynamicMesh. Le manuel 0.4.1 (adopt off, 4 layers) et le forum **0.7.0** restent des éditions distinctes.

**D reste D**, avec une portée **restreinte**. Ce n’est **pas** un verdict positif d’adaptation. Aucune intégration, aucun 08.7.

`Design Gate : aucun contrat ni statut modifié — audit M4, versions et compatibilité en lecture seule.`

---

## 14. Fichiers documentaires modifiés

Vérification ciblée du **2026-10-12** (portée du verdict M4 seulement) :

- `ExternalTechnologyAudits/01_TerraDyne/01_Audits/AUDIT-03/REPORT.md` (ce fichier)
- `ExternalTechnologyAudits/AUDIT_REGISTER.md`
- `ExternalTechnologyAudits/01_TerraDyne/README.md`

**Signalés, non modifiés ici :** `AUDIT-03/README.md`, `FINDINGS.md` (TF-08 / TF-13), `TESTS.md` A03-V1, `ETAT_PROJET.md`, `HISTORIQUE_MODIFICATIONS.md`.

**Non modifié :** proto TerraDyne, code GF, assets, config, contrats, `designGate.js`, `REGLES_PROJET.md`. AUDIT-01 et AUDIT-02 non réécrits.

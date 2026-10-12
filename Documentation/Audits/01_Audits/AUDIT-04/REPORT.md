# AUDIT-04 — Compatibilité M4 à nœuds avec TerraDyne OSS 0.4

| Champ | Valeur |
|---|---|
| Identifiant | `EXT-TERRADYNE-A04` |
| Date | **2026-10-12** |
| Statut documentaire | `TECHNICAL_REVIEW_COMPLETE` |
| Prototype | `C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311` |
| Plugin | TerraDyne OSS **0.4** commit **`270c311`** — **source plugin non modifiée** |
| Master M4 | `Content/M4/Materials/MasterMaterials/M4UE5MasterMat.uasset` — **graphe non modifié** |
| Intégration GardenFervor | **Non autorisée** |
| 08.7 / C-01 | **Non modifiés** |

**Design Gate : investigation et réparation autorisées dans le prototype TerraDyne isolé uniquement. Aucun changement de contrat, aucune adoption de TerraDyne et aucune modification de l’autorité terrain de GardenFervor.**

Ce rapport n’écrase pas AUDIT-01, AUDIT-02, AUDIT-03 ni `DEMO_TERRADYNE_M4.md`.

---

## 1. État initial exact

| Fait | Preuve |
|---|---|
| Campagne **D** | MIC `Content/Demo/M4_Eval/` sans master / textures ; soft-refs `/Game/M4/...` cassées (`DEMO_TERRADYNE_M4.md` §8) |
| AUDIT-03 | Refus d’adoption **générique** (7 expressions Landscape) ; appartenance de `M4UE5MasterMat` **non établie** (A03-V1 ouvert) |
| `Content/M4` proto | 1450 `.uasset` présents **après** A03 ; présence ≠ compatibilité |
| Host démo | `TryApplyM4Material` chargeait `/Game/Demo/M4_Eval/Island_UE5Preset5_Inst` puis `/Game/M4/Maps/Island_Demo_sharedassets/...` (**chemin faux**) |
| Autorité GF | `RuntimeTerraformStore` (C-01 D1) — hors proto |

Docs M4 officielles consultées (2026-10-12) : [index](https://aaronneal.online/docs/m4/), [peinture Landscape](https://aaronneal.online/docs/m4/getting-started/painting-on-the-terrain/) (18 paint layers), [création de couche](https://aaronneal.online/docs/m4/paint-layer-customization/creating-your-own-paint-layer/) (Layer Blend dans le Master), [automaterial](https://aaronneal.online/docs/m4/using-the-automaterials/automaterial-layers/), [Voxel Plugin](https://aaronneal.online/docs/m4/voxel-plugin-easy-setup/using-the-materials-with-voxel-plugin/) (chemin officiel **distinct**, pas TerraDyne).

Manuel TerraDyne 0.4.1 : adoption Landscape **off** par défaut ; graphes landscape-only refusés. Forum **0.7** : binding MI 8 couches, RVT « not bound » — **absent du proto**.

---

## 2. Causes reproduites (distinctes)

### C1 — Références d’assets / mauvais chemin (démontré, corrigé côté host)

- Orphan `M4_Eval` : **charge encore** (`orphanEval=ok`) mais n’est **pas** le graphe GardenFervor.
- `/Game/M4/Maps/Island_Demo_sharedassets/Island_UE5Preset5_Inst` : **FAIL** (dossier inexistant).
- MIC réel : `/Game/M4/Materials/PresetInstances/Island/Island_UE5Preset5_Inst` — **ok**, parent `M4UE5MasterMat`.

### C2 — Expressions Landscape **vivantes** sur le master (A03-V1 désormais exécuté)

Self-test éditeur `-game -nullrhi -M4CompatSelfTest` (`GetExpressions()` sur le master) :

- `MaterialExpressionLandscapeLayerBlend` (1, direct)
- `MaterialExpressionLandscapeGrassOutput` (1)
- `MaterialExpressionLandscapeLayerSample` (22)
- `MaterialExpressionLandscapePhysicalMaterialOutput` (1)
- **Total direct : 25** expressions dont le nom contient `Landscape` (hors fonctions imbriquées)

`MF_LandscapeLayerBlend_UE58safe` (appelée par le master ; switch **Enable UE 5.8 Safe Landscape Layer Blending = true**) contient encore `MaterialExpressionLandscapeLayerBlend`.

Le scan OSS `UsesLandscapeOnlyMaterialExpressions` **refuserait** donc ce master si `bAdoptLandscapeMaterialAsMasterMaterial` était true.

### C3 — Paramètres TerraDyne vs graphe M4 (démontré)

`Island_UE5Preset5_Inst` : **HeightMap** et **WeightMap** **non déclarés**.  
`ATerraDyneChunk::SetMaterial` pousse ces noms sur le MID : **sans effet** sur le shader M4.

### C4 — Switches M4 du preset île (démontré, valeurs éditeur)

| Switch | Valeur sur `Island_UE5Preset5_Inst` |
|---|---|
| Enable Voxel Plugin Support | **false** |
| Enable World Position UVs | **false** |
| Enable Splatmap Instead of Auto Detection | **false** |
| Enable RVT Support on Landscape Actor | **false** |
| Enable UE 5.8 Safe Landscape Layer Blending | **true** |

Le chemin officiel Voxel **n’est pas** activé sur le preset GardenFervor. L’activer au runtime sur un MID **ne change pas** les permutations static (nécessite un MIC éditeur).

### C5 — Assignation GPU sous `-nullrhi` (limite de test, pas une cause M4)

`SetMaterial` retourne si `!FApp::CanEverRender()`. SLOT0 = **null** dans le self-test automatisé. L’appel host a lieu ; le slot mesh n’est pas posé sans renderer.

### Non attribué

- Compilation Game/Editor Development : **succès** (host seulement).
- Hauteur Raise/Lower (plan de données TerraDyne) : **PASS** pendant le self-test, indépendant du graphe M4.
- Erreur de compilation shader M4 sur DynamicMesh : **non capturée** (pas de renderer dans ce run).

---

## 3. Solution implémentée (proto uniquement)

Adaptation **externe** (host), sans fork du master, sans rustine plugin :

1. Charger le MIC réel `Island_UE5Preset5_Inst` (parent `M4UE5MasterMat`).
2. `SetMaterial` TerraDyne inchangé (plugin).
3. Désactiver l’overlay vertex-color (ce n’est pas M4).
4. Self-test `-M4CompatSelfTest` : inventaire d’assets, diagnostics, Raise/Lower.
5. Bouton HUD **M4** existant : pointe désormais vers le vrai MIC.

**Non fait (volontaire) :**

- Remplacer le master par un matériau simplifié.
- Désactiver silencieusement des nœuds Landscape.
- Modifier `UsesLandscapeOnlyMaterialExpressions` / baker / `SetMaterial` plugin.
- Binding style 0.7 (intrusif ; édition absente).
- Intégration GardenFervor.

### Pourquoi pas une « réparation complète » du graphe

Une compatibilité **complète** du périmètre M4 GardenFervor (automaterial + 18 paint layers + Layer Blend + RVT + grass) sur DynamicMesh OSS 0.4 exigerait l’un de :

- **Binding 0.7** (plugin commercial, RVT annoncé non bound) ;
- **Réécriture du master** (remplacer LandscapeLayerBlend par un sample `WeightMap`) — interdit comme solution finale ici ;
- **Voxel Plugin** (chemin M4 officiel, autre middleware).

Présenter l’une de ces voies comme déjà réalisée serait faux.

---

## 4. Fichiers et assets modifiés

### Prototype (code host)

| Fichier | Nature |
|---|---|
| `Source/TerraDyneOSS_Proto/TerraDyneGardenDemoHost.cpp` | Chemins M4 réels, diagnostics, self-test, overlay off si M4 |
| `Source/TerraDyneOSS_Proto/TerraDyneGardenDemoHost.h` | `RunM4CompatSelfTest` / flag `bUseNativeM4Material` |
| `Source/TerraDyneOSS_Proto/TerraDyneGardenDemoGameMode.cpp` | `-M4CompatSelfTest` |
| `Results/m4_compat_selftest.log` | Sortie automatisée |
| `Saved/Logs/TerraDyneOSS_Proto.log` | DIAG expressions + switches (2026-10-12 14:33) |

**Non modifié :** `Plugins/TerraDyne/**`, `Content/M4/**` (graphe, fonctions, instances).

### GardenFervor (documentaire seulement)

Ce rapport, registre, README TerraDyne, TESTS, FINDINGS (TF-15), preuves index, ETAT / HISTORIQUE / REGISTRE. **Pas de C++ GF, pas de `designGate.js`, pas de contrat.**

---

## 5. Matrice de validation

Conditions du run automatisé : UnrealEditor **5.8.3**, `-game -nullrhi -unattended -M4CompatSelfTest`, map `L_TerraDyneGardenDemo`.

| Domaine | Résultat | Limite |
|---|---|---|
| Matériau M4 maître | **Chargé** (`class=Material`). 25 expressions Landscape **directes** listées. Graphe **non substitué**. | Compilation shader sur DynamicMesh **non vérifiée** (nullrhi) |
| Material Functions / instances | **17/17** assets requis chargés (master, MIC île, UE58 blend, splatmap, FlatlandA, Cliff, 10 paint layers, Output-RVT) | Pas un test de chaque paramètre d’instance |
| Couches Landscape | Noms / fonctions paint **présents**. WeightMap TerraDyne **4 canaux** ≠ 18 Landscape layers | Affichage des couches **non vérifié** |
| Peinture M4 | **Non vérifié** / **incompatibilité de bus démontrée** (LayerBlend + LayerSample vs WeightMap non déclaré) | Humain en PIE requis pour confirmer l’absence visuelle |
| Hauteur Raise/Lower | **PASS** data plane (`before=0 raise→346.3 lower→0`) | Un probe ; pas l’île GF |
| Interaction terraform × matériau | Overlay M4 désactivé pour ne pas masquer le MIC. Interaction visuelle **non vérifiée** | |
| Collision | **Non vérifié** (debounce) | HoleFix historique ≠ ce run |
| Sauvegarde | **Non vérifié** — `SaveWorld` host non câblé | |
| Packaging | **Non vérifié** | Game exe Development sans Zen store n’est pas un cook |
| Régression M4 | Graphe master **non édité** ; fonctions toujours loadables | Visuel Landscape GF **non rejoué** (hors proto) |

---

## 6. Compatibilité — niveaux exigés

| Niveau | Périmètre | Verdict |
|---|---|---|
| **Démontrée** | Chargement du MIC réel + master + fonctions paint/RVT/UE58 dans le proto ; Raise/Lower TerraDyne après tentative d’assignation ; listing éditeur des nœuds Landscape du master | oui |
| **Partielle** | Host peut *demander* `SetMaterial(Island_UE5Preset5_Inst)` sans simplifier le graphe | oui, GPU non prouvé ici |
| **Non démontrée** | Automaterial visible sur chunk ; transitions ; RVT ; trails ; foliage grass ; permutation Voxel/WPO ; packaged | oui |
| **Incompatibilité démontrée** | (1) Nœuds Landscape **vivants** ∈ ensemble du scan d’adoption OSS 0.4. (2) M4 **ne déclare pas** HeightMap/WeightMap. (3) Peinture M4 = Landscape layers (doc officielle) vs 4 buffers plugin. (4) 0.7 binding **absent**. | oui |

**Pas « 100 % compatible ».** Le périmètre M4 GardenFervor (preset île + paint layers + automaterial + RVT F2) n’est pas inventorié comme entièrement fonctionnel sur TerraDyne.

---

## 7. Architecture vis-à-vis de GardenFervor (non intégrée)

GardenFervor **conserve** l’autorité hauteur (`RuntimeTerraformStore`, C-01 D1). Une représentation TerraDyne, si 08.7 la retenait un jour, devrait **suivre** le Store (`LoadFromData`), pas `ApplyGlobalBrush`.

| Donnée | GardenFervor | TerraDyne OSS 0.4 | M4 réel |
|---|---|---|---|
| Hauteur | Store base+delta (cm) | `HeightBuffer` normalisé / `ZScale` | World Z / pente (automaterial) ; **pas** HeightMap TerraDyne |
| Peinture | Couches Landscape M4 (F2) | 4 `WeightBuffers` RGBA | `LandscapeLayerBlend` / `LayerSample` |
| Rendu | Landscape + RVT (F2 PASS) | `UDynamicMeshComponent` + MID | Master Landscape, RVT optionnel |

Adaptateur Store → `FTerraDyneChunkData` : **toujours absent** (TF-10).  
Alimenter M4 depuis le Store **sans** Landscape : **non démontré**.

---

## 8. Coûts et risques de maintenance

| Voie | Coût | Risque |
|---|---|---|
| Host actuel (charger le vrai MIC) | Faible | Visuel rose/noir/preview-weight ; grass/RVT inopérants |
| MIC éditeur Voxel + WPO | Moyen ; permutations M4 | Automaterial possible **peut-être** ; paint layers toujours Landscape ; non testé |
| Fork du master (retirer LayerBlend) | Élevé à chaque update Aaron Neal | Interdit comme « solution finale » ici |
| Patch plugin type 0.7 | Très élevé ; forks OSS | Maintenance divergente ; RVT 0.7 annoncé absent |
| Achat 0.7 | Commercial + preuves | Pas dans le proto ; RVT « not bound » ; 08.7 toujours ouvert |

---

## 9. Recommandation pour la fondation terrain GardenFervor

1. **Ne pas** fonder le rendu île sur TerraDyne OSS 0.4 + M4 natif : le graphe réel **est** landscape-only (A03-V1 clos).
2. **Conserver** M4 sur Landscape comme référence visuelle actuelle (08.7 **À DÉFINIR**, F2 PASS).
3. **Conserver** le Store comme autorité (option B AUDIT-02).
4. Traiter TerraDyne OSS comme proto **géométrique** (HoleFix), pas comme runtime M4.
5. Une fondation « terraform visible + M4 intact » dans *ce* proto n’est **pas** obtenue. Le blocage n’est plus « MIC introuvable » ; c’est le **vertex factory Landscape** du master.

---

## 10. Étape suivante (non exécutée)

Au choix humain, **sans** l’enchaîner ici :

1. PIE proto (éditeur Development déjà compilé) → bouton **M4** → observer le chunk (textures automaterial / rose / preview weights) ; Raise/Lower ; collision après délai.
2. Créer un **MIC éditeur** `MI_M4_TerraDyneWPO` (parent inchangé) avec Voxel + World Position UVs, **sans** éditer le master — test d’automaterial seulement.
3. Spécifier (document) l’adaptateur Store → `LoadFromData` (A03-V4) si 08.7 l’exige.
4. **Ne pas** acheter/porter 0.7, **ne pas** patcher le plugin, **ne pas** intégrer GF, sans ordre distinct.

---

## 11. Preuves

| Preuve | Emplacement |
|---|---|
| Self-test | proto `Results/m4_compat_selftest.log` |
| DIAG expressions / switches | proto `Saved/Logs/TerraDyneOSS_Proto.log` (~14:33, 2026-10-12) |
| Host | proto `TerraDyneGardenDemoHost.cpp` `TryApplyM4Material` / `RunM4CompatSelfTest` |
| Scan adoption (inchangé) | `Plugins/TerraDyne/.../TerraDyneManager.cpp` L103–118 |

`Design Gate : investigation et réparation autorisées dans le prototype TerraDyne isolé uniquement. Aucun changement de contrat, aucune adoption de TerraDyne et aucune modification de l’autorité terrain de GardenFervor.`

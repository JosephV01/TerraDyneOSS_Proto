# Constats techniques transversaux — TerraDyne

Chaque constat a été établi avant ce référentiel. Aucun n’est une décision d’architecture. Aucun n’autorise l’intégration.

## TF-01 — Plancher du template masque le creusement

- **Description :** `Floor_0` / `SM_Template_Map_Floor` à Z=0 cachait le DynamicMesh abaissé ; l’utilisateur ne voyait pas de trou malgré un Lower API.
- **Source / version :** host proto ; diagnostic campagne `DEMO_TERRADYNE_M4.md` ; corrigé visuellement par masquage du plancher (HoleFix).
- **Niveau de preuve :** VALIDÉ — campagne + utilisateur HoleFix.
- **Conséquences :** ne pas attribuer l’échec visuel au clamp HeightBuffer comme cause racine.
- **Incertitudes :** comportement si d’autres meshes template restent visibles.
- **Audits :** A01.

## TF-02 — HeightBuffer recombinant Base+Sculpt+Detail

- **Description :** hauteur combinée dans HeightBuffer ; clamp historique `[-1,1]` après edit (LowerFix). Utile à l’API, **pas** la cause du creusement invisible.
- **Source / version :** plugin `TerraDyneChunk` OSS `270c311`.
- **Niveau de preuve :** INSPECTÉ DANS LE CODE.
- **Conséquences :** modèle d’autorité hauteur ≠ Store GardenFervor 08.6 ; à traiter en audit 2.
- **Incertitudes :** mapping exact UU ↔ normalized sur ZScale 3000.
- **Audits :** A01 ; A02.

## TF-03 — Collision DynamicMesh différée

- **Description :** collision ComplexAsSimple, debounce ~0,2 s, cook asynchrone, max 1 collision/frame ; LOD lointain sans collision.
- **Source / version :** plugin + confirmation utilisateur HoleFix (collision après délai).
- **Niveau de preuve :** code INSPECTÉ ; comportement après délai VALIDÉ MANUELLEMENT PAR L’UTILISATEUR (HoleFix).
- **Conséquences :** unités RTS pendant la fenêtre 0,2 s + LOD : non mesurés.
- **Incertitudes :** navmesh, traces Landscape GF, multi-chunks.
- **Audits :** A01.

## TF-04 — Sauvegarde plugin non câblée au host

- **Description :** `SaveWorld`/`LoadWorld` existent ; la démo ne les appelle pas.
- **Source / version :** Manager plugin OSS `270c311` ; host démo.
- **Niveau de preuve :** INSPECTÉ DANS LE CODE ; persist **NON VALIDÉ**.
- **Conséquences :** aucun argument F1 / C-19 à partir de la démo.
- **Incertitudes :** format SaveGame v2 vs contrat C-19.
- **Audits :** A01.

## TF-05 — Résolution chunk clampée 32–256

- **Description :** pas démo ≈ 107,5 UU ; clamp `InitializeChunk` 32–256 ; île 25 cm non démontrable sur un chunk unique ainsi borné.
- **Source / version :** plugin OSS `270c311`.
- **Niveau de preuve :** INSPECTÉ DANS LE CODE.
- **Conséquences :** objectif résolution île GF **NON VALIDÉ**.
- **Incertitudes :** multi-chunks / streaming non évalués.
- **Audits :** A01 ; A02.

## TF-06 — GPU brushes stub

- **Description :** `bEnableGPUBrushes=false` ; chemin GPU non opérationnel dans cette baseline.
- **Source / version :** config proto + plugin.
- **Niveau de preuve :** INSPECTÉ DANS LE CODE.
- **Conséquences :** perf RTS **NON VALIDÉ**.
- **Incertitudes :** coût CPU réel à l’échelle île.
- **Audits :** A01.

## TF-07 — Double autorité peinture

- **Description :** PaintStore host (couches GF) vs WeightBuffers plugin (4) ; overlay vertex colors distinct des couches Landscape M4.
- **Source / version :** host + plugin ; visuel HoleFix confirmé par l’utilisateur.
- **Niveau de preuve :** visuel VALIDÉ MANUELLEMENT ; modèle d’autorité **NON VALIDÉ** pour GF/M4.
- **Conséquences :** verdict M4 reste **D** ; clôturé documentairement par A03 (même verdict).
- **Incertitudes :** superposition M4 native.
- **Audits :** A01 ; A03.

## TF-08 — M4 non compatible (campagne)

- **Description :** essai MIC / matériau M4 sur mesh TerraDyne : verdict campagne **D**.
- **Source / version :** `DEMO_TERRADYNE_M4.md`.
- **Niveau de preuve :** constat de campagne ; **pas** clôture d’audit 3.
- **Conséquences :** pas d’intégration M4.
- **Incertitudes :** autres chemins matériau non explorés ici.
- **Audits :** A01 (campagne) ; A03 (clôture documentaire, **D conservé**).

## TF-12 — Identité OSS 0.4 / `270c311` vs uplugin local 5.8.0

- **Description :** Commit enregistré et GitHub `main` (2026-10-12) = `270c31117e20e491ef5333dd7e2f9dc133fcdc38`, manifeste `VersionName` 0.4. Upstream `EngineVersion` 5.7.0 ; copie proto `5.8.0`. README plugin cite encore UE 5.7 + VHM ; CHANGELOG s’arrête à une section `[0.3]`. Host `EngineAssociation` 5.8. Pas de git dans le proto.
- **Source / version :** `TerraDyne.uplugin` local vs raw GitHub `270c311` ; `TERRADYNE_COMMIT.txt` ; `.uproject`.
- **Niveau de preuve :** `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS`.
- **Conséquences :** ne pas traiter le proto comme TerraDyne 0.7 ni comme OSS « vendor 5.8.3 ».
- **Incertitudes :** diff fichier-par-fichier hors uplugin sans git plugin.
- **Audits :** A03.

## TF-13 — Conversion Landscape OSS = éditeur, 4 couches, refus nœuds Landscape

- **Description :** `ImportFromLandscapeWithOptions` / baker présents ; `#if WITH_EDITOR` pour l’import live. Poids max 4 ; surplus `UnmappedLayerNames`. `UsesLandscapeOnlyMaterialExpressions` refuse LayerBlend/Coords/Sample/Weight/Switch/GrassOutput/visibility.
- **Source / version :** `TerraDyneManager.h/.cpp`, `TerraDyneBaker.cpp`, OSS `270c311`.
- **Niveau de preuve :** `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` ; **non** exécuté sur l’île GF.
- **Conséquences :** ≠ conversion Store GF. M4 paint (Landscape layers) n’est pas alimenté par ces 4 canaux.
- **Incertitudes :** comportement World Partition si cellules non chargées (le code échoue sans composants).
- **Audits :** A03.

## TF-14 — Binding 8 couches = édition 0.7 annoncée, hors proto

- **Description :** Forum GregOrigin **0.7.0** (2026-10-10) : 8 canaux, MI binding, UV Landscape sur mesh, `CreateTerrainMaterialBinding`. Symboles absents de l’OSS 0.4. Fab mélange 0.7.0 / 0.5.1. Limites 0.7 annoncées : pas RVT, pas MobileWeightTextureArray, pas Material Layers Landscape.
- **Source / version :** thread UE 2699018 ; code OSS local/GitHub `NumWeightLayers = 4`.
- **Niveau de preuve :** `ANNONCÉ_PAR_LE_DÉVELOPPEUR` vs `ABSENT_DU_CODE_EXAMINÉ`.
- **Conséquences :** ne pas porter 0.7 dans l’OSS par une rustine. RVT M4 GF resterait hors binding même en 0.7.
- **Incertitudes :** code commercial non inspecté.
- **Audits :** A03.

## TF-15 — `M4UE5MasterMat` appartient au scan d’adoption Landscape OSS 0.4

- **Description :** Listing `GetExpressions()` (A04, proto `Content/M4`) : 25 expressions Landscape **directes** sur le master (`LayerBlend`, `GrassOutput`, `LayerSample` ×22, `PhysicalMaterialOutput`). `MF_LandscapeLayerBlend_UE58safe` (switch UE 5.8 **on**) contient encore `LandscapeLayerBlend`. Le MIC île **ne déclare pas** `HeightMap`/`WeightMap`. Le host charge désormais le MIC réel ; le visuel sur DynamicMesh n’est pas démontré.
- **Source / version :** AUDIT-04 ; `Results/m4_compat_selftest.log` ; `Saved/Logs/TerraDyneOSS_Proto.log` 2026-10-12 14:33 ; `TerraDyneManager.cpp` L103–118.
- **Niveau de preuve :** expressions `INSPECTÉ_DANS_LE_CODE_OU_LES_ASSETS` + listing runtime éditeur. Visuel shader `NON VALIDÉ`.
- **Conséquences :** A03-V1 clos. Campagne D (MIC incomplet) reste vraie **et** le graphe réel est landscape-only. Pas d’intégration.
- **Incertitudes :** permutation Voxel/WPO non exercée ; SLOT0 null sous `-nullrhi`.
- **Audits :** A04 (précise A03).

## TF-09 — Option A incompatible avec C-01 D1 / 08.6

- **Description :** Les contrats VALIDÉ nomment `RuntimeTerraformStore` comme unique autorité de hauteur après terraform (C-01 D1, 08.6, 00.5.S4, 00.6.A2, C-02 F1). TerraDyne OSS fait de `HeightBuffer` (couches Base/Sculpt/Detail) sa vérité interne, avec `ApplyGlobalBrush` void.
- **Source / version :** `designGate.js` 08.6 VALIDÉ / 08.7 À DÉFINIR ; `CONTRATS/C-01_TERRAIN_RUNTIME.md` ; `TerraDyneChunk.cpp` / `TerraDyneManager.cpp` OSS `270c311`.
- **Niveau de preuve :** `INCOMPATIBILITÉ_DÉMONTRÉE` au plan **contractuel** ; pas une impossibilité runtime du proto HoleFix.
- **Conséquences :** retenir l’option A exigerait un amendement humain de C-01/08.6. AUDIT-02 recommande l’option B sans adopter 08.7.
- **Incertitudes :** un wrap Store-au-dessus de TerraDyne resterait une seconde vérité si les lecteurs GF et le plugin divergent.
- **Audits :** A02.

## TF-10 — Chemin public `LoadFromData` sans preuve d’adaptateur Store

- **Description :** `ATerraDyneChunk::LoadFromData` (public) injecte `FTerraDyneChunkData` et salit mesh/collision. `RebuildPhysicsMesh` est public. `RebuildMesh` est privé. Aucun convertisseur GardenFervor Store → paquet TerraDyne n’existe.
- **Source / version :** `TerraDyneChunk.cpp` `LoadFromData` ; Store GF `4033²` / `QuadSizeUU` défaut 100 vs clamp chunk 32–256.
- **Niveau de preuve :** chemin plugin `INSPECTÉ_DANS_LE_CODE` ; adaptateur **absent** → hybride `POSSIBLE_SOUS_CONDITIONS` non démontré.
- **Conséquences :** ne pas affirmer qu’une représentation TerraDyne alimentée par GardenFervor est réalisable sans cet adaptateur. Ne pas appeler `ApplyGlobalBrush` si le Store doit rester l’autorité.
- **Incertitudes :** mapping cm ↔ normalisé/`ZScale` ; seams multi-chunks ; M4 D.
- **Audits :** A02.

## TF-11 — Pas 107,5 UU : démo TerraDyne, pas vérité Landscape GF

- **Description :** PHASE3A traitait 107,5 UU comme pas de l’île GardenFervor. C’est le pas du chunk démo TerraDyne (~13652,5/128). Le Store GF initialise `QuadSizeUU` depuis le Landscape / DataAsset (défaut code **100**). Grille prouvée **4033×4033**.
- **Source / version :** AUDIT-01 ; `GardenFervorRuntimeTerraformStore.cpp` ; `HISTORIQUE_MODIFICATIONS.md` §3.3 ; PHASE3A proto.
- **Niveau de preuve :** `INSPECTÉ_DANS_LE_CODE` + mémoire projet ; QuadSize **cooked** exact du DataAsset île non rouvert ici.
- **Conséquences :** ne pas recopier les totaux mémoire PHASE3A comme mesure GF. L’ordre de grandeur « 25 cm île entière = ~Gio » reste un **calcul**, pas un benchmark.
- **Incertitudes :** scale réel de `GardenFervorIsland`.
- **Audits :** A02.

# Démo TerraDyne + M4 (proto isolé uniquement)

GardenFervor = lecture seule. Design Gate inchangé. Plugin TerraDyne : GPU stub (`bEnableGPUBrushes=false`) et EngineVersion 5.8.0 inchangés. Reports 1A–3A non écrasés.

Ce n’est **pas** une intégration GardenFervor. Ce n’est **pas** une preuve de compatibilité M4.

## 1. Objectif et périmètre

Démo jouable **hors GardenFervor** pour évaluer TerraDyne OSS 0.4 (`270c311`) : un chunk, Raise / Lower / Paint, HUD type Terraformer. Hauteur = TerraDyne uniquement. Aucun LandscapeEdit. Peintures = couleurs de remplacement (overlay), pas les textures M4.

Map : `/Game/Demo/L_TerraDyneGardenDemo` (copie `Template_Default`).  
GameMode : `ATerraDyneGardenDemoGameMode`.  
Host : `ATerraDyneGardenDemoHost` → `ApplyGlobalBrush` / store paint local.

## 2. Verdicts distincts

| Axe | Verdict | Source |
|---|---|---|
| Empaquetage | **VALIDÉ** | UAT `BUILD SUCCESSFUL` sur les archives listées §4. |
| Démarrage graphique Shipping | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | `Packaged/Win64ShippingHoleFix/TerraDyneOSS_Proto.exe` |
| Gameplay Shipping (Raise / Lower / peintures) | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | Même build. Détail §5. |
| Compatibilité M4 | **D — incompatibilité directe** | Doc M4 + essai MIC. **Inchangé.** Indépendant du gameplay TerraDyne. |

Verdict global : **C — partielle** (démo TerraDyne fonctionnelle en Shipping ; M4 non démontrée ; pas une base d’intégration GF approuvée).

## 3. Build Shipping final

**Validé manuellement :**

`C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311\Packaged\Win64ShippingHoleFix\TerraDyneOSS_Proto.exe`  
(binaire : `...\Packaged\Win64ShippingHoleFix\TerraDyneOSS_Proto\Binaries\Win64\TerraDyneOSS_Proto-Win64-Shipping.exe`)

UAT HoleFix : `Results/shipping_holefix_uat.log` (`BUILD SUCCESSFUL`).

Archives **conservées** (comparaison, non écrasées) :

| Dossier | Rôle |
|---|---|
| `Packaged/Win64Shipping` | Témoin crash graphique (D3D12/D3D11, AV nullptr). |
| `Packaged/Win64ShippingFixed` | Premier boot GPU (ColorOverride Shipping). |
| `Packaged/Win64ShippingLowerFix` | Mesures API `-LowerSelfTest` ; creux encore masqué par le floor. |
| `Packaged/Win64ShippingHoleFix` | Floor template masqué — **build validé manuellement**. |

## 4. Fonctionnalités validées et provenance

| Fonction | Statut | Provenance |
|---|---|---|
| Empaquetage Win64 Shipping | **VALIDÉ** | UAT (516 puis 567 packages selon archive). |
| Crash graphique build d’origine | **ÉCHEC** documenté | `Results/shipping_crash_windowed.xml`, `shipping_crash_d3d11.xml`. Cause : `ColorOverrideMode::VertexColors` → matériau debug Engine `nullptr` en cooked. Correctif : matériau vertex-color cuit, override `None`. |
| Démarrage graphique HoleFix | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | Essai Shipping `Win64ShippingHoleFix`. |
| `Raise` visible | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | Même essai. |
| `Lower` sur zone intacte (sans Raise préalable) | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | Même essai. Creux sous le niveau initial **visible**. |
| `Raise` puis `Lower`, et `Lower` puis `Raise` | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | Même essai. |
| Peintures des couches exposées | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | Même essai. |
| Peinture ↔ terraformation | **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** | Même essai. |
| Overlay paint (store + RGB) | **11/11 PASS** (automatisé, `-nullrhi`) | `Results/demo_paint_selftest2.log`. Couleurs plates, pas M4. |
| Hauteur API sous T0 (`-LowerSelfTest`) | **5/5 PASS** (automatisé) | `Results/lower_selftest.txt`, `Results/lower_selftest_shipping.txt`. Mesure stock / WorldZ, pas un visuel souris. |
| Collision / marche Terraformer après Lower | **non revalidé** sur HoleFix | 1C (proto antérieur) + HUD `Collision HIT` sur un essai graphique antérieur. Pas de preuve distincte dans ce dernier essai manuel. |
| Persistance Stop/Play ou fermeture/relance | **non revalidée** ici | Relance = chunk neuf par conception. `SaveWorld` OSS non branché. |
| Performance production | **NON TESTÉ** | |
| M4 natif sur DynamicMesh | **D** | Voir §8. |

## 5. Diagnostic définitif du creusement

**Cause racine constatée : conflit visuel avec le plancher statique de la map, pas un clamp du stockage de hauteurs.**

Faits :

1. `L_TerraDyneGardenDemo` est une copie de `Template_Default`.
2. Elle contient `Floor_0`, mesh `SM_Template_Map_Floor`, à **Z = 0**.
3. Le `HeightBuffer` TerraDyne **permettait déjà** de descendre sous zéro. Lower n’était pas une opération morte.
4. `Raise` déformait le mesh **au-dessus** du plancher → bosse visible.
5. `Lower` pouvait enfoncer le maillage TerraDyne **sous** ce plancher. Le floor Engine restait affiché → le creux disparaissait à l’écran.
6. Après un `Raise`, `Lower` semblait s’arrêter au niveau du plancher : impression que le terrain ne pouvait pas être creusé sous son niveau initial.

Ce n’était **pas** :

- un Lower qui n’écrivait pas ;
- un terrain « trop bas » qu’il aurait fallu surélever artificiellement ;
- la cause racine d’un clamp `HeightBuffer` incorrect (diagnostic antérieur, **écarté** pour le défaut visuel observé).

**Correctif (build `Win64ShippingHoleFix`) :** `HideTemplateFloor()` masque uniquement `Floor*` / `SM_Template_Map_Floor` et coupe sa collision. Il ne relève pas la hauteur initiale TerraDyne, ne crée pas de bosse cachée, et ne remplace pas TerraDyne par LandscapeEdit.

Fichier : `Source/TerraDyneOSS_Proto/TerraDyneGardenDemoHost.cpp`.

## 6. Note historique (non cause racine du visuel)

Une piste clamp (`HeightBuffer = clamp(Base+Sculpt+Detail, 0, 1)`) a été explorée et un recombine `[-1, 1]` a été appliqué dans la copie isolée du plugin (`TerraDyneChunk.cpp`), avec mesures API `Win64ShippingLowerFix`.

Cette piste **n’explique pas** le comportement vu par l’utilisateur (Raise visible, Lower « bloqué » au plan initial). Le floor template le fait. Ne pas relire les PASS `-LowerSelfTest` comme une validation visuelle du creux.

## 7. Crash graphique Shipping (historique, distinct)

Build `Packaged/Win64Shipping` : AV `nullptr` à t=0, D3D12 et D3D11 ; `-nullrhi` vivait.

Cause : `SetColorOverrideMode(VertexColors)` → `GetDefaultVertexColorMaterial_RenderThread()->GetRenderProxy()` sans null-check. En cooked, `AllowDebugViewmodes()` est false ; le matériau debug Engine n’est pas chargé.

Correctif (toujours dans les builds Fixed / LowerFix / HoleFix) : matériau `VertexColorViewMode_ColorOnly` cuit via `ConstructorHelpers`, override `None`. Preuves : `Results/shipping_symbols.txt`, `shipping_crash_windowed.xml`.

Sans rapport avec M4 ni avec le floor.

## 8. Verdict M4 (indépendant)

**D — incompatibilité directe. Conservé.**

La doc officielle M4 lie la peinture aux Landscape Paint Layers. Le MIC copié (`Content/Demo/M4_Eval/Island_UE5Preset5_Inst`) n’a pas le plugin M4 / master / textures ; soft-refs `/Game/M4/...` cassées.

- Masquer le floor template **n’est pas** une preuve M4.
- Raise / Lower / peintures de remplacement **n’est pas** une preuve M4.
- Les couleurs overlay (Auto, Grass, Dirt, Sand, Mud, Forest, Stones, Jagged_Rock, Flat_Ice, Custom01, Custom02) sont un adaptateur de validation, pas l’île GardenFervor.

Aucun nouvel essai MIC n’a été fait dans cette clôture.

## 9. Limites restantes

- Collision et nav après Lower : pas de revalidation manuelle HoleFix.
- Persistance : absente (relance = neuf).
- Logs Shipping compilés hors (défaut UE).
- GPU TerraDyne : stub, `false`.
- Pas de 25 cm, seams, VHM, save chunk.
- Pas d’intégration dans GardenFervor.

## 10. Conclusion pour GardenFervor

La démo TerraDyne isolée est **apte à servir de base d’évaluation technique** : Shipping graphique, Raise, Lower sous le niveau initial, peintures de remplacement, dans `Win64ShippingHoleFix`.

Cela **n’autorise pas** une intégration TerraDyne dans GardenFervor, **n’invalide pas** Store comme autorité GF, et **ne démontre pas** M4 sur DynamicMesh. 08.7 et F1 GF restent hors de ce proto.

Design Gate : aucune modification — mission documentaire limitée au prototype TerraDyne.

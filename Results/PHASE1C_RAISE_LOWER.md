# Phase 1C — Raise / Lower T0 T1 T2

Does not overwrite PHASE1A_* / PHASE1B_HOST.md or earlier reports.

## Statut

`PASS — RAISE/LOWER ET COHÉRENCE DE BASE MESURÉS`

CPU, API de hauteur, bounds du mesh et traces de collision ont tous changé de façon mesurable après Raise, puis sont revenus à T0 après Lower. Aucun test 25 cm, seam, LOD, sauvegarde ou GPU.

## Fichiers

Modifiés (hôte uniquement) :
- `Source/TerraDyneOSS_Proto/TerraDyneOSS_ProtoGameMode.h`
- `Source/TerraDyneOSS_Proto/TerraDyneOSS_ProtoGameMode.cpp`

Ajoutés :
- `Results/compile_game_phase1c.log`
- `Results/compile_editor_phase1c.log`
- `Results/phase1c_raise_lower.log` (essai officiel)
- `Results/phase1c_raise_lower_run1_premature_lower.log` (premier essai, critère d’attente insuffisant)
- `Results/PHASE1C_RAISE_LOWER.md`

Plugin TerraDyne : aucun fichier modifié dans cette phase.
GardenFervor : inchangé (`c10126178f524432a9459cf551f12efd0175dab0`).

## API réelle utilisée

`ApplyGlobalBrush` (`TerraDyneManager.cpp` 563) :

```
void ApplyGlobalBrush(FVector WorldLocation, float Radius, float Strength,
    ETerraDyneBrushMode BrushMode,
    ETerraDyneLayer TargetLayer = Sculpt,
    int32 WeightLayerIndex = 0,
    float FlattenHeight = 0.f);
```

- Fonction `void` : aucun booléen de succès. Le résultat est la variation CPU mesurée.
- Les chunks sont sélectionnés par `WorldToChunkCoord` : `Floor((LocalAxis + GlobalChunkSize * 0.5) / GlobalChunkSize)`. Avec manager à l’origine et `GlobalChunkSize = 1000`, le point `(0,0,0)` appartient au chunk `(0,0)`.
- `RelativePos = WorldLocation - Chunk->GetActorLocation()`.
- `ApplyLocalIdempotentEdit` refuse si `RelativePos` est hors `±HalfSize + Radius`.
- GPU : si `bUseGPU` et mode ≠ Paint, `ApplyBrushGPU` est tenté. Ici `IsUsingGPU() = false` (`bEnableGPUBrushes=false`, `-nullrhi` → `CanEverRender=false`). Le stub `DispatchTerraDyneBrushSimulation` n’est pas atteint.
- Raise/Lower CPU : `ApplyBrushCPUInternal` écrit le buffer de couche (Sculpt). `Strength` est en UU monde. `StrengthNorm = Strength / ZScale`. Falloff smoothstep sur le disque `Radius`.
- Après édition : `HeightBuffer[i] = clamp(Base + Sculpt + Detail, 0, 1)`, `bMeshDirty = true`, `CollisionDebounceTimer = CollisionDebounceTime` (défaut 0.2 s).
- Mesh : `Tick` lance `StartAsyncMeshBuild`, puis `ApplyPendingMeshBuild`. `bMeshDirty` est privé. Mesure publique : `UDynamicMeshComponent::GetLocalBounds()` (`FBoxSphereBounds`).
- Collision : après mesh appliqué et debounce, `UpdateCollision()` → `DynamicMeshComp->UpdateCollision(true)` (`bUseAsyncCooking = true`). Mesure publique : `LineTraceComponent` + `LineTraceSingleByChannel(ECC_Visibility)`.
- `GetHeightAtLocation(LocalPos)` : interpolation triangle-aware de `HeightBuffer` × `ZScale`.
- `GetWorldHeightAtLocation(World)` : même valeur + `Chunk->GetActorLocation().Z`.
- `HeightBuffer` est un membre public. Lecture hôte légale, sans accès privé.

## Paramètres effectifs

| Paramètre | Valeur | Source |
|---|---|---|
| Carte | `/Engine/Maps/Templates/OpenWorld` | même procédure que 1A/1B |
| Chunk | `(0,0)`, `WorldSize = 1000` UU, `Resolution = 32` | Phase 1B |
| `ZScale` | `3000` | constructeur chunk |
| GPU | `false` | settings + `-nullrhi` |
| Centre monde | `(0, 0, 0)` | origine du chunk |
| Voisin monde | `(32.258064, 0, 0)` | un pas de grille = `1000 / 31` |
| Index CPU centre | `528` = `(X=16, Y=16)` | même formule que le pinceau CPU |
| Index CPU voisin | `529` = `(X=17, Y=16)` | |
| Mode T1 | `Raise`, couche `Sculpt` | |
| Mode T2 | `Lower`, même centre / rayon / force | |
| Rayon | `150` UU | ~4.65 pas de grille |
| Force | `80` UU | `StrengthNorm = 80 / 3000 ≈ 0.026667` |

Le sommet `(16,16)` n’est pas exactement à `(0,0)` : position locale `(16.129, 16.129)`, distance `≈ 22.81` UU. Le falloff smoothstep réduit le delta. Valeur CPU observée `0.02500428` × `ZScale` = `75.012856` UU, cohérente avec cette géométrie. Ce n’est pas un échec.

## Ordre d’initialisation puis T0/T1/T2

1. Spawn différé manager + chunk, `InitializeChunk((0,0), 1000, 32, null, null)` — inchangé depuis 1B.
2. `Tick` : attendre un signal public de surface initiale (`MeshExtentX > 1` et hit `LineTraceComponent`).
3. T0.
4. Un seul `ApplyGlobalBrush(Raise)`.
5. Mesure immédiate CPU / API (mesh et collision encore à T0).
6. `Tick` : attendre un changement public des bounds Z, puis un changement numérique de `CompZ` (pas un simple miss→hit).
7. Un seul `ApplyGlobalBrush(Lower)` seulement si le CPU a changé.
8. Même attente mesh / collision.
9. `QUIT`.

Aucun appel à `RebuildPhysicsMesh` / `UpdateCollision` / membres privés.

## Compilation

- Game Win64 Development : Succeeded (première passe : `C2440` `GetLocalBounds` → `FBox` ; corrigé en `FBoxSphereBounds` côté hôte uniquement). Relink Succeeded.
- Editor Win64 Development : Succeeded (requis pour `UnrealEditor-Cmd -game`).

## Exécution

Commande (identique à 1A/1B) :

```
UnrealEditor-Cmd.exe TerraDyneOSS_Proto.uproject /Engine/Maps/Templates/OpenWorld
  -game -unattended -nopause -nosplash -nullrhi
  -AbsLog=Results/phase1c_raise_lower.log
```

Module : `TerraDyne Runtime Module Started.` / `Chunk [0,0]: Ready (GPU: NO)`.
Game class : `TerraDyneOSS_ProtoGameMode`.
`bEnableGPUBrushes=false`, `CanEverRender=false`, `CollisionDebounceTime=0.200`.

### Premier essai (non officiel)

`phase1c_raise_lower_run1_premature_lower.log`

T0 a été pris avant que la collision du chunk existe (`CompTraceHit=false`, hit monde = Landscape OpenWorld à Z=0). Le premier hit composant à Z=0 a été compté à tort comme collision T1. Lower a été envoyé ~1 ms après la mise à jour mesh, avant le debounce 0.2 s. `CompZ` est resté 0. Critère d’attente corrigé (hit T0 requis ; collision = variation numérique de Z). Ce n’est pas un échec plugin.

### Essai officiel

`phase1c_raise_lower.log`

#### T0 — état initial (`t = 0.2798 s`)

| Couche | Centre | Voisin | Méthode |
|---|---|---|---|
| CPU `HeightBuffer` | `0.00000000` | `0.00000000` | membre public |
| API locale | `0.000000` | `0.000000` | `GetHeightAtLocation` |
| API monde | `0.000000` | `0.000000` | `GetWorldHeightAtLocation` |
| Mesh bounds Z | `[0.000000, 0.000000]` | ExtentX `500` | `GetLocalBounds` |
| Collision composant | hit `true`, Z `0.000000`, `DynamicMeshComp` | | `LineTraceComponent` |
| Collision monde | hit `true`, Z `0.000000`, `TerraDyneChunk_0` | | `ECC_Visibility` |

#### T1 — Raise immédiat (`t = 0.2805 s`)

`ApplyGlobalBrush` void. GPU `false`.

| Couche | Centre | Voisin | Δ vs T0 |
|---|---|---|---|
| CPU | `0.02500428` | `0.01951381` | `+0.02500428` / `+0.01951381` |
| API locale | `75.012856` | `66.777145` | `+75.012856` / `+66.777145` |
| API monde | `75.012856` | `66.777145` | identique (chunk Z = 0) |
| Mesh MaxZ | `0.000000` | | pas encore reconstruit |
| CompZ | `0.000000` | | pas encore mise à jour |

CPU et API changent dans le même appel. Mesh et collision restent à T0.

#### T1 — différé

- Mesh : `t = 0.2814 s`, MaxZ `0.000000 → 75.012856` (≈ 0.9 ms après le pinceau, 2 frames).
- Collision : `t = 0.4513 s`, CompZ `0.000000 → 75.012856` (≈ 170 ms après le mesh, cohérent avec debounce 0.2 s + cook async). Hit monde : `TerraDyneChunk_0` Z `75.012856`.

#### T2 — Lower immédiat (`t = 0.4521 s`)

Même centre, rayon 150, force 80.

| Couche | Centre | Δ vs T1 immédiat | vs T0 |
|---|---|---|---|
| CPU | `0.00000000` | `-0.02500428` | `0` |
| API locale / monde | `0.000000` | `-75.012856` | `0` |
| Mesh MaxZ | `75.012856` | encore T1 | |
| CompZ | `75.012856` | encore T1 | |

#### T2 — différé

- Mesh : `t = 0.4533 s`, MaxZ `75.012856 → 0.000000`.
- Collision : `t = 0.6226 s`, CompZ `75.012856 → 0.000000` (≈ 169 ms après le mesh). Hit monde : `TerraDyneChunk_0` Z `0.000000`.

Lower a ramené CPU, API, mesh et collision à T0. Ce retour exact est dû à un terrain initial nul et à deux strokes opposés identiques. Il n’est pas généralisé à un terrain déjà sculpté.

## Conclusion par couche

1. **Données CPU** — Confirmé. Raise augmente `HeightBuffer` ; Lower l’annule sur ce champ nul.
2. **API de hauteur** — Confirmé. `GetHeightAtLocation` / `GetWorldHeightAtLocation` = échantillon interpolé × `ZScale`. Même valeur monde car le chunk est à Z = 0.
3. **Mesh** — Confirmé par bounds publics. `MaxZ` passe à `75.012856` puis revient à `0`, égal à l’API au centre. Pas de lecture sommet par sommet (`GetTriangleCount` exclu : LNK2019 Phase 1B).
4. **Collision** — Confirmé séparément. `LineTraceComponent` et la trace monde frappent `TerraDyneChunk_0` à Z = 0, puis 75.012856, puis 0, après le debounce réel. La présence du composant en 1B ne suffisait pas.

## Avertissements restants (déjà connus)

- CDO `TerraDyneEditController` : `/TerraDyne/Materials/M_TerraDyne_BrushPreview` absent (OSS strippé).
- `TerraDyne shader directory was not found` (chemin GPU non restauré).
- Ces messages n’ont pas empêché le chemin CPU.

## Limites de mesure

- Lancement `-nullrhi` : pas de preuve de rendu GPU / matériau.
- Mesh : bounds Z, pas une comparaison sommet à sommet.
- Collision : traces au centre XY seulement. Pas de validation Chaos packagée / Shipping.
- Carte OpenWorld : un Landscape existe. Sur l’essai officiel, après surface initiale, le premier hit monde est le chunk, pas le Landscape.
- Seuil d’égalité : `KINDA_SMALL_NUMBER` uniquement pour détecter un changement, pas pour déclarer une tolérance métier.
- Un seul chunk 10 m, résolution 32. Pas de 25 cm, pas de frontière, pas de LOD, pas de save/load.
- `ApplyGlobalBrush` ne retourne rien ; le succès est la mesure, pas un booléen.

## Étape suivante recommandée (non exécutée)

Ne pas enchaîner sur 25 cm sans nouvelle autorisation.

Prochaine campagne utile, si autorisée : mesurer le plus petit rayon/force encore visible sur ce chunk (pas encore une grille 25 cm), ou répéter Raise/Lower avec collision déjà chaude sans Landscape OpenWorld. Ne pas traiter ce PASS comme une adoption TerraDyne ni comme une preuve F1.

Design Gate : aucun statut modifié. F1 reste INVALIDÉ et 08.7 reste À DÉFINIR. Aucun contrat ni choix architectural n’est validé par cette campagne.

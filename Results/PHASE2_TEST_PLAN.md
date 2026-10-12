# Phase 2 — plan de test : précision spatiale et résolution

Document de préparation uniquement. Aucun test runtime, aucune compilation, aucune modification de code n’a été effectuée pour produire ce plan.

Prototype : `C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311`  
OSS : `270c31117e20e491ef5333dd7e2f9dc133fcdc38`  
Référence mesurée : Phase 1C `PASS — RAISE/LOWER ET COHÉRENCE DE BASE MESURÉS` (chunk 1000 UU, résolution 32).

**Ce document attend une autorisation humaine avant toute exécution (phase 2B).**

## 1. Sources inspectées

Rapports :
- `Results/PHASE1B_HOST.md`
- `Results/PHASE1C_RAISE_LOWER.md`
- `Results/phase1c_raise_lower.log`
- `Results/phase1c_raise_lower_run1_premature_lower.log`
- `Results/ADAPTATIONS.md`
- `Results/COMPILE_AFTER_ADAPT.md`

Code OSS réellement présent (pas le manuel Fab) :
- `TerraDyneChunk.cpp` : `Initialize`, `InitializeChunk`, `ApplyBrushCPUInternal`, `ApplyLocalIdempotentEdit`, `GetHeightAtLocation`, `BuildTerrainSurfaceSamples`, `Tick`, `UpdateCollision`, `UpdateLOD`
- `TerraDyneManager.cpp` : `ApplyGlobalBrush`, `ApplyAuthorizedBrush`, `WorldToChunkCoord` / `ChunkIndexFromLocalAxis`, `SpawnConfiguredChunk`, `Tick` (LOD + streaming)
- `TerraDyneChunk.h` : `Resolution`, `WorldSize`, `HeightBuffer` public
- `TerraDyneSettings.h/.cpp` : clamps UPROPERTY, `CollisionDebounceTime`, `LODDistanceThreshold`, `MaxBrushRadius`
- Hôte actuel : `TerraDyneOSS_ProtoGameMode.cpp` (valeurs 1000 / 32 codées en dur)

Aucune source Fab n’est utilisée comme preuve.

## 2. Résolution : signification confirmée dans le code

`Resolution` est le **nombre de sommets par côté**, pas le nombre d’intervalles.

Preuves dans le chemin réellement exécuté :
- `InitializeChunk` appelle `Initialize(FMath::Clamp(InRes, 32, 256), Size)` puis `HeightBuffer.SetNumZeroed(Resolution * Resolution)`.
- `BuildTerrainSurfaceSamples` / `HasTerrainMeshTopology` : `ExpectedVertices = Resolution * Resolution`, `ExpectedTriangles = 2 * (Resolution - 1) * (Resolution - 1)`.
- Espacement : `Step = WorldSize / (Resolution - 1)`.
- Sommet `(X, Y)` en local chunk : `(-HalfSize + X * Step, -HalfSize + Y * Step)` avec `HalfSize = WorldSize * 0.5`.
- Index CPU : `Y * Resolution + X`.

Donc :
- `N` sommets ⇒ `N - 1` intervalles.
- Un chunk de côté `S` a un pas `S / (N - 1)`.

`Initialize` (sans clamp) n’est appelé que via `InitializeChunk` ou `BeginPlay` (Base/Sculpt seulement). L’hôte 1B/1C appelle `InitializeChunk`. C’est ce clamp qui compte.

## 3. Limites de résolution confirmées

| Source | Limite | Nature |
|---|---|---|
| `InitializeChunk` | `FMath::Clamp(InRes, 32, 256)` | **Runtime, chemin hôte** |
| `SpawnConfiguredChunk` | même clamp 32–256 | Runtime, privé, non utilisé par l’hôte |
| `UTerraDyneSettings::DefaultResolution` | meta `ClampMin=32`, `ClampMax=1024` | Métadonnée éditeur seulement |
| Constructeur chunk | `Resolution = 128` | Défaut avant init |

**Le clamp exécuté est 32–256, pas 1024.** Une valeur 1024 saisie dans les settings serait ramenée à 256 si `SpawnConfiguredChunk` était utilisé. L’hôte passe par `InitializeChunk` : 41, 40 et 127 sont tous dans [32, 256].

Aucune autre contrainte d’imparité / puissance de deux n’apparaît dans ces fonctions. 40, 41 et 127 sont acceptés tels quels.

`WorldSize` / `GlobalChunkSize` : aucun clamp runtime dans `InitializeChunk`. La meta settings `DefaultChunkSize` a `ClampMin=100` (éditeur). 1000 et 13545 UU sont acceptables à l’init.

`GetHeightAtLocation` exige `Resolution >= 2` et `HeightBuffer.Num() == Resolution * Resolution`.

## 4. Formules vérifiées

Unités : 1 UU Unreal = 1 cm. Phase 1C a traité 1000 UU comme un chunk de 10 m. Conservé ici. Ce n’est pas un résultat expérimental nouveau.

### Grille

```
Step      = WorldSize / (Resolution - 1)
HalfSize  = WorldSize * 0.5
VertexXY  = (-HalfSize + X * Step, -HalfSize + Y * Step)
Index     = Y * Resolution + X
```

`GlobalChunkSize` du manager doit être égal au `WorldSize` du chunk. `WorldToChunkCoord` fait `Floor((LocalAxis + GlobalChunkSize * 0.5) / GlobalChunkSize)`. Un décalage manager/chunk ferait rater le chunk.

Le chunk `(0,0)` est centré sur l’acteur, donc sur l’origine si l’acteur est à `(0,0,0)`. Le centre monde `(0,0,0)` est un sommet seulement si `Resolution` est impair (`(Resolution-1)/2` entier).

### Pinceau CPU (`ApplyBrushCPUInternal`)

```
CenterX/Y     = RoundToInt((LocalPos + HalfSize) / Step)
RadiusInPixels = CeilToInt(Radius / Step)
Fenêtre       = [Center ± RadiusInPixels] clampée à [0, Resolution-1]
Inclusion     = DistSq(vertex, LocalPos.XY) <= Radius * Radius
FalloffRaw    = 1 - Dist / Radius
Falloff       = FalloffRaw² * (3 - 2 * FalloffRaw)   // smoothstep
StrengthNorm  = Strength / ZScale                     // ZScale défaut = 3000
Delta         = StrengthNorm * Falloff
Couche        = clamp(couche + Delta, -1, 2)          // ici Sculpt
HeightBuffer  = clamp(Base + Sculpt + Detail, 0, 1)
API UU        = HeightBuffer interpolé * ZScale
```

`ApplyGlobalBrush.Radius` est un **rayon** monde en UU, pas un diamètre.  
`Strength` est un delta de hauteur monde en UU, pas une intensité normalisée.

`ApplyGlobalBrush` est `void` et n’a pas de rayon minimal.  
`ApplyAuthorizedBrush` refuse `Radius <= 0` ou `Radius > MaxBrushRadius` (défaut 10000). Le protocole doit continuer d’appeler `ApplyGlobalBrush`, comme en 1C.

### Un pinceau peut ne modifier aucun sommet

Oui. Si aucun sommet n’est dans le disque, `ModifiedPixels = 0`.

Point critique : `ApplyLocalIdempotentEdit` pose `bModifiedHeight = true` pour Raise/Lower **même si zéro sommet a bougé**. Le mesh et la collision peuvent donc être marqués dirty sans variation CPU. Le protocole doit compter les deltas `HeightBuffer`, pas l’appel `void` ni le dirty interne.

À `Dist == Radius`, le sommet est inclus (`DistSq <= Radius²`) mais `Falloff = 0` ⇒ `Delta = 0`. Comptage « modifié » possible avec variation nulle.

### Mesh et collision aux résolutions plus fines

Aucun chemin distinct selon la résolution dans le code inspecté. Même pipeline qu’en 1C :
- mesh async via `Tick` ;
- collision après mesh + `CollisionDebounceTime` (0.2 s) + cook async ;
- sommets mesh `Height * ZScale` ;
- `GetHeightAtLocation` interpolé triangle-aware (même winding que le mesh).

`UpdateLOD` : si la distance au viewer > `LODDistanceThreshold` (50000 UU), collision désactivée. Sans pion, le manager utilise `(0,0,0)`. Un chunk à l’origine reste à distance 0. **LOD n’est pas indispensable à interpréter A/B/C si le chunk reste à l’origine.** À signaler seulement si un test éloigne le viewer.

Streaming : `UpdateStreaming` peut **créer** d’autres chunks dans le rayon de load (défaut 5). `ApplyGlobalBrush` itère les coords couvertes par `Radius + GlobalChunkSize * 0.5`. Même un rayon de 12.5 UU à l’origine interroge les coords voisines. Si un chunk extra existe, il peut être touché. **`bStreamingPaused = true` (propriété publique) est nécessaire à l’isolement.** Pas un test de streaming.

### Mémoire visible dans le code (ordre de grandeur, non mesurée)

Par chunk, tableaux `float` CPU (Base, Sculpt, Detail, Height, SmoothScratch, 4 poids) ≈ `9 * Resolution² * 4` octets, plus le `DynamicMesh` (`Resolution²` sommets, `2*(Resolution-1)²` triangles).

| Config | Samples | Buffers CPU approx. | Triangles |
|---|---|---|---|
| A 41² | 1681 | ~60 Ko | 3200 |
| B 40² | 1600 | ~58 Ko | 3042 |
| C 127² | 16129 | ~580 Ko | 31752 |
| Max runtime 256² | 65536 | ~2,3 Mo | 130050 |

`MaxUncompressedChunkStateBytes` = 64 Mo : C est loin du plafond. Aucune limite mémoire bloquante n’apparaît pour A/B/C. Ce n’est pas une mesure runtime.

## 5. Configurations A / B / C

Convention UU : 1 UU = 1 cm. Les espacements « 25 cm » ci-dessous sont **théoriques**.

### A — terrain local fin

| Paramètre | Valeur | Accepté par le code ? |
|---|---|---|
| `WorldSize` / `GlobalChunkSize` | 1000 UU | oui, pas de clamp runtime |
| `Resolution` demandée | 41 | oui, `Clamp(41, 32, 256) = 41` |
| Intervalles | 40 | |
| `Step` | `1000 / 40 = 25` UU = 25 cm | |
| Sommet au centre `(0,0)` | oui (41 impair) | index `(20,20)` = 840 |
| `HeightBuffer.Num()` attendu | 1681 | |

**A est cohérente avec l’API.** 41 sommets / 40 intervalles / 25 cm est la lecture correcte de `Step = WorldSize / (Resolution - 1)`.

### B — variante 40 sommets

| Paramètre | Valeur | Accepté ? |
|---|---|---|
| Côté | 1000 UU | oui |
| `Resolution` | 40 | oui |
| Intervalles | 39 | |
| `Step` | `1000 / 39 ≈ 25.6410256` UU ≈ 25,641 cm | |
| Sommet au centre | **non** | `(0,0)` est le centre du quad des 4 sommets `(19|20, 19|20)` |
| `HeightBuffer.Num()` | 1600 | |

**B est acceptée.** Ce n’est pas un espacement de 25 cm. Un pinceau au centre du chunk n’est **pas** un pinceau « sur sommet ».

### C — dimensions analogiques GardenFervor (générées, pas importées)

| Paramètre | Valeur | Accepté ? |
|---|---|---|
| Côté | 13545 UU | oui (pas de clamp `InitializeChunk`) |
| `Resolution` | 127 | oui |
| Intervalles | 126 | |
| `Step` | `13545 / 126 = 107.5` UU = 107,5 cm | |
| Sommet au centre | oui (127 impair) | index `(63,63)` = 8064 |
| `HeightBuffer.Num()` | 16129 | |

**C est dimensionnellement cohérente avec l’API TerraDyne.** Ce n’est **pas** un import Landscape GardenFervor, ni une preuve que des données GF et des données générées sont équivalentes. Terrain initial prévu : zéros (`bEnableSeededOutskirts = false`), comme en 1C.

Incohérences à ne pas commettre :
- confondre 41 sommets et 41 intervalles ;
- croire que 40 sommets donnent 25 cm ;
- croire que le clamp settings 1024 s’applique à `InitializeChunk` ;
- placer B « sur sommet » en visant `(0,0,0)`.

## 6. Protocole de pinceaux (non exécuté)

### Paramètres communs

- GPU : `bEnableGPUBrushes=false`, `-nullrhi` (même procédure que 1C).
- `ZScale = 3000` (constructeur).
- `Strength = 80` UU (même sémantique qu’en 1C : `StrengthNorm = 80/3000`). Sur un sommet exact, falloff 1 ⇒ CPU `≈ 0.02666667`, API `≈ 80` UU. C’est un **choix de protocole**, pas une exigence du plugin.
- Mode : un seul `Raise` par cas, couche `Sculpt`.
- Pas de Smooth / Flatten / Paint.
- `bStreamingPaused = true` avant toute édition.
- `bSpawnDefaultChunksOnBeginPlay = false`.
- Attente mesh / collision : mêmes signaux publics qu’en 1C (bounds Z + `LineTraceComponent` avec variation numérique de Z). Ne pas traiter un délai fixe comme preuve.
- Terrain initial : zéros, T0 enregistré.
- Après chaque cas : un `Lower` identique pour revenir à T0 **si et seulement si** le Raise a produit un delta CPU, comme en 1C. Sinon, ne pas Lower.

### Rayons (diamètre demandé → rayon code)

| Intention | Diamètre | `Radius` passé à `ApplyGlobalBrush` |
|---|---|---|
| 25 cm | 25 UU | **12.5 UU** |
| 2 m | 200 UU | **100 UU** |

### Positions

Pour chaque config, calculer les coordonnées avec la formule de grille, pas à l’œil.

**Sur sommet**
- A : monde `(0, 0, 0)` = sommet `(20,20)`.
- B : sommet le plus proche du centre, ex. local `(-500 + 20 * StepB, -500 + 20 * StepB)` ≈ `(12.8205, 12.8205)`. Ne pas utiliser `(0,0,0)` comme « sommet » pour B.
- C : monde `(0, 0, 0)` = sommet `(63,63)`.

**Entre sommets (centre de quad)**
- A : local `(12.5, 12.5)` = milieu du quad `(20,20)–(21,21)`.
- B : monde `(0, 0, 0)` est déjà un centre de quad. L’utiliser.
- C : local `(53.75, 53.75)` = demi-pas.

Option utile, non obligatoire : **milieu d’arête** A à local `(12.5, 0)` pour observer `Dist == Radius` ⇒ inclusion, `Delta = 0`.

### Cas à exécuter (12 minimum)

Pour A, B, C :
1. Raise 25 cm sur sommet.
2. Raise 25 cm entre sommets.
3. Raise 2 m sur sommet.
4. Raise 2 m entre sommets.

Ordre : A puis B puis C, ou trois lancements séparés. Un seul chunk actif à la fois. Recréer le chunk (destroy + `InitializeChunk`) entre configurations pour changer `GlobalChunkSize` / `Resolution`.

### Attentes théoriques (prédictions, pas des résultats)

Soit `R25 = 12.5`, `R200 = 100`.

**A — Step = 25**
- 25 cm sur sommet : 1 sommet, Dist 0, ΔCPU max `80/3000`, API au sommet `80`.
- 25 cm centre de quad : distance aux 4 sommets `12.5 * √2 ≈ 17.678 > 12.5` ⇒ **0 sommet**. CPU inchangé. API 0. Mesh/collision ne doivent pas changer de hauteur.
- 2 m sur sommet : disque r=100, ~4 pas. Plusieurs sommets (ordre de grandeur : fenêtre `Ceil(100/25)=4` ⇒ jusqu’à 9×9 testés, sous-ensemble dans le disque). Δ max au centre = `80`.
- 2 m centre de quad : 4 sommets proches à 17.678 < 100, plus d’autres. Δ max < 80 (aucun sommet à Dist 0).

**B — Step ≈ 25.641**
- 25 cm sur un vrai sommet : 1 sommet, même Δ qu’A.
- 25 cm en `(0,0)` (quad) : distance aux 4 sommets ≈ `12.8205 * √2 ≈ 18.13 > 12.5` ⇒ **0 sommet**.
- 2 m : analogue à A, pas légèrement plus large.

**C — Step = 107.5**
- 25 cm sur sommet : 1 sommet (12.5 ≪ 107.5).
- 25 cm centre de quad : distance `53.75 * √2 ≈ 76.01 > 12.5` ⇒ **0 sommet**.
- 2 m sur sommet : 1 sommet (100 < 107.5). Les voisins sont à 107.5 > 100.
- 2 m centre de quad : 4 sommets à ≈ 76.01 < 100 ⇒ **4 sommets**, falloff faible.

Interprétation : un diamètre de 25 cm n’est « spatialement résolu » que si la grille a un pas ≤ 25 cm **et** si le centre tombe assez près d’un sommet. Sur C (107,5 cm), le pinceau 25 cm ne peut toucher qu’un sommet déjà visé ; il ne sculpte pas un disque de 25 cm. Sur A, le même pinceau entre sommets peut ne rien faire.

### Mesures par cas

Immédiat après `ApplyGlobalBrush` :
- `HeightBuffer.Num()` vs `Resolution²`
- liste des index dont `|H - T0| > KINDA_SMALL_NUMBER` (égalité machine, pas tolérance métier)
- `max |ΔCPU|` et index
- API locale / monde au point de pinceau, au sommet le plus proche, au centre de quad
- `IsUsingGPU() == false`

Après mise à jour effective (signaux publics, comme 1C) :
- `GetLocalBounds` MinZ / MaxZ
- `LineTraceComponent` au point de pinceau et au sommet le plus proche
- trace monde `ECC_Visibility` (peut frapper le Landscape OpenWorld ; ne pas s’y fier seul)

Ne pas déclarer un cas réussi sur le seul `void` ou sur un mesh dirty sans delta CPU.

### Critères d’interprétation

| Lecture | Condition |
|---|---|
| CPU atteint le sommet visé | 25 cm sur sommet : exactement 1 index changé, Δ cohérent avec `80/ZScale` |
| CPU rate l’inter-sommet | 25 cm centre de quad A/B/C : 0 index changé |
| 2 m plus large que 25 cm | plus d’index changés que le cas 25 cm correspondant, sauf C-sur-sommet (1 dans les deux cas) |
| API cohérente | `GetHeightAtLocation` = interpolation × `ZScale` des samples mesurés |
| Mesh cohérent | MaxZ suit le max API/CPU au sommet modifié, après apply async |
| Collision cohérente | CompZ au sommet modifié suit la même hauteur après debounce |
| Échec de précision 25 cm | 25 cm sur sommet A : 0 delta, ou plus d’un sommet, ou delta sans falloff 1 |
| PARTIEL | CPU OK, mesh ou collision sans variation alors qu’un sommet a changé |
| Hors scope | seams, LOD, save/load, GPU, perf, import GF |

Ne pas inventer de tolérance en centimètres. Rapporter les valeurs brutes.

## 7. Risques et limites du protocole

- OpenWorld contient un Landscape. Traces monde T0 peuvent le frapper (vu en 1C run1). Utiliser `LineTraceComponent`.
- Streaming peut spawn des chunks extra si `bStreamingPaused` n’est pas posé.
- Raise/Lower avec 0 sommet dirty quand même le mesh : faux positif mesh si on ne compare pas les hauteurs.
- `-nullrhi` : pas de preuve visuelle. Bounds + traces seulement.
- Mesh : pas de lecture sommet à sommet (LNK2019 `GetTriangleCount` en 1B). Bounds Z suffisent si un seul sommet s’élève de 80 UU.
- Collision : cook async ; attendre une variation de Z, pas un premier hit.
- C plus grand : traces depuis Z=10000 restent valides (`ZScale` 3000). Incertain : recouvrement Landscape sur ±6772 UU. À consigner, pas à « corriger » par le plugin.
- Unités 1 UU = 1 cm : convention UE / 1C, pas une métadonnée TerraDyne.
- Ce plan n’évalue pas un terrain importé GardenFervor.

Fonctions **non** nécessaires à l’interprétation : seams, save/load, GPU, perf.  
LOD : non nécessaire si viewer à l’origine.  
Streaming : pas un objet de test, mais **pause obligatoire** pour l’isolement.

## 8. Modifications de code qui seraient nécessaires — non appliquées

Plugin TerraDyne : **aucune**.

GardenFervor : **aucune**.

Hôte seulement, si le plan est autorisé :

1. `TerraDyneOSS_ProtoGameMode.h` — remplacer les constantes 1000/32 par une petite structure de config (`WorldSize`, `Resolution`, libellé A/B/C) et une liste de cas (rayon, point monde, nom).
2. `TerraDyneOSS_ProtoGameMode.cpp` — pour chaque config : destroy/respawn ou ré-init du chunk ; `Manager->GlobalChunkSize = WorldSize` ; `InitializeChunk(coord, WorldSize, Resolution, nullptr, nullptr)` ; vérifier `Chunk->Resolution` **après** clamp et `HeightBuffer.Num() == Resolution * Resolution`.
3. Poser `Manager->bStreamingPaused = true`.
4. Calculer les positions sommet / inter-sommet avec `Step = WorldSize / (Resolution - 1)`.
5. Pour chaque cas : T0 buffer complet ou au moins la fenêtre `Center ± Ceil(Radius/Step)` ; Raise ; compter les index changés ; logger min/max index et Δ max ; attendre mesh/collision ; Lower si delta CPU.
6. Logs `PHASE2` par config/cas, sans log permanent excessif.
7. `DefaultGame.ini` : `bEnableGPUBrushes=false` déjà présent — pas de changement prévu.
8. `DefaultEngine.ini` : GameMode déjà posé — pas de changement prévu.
9. Compilation Game + Editor, puis `UnrealEditor-Cmd` OpenWorld `-game -unattended -nullrhi`, log `Results/phase2_precision.log`, rapport `Results/PHASE2_PRECISION.md`.

Ne pas implémenter ces points avant autorisation.

## 9. Constats confirmés (préparation)

- `Resolution` = sommets par côté ; pas = `WorldSize / (Resolution - 1)`.
- Clamp runtime réel : **32–256**. 40, 41 et 127 passent. Le 1024 des settings n’est pas le clamp exécuté.
- A (41 / 1000) donne théoriquement 25 cm. B (40 / 1000) donne ≈ 25,641 cm et n’a pas de sommet à l’origine.
- C (127 / 13545) donne 107,5 cm. C’est une analogie dimensionnelle, pas un import GF.
- `ApplyGlobalBrush.Radius` est un rayon UU. 25 cm de diamètre ⇒ `Radius = 12.5`. 2 m ⇒ `Radius = 100`.
- Un pinceau plus petit que la distance au sommet le plus proche ne change aucun sample. Raise dirty quand même le mesh.
- Force = UU monde / `ZScale`. Reprendre `Strength = 80` pour rester comparable à 1C.
- Mesh et collision restent asynchrones (≈ 1 ms / ≈ 170 ms observés en 1C). Même méthode de poll public.
- Mémoire A/B/C : largement sous les plafonds visibles. Non mesurée.

**Le protocole n’a pas été exécuté. Aucune capacité 25 cm n’est démontrée. La phase 2B attend une autorisation humaine explicite.**

Design Gate : aucun statut modifié. F1 reste INVALIDÉ et 08.7 reste À DÉFINIR. Cette préparation ne valide aucune capacité supplémentaire ni aucun choix architectural.

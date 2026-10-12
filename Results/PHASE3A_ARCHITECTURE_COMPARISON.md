# Phase 3A — comparaison d’architectures de terrain

Étude documentaire. Aucun build, aucun runtime, aucun changement de code, de plugin, de GardenFervor ou du Design Gate.

Prototype inspecté : `TerraDyneOSS_Proto_270c311`  
OSS : `270c31117e20e491ef5333dd7e2f9dc133fcdc38`  
GardenFervor : lecture seule du Design Gate pour les décisions déjà VALIDÉES. Aucune écriture.

Légende des affirmations :
- **[TEST]** établi par les phases 1–2 du prototype isolé
- **[CODE]** vérifié dans le source OSS présent
- **[DG]** décision GardenFervor déjà VALIDÉE (principe, pas une implémentation)
- **[CALC]** calcul théorique à partir des dimensions données
- **[HYP]** hypothèse d’architecture, non démontrée
- **[OUVERT]** question pour un test ou un arbitrage ultérieur

## 1. Résumé exécutif

Les phases 1–2 montrent qu’un chunk TerraDyne OSS CPU peut être initialisé, que Raise/Lower changent `HeightBuffer`, l’API, le mesh et la collision, et qu’un pinceau n’affecte que les **sommets dans le disque**. Un diamètre de 25 cm n’est pas un disque continu : entre sommets, y compris sur une grille à 25 cm, le CPU peut rester à zéro. **[TEST]**

Sur le Landscape GardenFervor (4033×4033, pas 107,5 UU), le même pinceau de 25 cm ne peut toucher qu’un sommet déjà visé. Un pinceau de 2 m sur cette grille ne s’élargit pas au-delà d’un sommet s’il est centré dessus (`100 < 107,5`). **[TEST]** (config C)

Une grille uniforme à 25 cm sur toute l’île multiplie les sommets par ≈ **18,5** (ordre **3,0×10⁸**). Un seul buffer `float32` ferait ≈ **1,1 Gio** hors mesh, collision, GPU et persistance. Ce n’est pas un benchmark. **[CALC]**

TerraDyne OSS n’a pas de raffinement local, pas de mesh adaptatif, un clamp **32–256** sommets par chunk, une résolution **uniforme par chunk**. **[CODE]** Ce n’est pas le produit Fab.

Le Design Gate dit déjà : après terraformation, le Store `Base+Delta` est l’autorité de hauteur ; Landscape = base initiale ; 08.7 reste À DÉFINIR. **[DG]** Faire de TerraDyne l’autorité entrerait en conflit avec 08.6 / 00.5.S4 / C-01 D1.

**Recommandation technique (non validée)** : ne pas adopter TerraDyne comme autorité ; ne pas lancer une grille uniforme 25 cm île entière comme première architecture. La famille la plus cohérente avec 08.6 et avec 2B est **B — base actuelle + détail local**, à condition qu’un seul store GardenFervor reste l’autorité et que les raccords soient conçus et prouvés. Confiance : **moyenne**. Elle baisserait si l’on décide que 25 cm n’est pas requis, ou si une preuve montre qu’un pinceau « un sommet » à 107,5 cm suffit au gameplay.

Aucune architecture n’est retenue. 08.7 reste ouvert.

## 2. Faits établis et limites des phases 1–2

### Établis [TEST]

| Fait | Source |
|---|---|
| Module chargeable sur UE 5.8.3 après `EngineVersion` 5.8.0 isolé | 1A |
| Manager + 1 chunk initialisables ; `HeightBuffer.Num() = Resolution²` | 1B |
| Raise/Lower CPU mesurables ; API = sample × `ZScale` ; mesh MaxZ et CompZ suivent après async | 1C |
| Debounce collision observé ≈ 0,16–0,18 s (settings 0,2 s) | 1C, 2B |
| `Resolution` = sommets/côté ; pas = `WorldSize/(Resolution-1)` | 2 / 2B |
| 41 / 40 / 127 acceptés (clamp runtime 32–256) | 2B |
| Pas effectifs : 25,000 / 25,641 / 107,500 UU | 2B |
| Diamètre 25 cm (`R=12,5`) sur sommet : 1 sample ; entre sommets : 0 | 2B A/B/C |
| Diamètre 2 m (`R=100`) : ~45–52 samples sur A/B ; **1** sur C-sommet ; **4** sur C-quad | 2B |
| Lower identique ramène un champ nul à T0 | 1C, 2B |

### Vérifiés [CODE], non démontrés en runtime à l’échelle île

- `ApplyGlobalBrush` est `void` ; `Radius` = rayon UU ; `Strength` / `ZScale`.
- Pas de raffinement adaptatif, pas d’upsample, pas de T-junction API dans le source inspecté.
- `UpdateLOD` désactive la collision au-delà de 50 000 UU ; ne réduit pas la résolution du mesh (commentaire « optional » non implémenté).
- Streaming peut créer/détruire des chunks ; `bStreamingPaused` utilisé en 2B.
- Couches internes Base/Sculpt/Detail + poids : **ce n’est pas** le Store GardenFervor.
- GPU OSS : header de simulation absent ; stub isolé retourne `false`. Chemin GPU **non testé**.
- Clamp settings 1024 : métadonnée éditeur, pas le clamp `InitializeChunk`.

### Non testé (limites 1–2)

Seams, LOD, save/load, GPU, perf, packaging, Shipping, plusieurs chunks, import Landscape, navigation, eau/sol, Store `Base+Delta`, visibilité F1, Nanite, RVT/M4.

### Capacités Fab / manuel

Non attribuées à cet OSS. VHM exigé par README/manuel, absent du `.uplugin` OSS inspecté.

## 3. Calculs de densité et de mémoire

Hypothèse d’étendue : Landscape 4033×4033, pas 107,5 UU.

```
intervalles = 4032
étendue     = 4032 × 107,5 = 433 440 UU = 4 334,4 m
sommets     = 4033 × 4033 = 16 265 089
```

Les pas 50 cm (50 UU) et 25 cm (25 UU) **ne divisent pas** 107,5. Une grille uniforme à ces pas **ne peut pas** rester exactement superposée aux sommets Landscape actuels.

| Cible | Intervalles théoriques | Côté théorique | Sommets théoriques | Ratio vs 4033² |
|---|---|---|---|---|
| 1,075 m (actuel) | 4032 | 4033 | **16 265 089** | 1 |
| 50 cm | 8668,8 | 8669,8 | **≈ 75 165 432** | **≈ 4,62** `(107,5/50)² = 4,6225` |
| 25 cm | 17337,6 | 17338,6 | **≈ 300 627 050** | **≈ 18,48** `(107,5/25)² = 18,49` |

Entiers les plus proches (étendue ≠ 433 440 UU) :

| Cible | Côté entier | Étendue | Écart |
|---|---|---|---|
| 50 cm | 8669 | 433 400 UU | −40 UU |
| 50 cm | 8670 | 433 450 UU | +10 UU |
| 25 cm | 17338 | 433 425 UU | −15 UU |
| 25 cm | 17339 | 433 450 UU | +10 UU |

Ce n’est **pas** un benchmark Unreal ou TerraDyne.

### Mémoire théorique — hauteurs seules

`N` = sommets théoriques ci-dessus. Coût = `N × octets`.

| Stockage / sommet | 4033² actuel | ≈ 50 cm | ≈ 25 cm |
|---|---|---|---|
| 4 o (1× float32) | 62,0 Mio | 287 Mio | 1,12 Gio |
| 8 o (2× float32, ex. Base+Delta) | 124 Mio | 573 Mio | 2,24 Gio |
| 16 o (4× float32) | 248 Mio | 1,15 Gio | 4,48 Gio |

**Non inclus** : indices mesh, normales, UV, DynamicMesh, collision Chaos, GPU/RT, LOD, navmesh, métadonnées, persistance, copies CPU/GPU, padding, streaming.

TerraDyne [CODE] : par chunk, plusieurs tableaux (`Base`, `Sculpt`, `Detail`, `Height`, scratch, 4 poids) plus un `DynamicMesh` `Resolution²` verts / `2(Resolution-1)²` tris. Un chunk max = 256² = 65 536 samples. **Un seul chunk OSS ne peut pas porter 4033².** À 25 UU et 256 sommets/côté : ≈ 68 chunks/côté, ≈ 4 623 chunks, ≈ 3,0×10⁸ samples — même ordre que la grille 25 cm, plus les coûts mesh/collision **par chunk**.

## 4. Comparaison A / B / C

### Option A — grille uniforme fine

Principe : un seul pas (50 ou 25 cm) sur toute l’île.

Avantages [HYP] : un modèle, pas de T-junction, interpolation simple, terrassement large et détail local sur la même grille.

Limites [CALC + TEST] :
- ×4,6 / ×18,5 sommets vs aujourd’hui.
- 2B : même à 25 cm, un pinceau 25 cm **entre** sommets ne change rien. La « crédibilité 25 cm » exige à la fois le pas **et** le placement du pinceau.
- 50/25 cm ne retombent pas sur la grille 107,5 : import / Store actuel à rééchantillonner [OUVERT].
- Mesh + collision : 1C/2B prouvent le cycle sur **un** petit chunk, pas 4 600 chunks ni 3×10⁸ verts [TEST ≠ benchmark].
- Navmesh / persistance / simulations : coût non mesuré [OUVERT].
- TerraDyne A : clamp 256 impose un pavage ; seams **non testés**.

Pertinence : simple conceptuellement, cher et non aligné sur le Landscape actuel. Inadaptée comme **premier** pas si 08.7 exige des preuves.

### Option B — base + détail local

Principe : garder ≈ 107,5 cm (ou le Store actuel) comme base île ; activer une grille fine seulement où le terrassement / l’eau / le sol l’exigent.

Avantages [HYP] : mémoire de base proche de l’actuel ; 2B montre qu’un chunk 25 cm local (1000 UU, 41²) est initialisable et cohérent CPU/API/mesh/collision **en isolation** [TEST]. Les grandes opérations restent sur la base.

Risques [HYP + CODE] :
- Transfert base ↔ patch (rééchantillonnage, autorité unique).
- Raccords, T-junctions, fissures : **non testés**, absents de l’API OSS inspectée.
- Collision / nav / persist doivent suivre le patch, pas seulement la base (00.5.G5) [DG].
- Agents autonomes : quelle grille interrogent-ils au bord d’un patch ? [OUVERT]
- TerraDyne : chunks à résolutions **différentes** voisines = [HYP] non démontré. `GlobalChunkSize` unique par manager [CODE]. Mélanger 1000@41 et 13545@127 sous un seul manager n’est pas prouvé.

Preuves TerraDyne pour B : un chunk uniforme isolé, oui [TEST]. Raffinement adaptatif, transitions, multi-résolution : **non, lacune**.

### Option C — autres représentations

| Variante | Utile si | Conséquences | Verdict ici |
|---|---|---|---|
| Heightfield + mesh local (PMC / DynamicMesh) sur delta | Garder Store + Landscape, remplacer seulement les zones touchées | Overhangs/tunnels : non (heightfield). Collision locale déjà étudiée côté GF (1C PIE) sans valider D2. Seams [OUVERT] | Compatible 08.6 en principe ; 08.7 ouvert |
| Chunks taille variable, résolution uniforme | Adapter la taille au rayon d’outil, pas le pas | Même limite 2B : le pas fixe la plus petite forme. Seams [OUVERT] | N’apporte pas 25 cm si le pas reste 107,5 |
| Voxel / SDF | Overhangs, tunnels, érosion 3D | Coût, nav, RTS lecture hauteur, hors besoin actuel [HYP] | Non justifié maintenant |
| Plugin Fab / VHM | Rendu heightfield | Pas dans l’OSS testé ; réintroduit VHM = décision 08.7 | Non attribué à 270c311 |

Overhangs / tunnels : aucune option heightfield (A, B, TerraDyne OSS, Landscape) ne les offre. Les viser imposerait une autre primitive [HYP].

## 5. Trois modèles d’autorité de hauteur

Conflit de référence [DG] : 08.6, 00.5.S4, 00.6.A2, C-01 D1 — après terraform, Store `Base+Delta` = vérité hauteur. Landscape = base initiale. Collision/nav suivent le relief modifié (00.5.G5). 08.7 n’a choisi aucune techno.

### 5.1 TerraDyne devient l’autorité

| Sujet | Analyse |
|---|---|
| Cohérence hauteurs | `HeightBuffer` = clamp(Base+Sculpt+Detail, 0–1) × `ZScale` [CODE]. Unités et quantification ≠ Store GF. |
| Runtime | Raise/Lower OK sur 1 chunk CPU [TEST]. Pas de preuve île, seams, GPU. |
| Mesh / collision | Cohérents sur 1 chunk après debounce [TEST]. LOD coupe la collision à 500 m [CODE]. |
| Persistance | Cache chunks / save TerraDyne [CODE], **non testé**. Remplacerait C-19 / E6 [DG conflit]. |
| Import | Migration Landscape dans le plugin [CODE], non exercée. ≠ Store actuel. |
| Navigation | `bRefreshNavMeshOnTerrainChanges` [CODE], non mesuré. |
| Raccords / résolutions | Non testé. |
| Double autorité | **Oui**, si le Store GF continue d’exister. Conflit 08.6. |
| Coût / inconnues | Réécrire gameplay (SampleGround, persist, F1). OSS strippé, GPU stub isolé. |

**Conflit [DG]** : remplacer silencieusement le Store. Interdit par 08.6 tant que 08.7 n’a pas tranché **et** que le Store n’a pas été explicitement abandonné — ce qui n’est pas le cas.

### 5.2 Store `Base+Delta` reste l’autorité ; le terrain est alimenté

| Sujet | Analyse |
|---|---|
| Cohérence | Un writer : le Store. TerraDyne / Landscape / mesh = représentation [DG]. |
| Runtime | `ApplyBrushAt` déjà défini côté GF comme succès = écriture Store (E1, runtime non rappelé ici comme preuve 3A). |
| Mesh / collision | Doivent **lire** le Store (E2 visé). 1C TerraDyne ne lit pas le Store. |
| Persistance | Reste C-19 / capacité Store [DG]. |
| Import | Landscape → Store déjà le modèle 00.5.S4. |
| Navigation | G5 : suivre le Store, pas l’inverse. |
| Raccords | Problème de **représentation**, pas d’autorité. |
| Double autorité | Évitable si TerraDyne n’est jamais sourcé pour le gameplay. |
| Coût / inconnues | F1 visibilité toujours INVALIDÉE. Alimenter TerraDyne depuis le Store = sync non écrite [OUVERT]. |

**Compatible [DG]** avec 08.6. N’impose pas TerraDyne.

### 5.3 Hybride à responsabilité unique

Exemple [HYP] : Store = vérité ; patch TerraDyne ou PMC = mesh/collision locale ; Landscape = base non touchée.

| Sujet | Analyse |
|---|---|
| Cohérence | Viable seulement si **une** API de lecture hauteur (Store) et des sinks en aval. |
| Risque | Deux buffers qui divergent (déjà le motif F1 / overlay). |
| Raccords | Toujours à prouver. |
| DG | Acceptable **si** le Store reste la source et 08.7 choisit les sinks. |

Sans contrat d’ownership, l’hybride **est** une double autorité.

## 6. Matrice risques / bénéfices / inconnues

| Voie | Bénéfice | Risque principal | Inconnue majeure | Conflit DG |
|---|---|---|---|---|
| A uniforme 25 cm | Un modèle | Mémoire ×18,5 + mesh/col île | Perf, import, nav | Non, si Store reste autorité |
| A uniforme 50 cm | Moins cher que 25 cm | Pas non multiple de 107,5 ; 25 cm toujours impossible | Idem | Non, même réserve |
| B base + local | Coût localisé ; 2B local OK | Seams, sync, G5 sur le patch | Multi-résolution TerraDyne | Non, si Store autorité |
| C voxel | Overhangs | Coût, RTS, hors besoin | Tout | Possible détournement 08.7 |
| TerraDyne autorité | Runtime chunk déjà mesuré | Remplace le Store | Île, persist, F1 | **Oui (08.6)** |
| Store autorité + sink | Respecte 08.6 | F1 / G5 toujours ouverts | Quel sink (M4, PMC, TD, …) | Non |
| Hybride flou | Flexibilité apparente | Double vérité | Ownership | Oui si mal cadré |

## 7. Questions encore ouvertes

1. Le gameplay a-t-il **besoin** d’une forme de 25 cm, ou d’une touche au sommet 107,5 cm ? (08.7 / design, pas un test plugin)
2. Comment rééchantillonner Store 107,5 → 25/50 sans perdre l’autorité unique ?
3. Raccord mesh + collision + nav au bord d’un patch (G5) ?
4. Un manager TerraDyne peut-il porter des chunks de tailles/résolutions différentes sans fissure ? **[OUVERT, non testé]**
5. F1 : visibilité shipping Base+Delta, indépendante de TerraDyne.
6. Persist C-19 : le sink doit pouvoir se reconstruire depuis le Store, pas l’inverse.
7. Simulations eau/sol : pas de grille, ou grille fine locale ?
8. GPU TerraDyne OSS : inexistant dans l’arbre public testé.

## 8. Prochaine expérience minimale — non exécutée

**Ne pas** commencer une phase 3B d’intégration, ni un pavage multi-chunks TerraDyne, ni un 25 cm île entière.

Expérience minimale utile, **si autorisée** :

> **Étude 3B (papier + éventuellement un proto isolé plus tard)** : spécifier le contrat d’un **patch local** (emprise, pas, ownership Store, règles de raccord) **sans implémenter** le sink. Ou, si l’humain tranche que 25 cm n’est pas requis, clore la branche précision et revenir à 08.7 / F1 (visibilité + G5) sur la grille actuelle.

Un test runtime seams TerraDyne n’informe GardenFervor **que si** 08.7 a déjà accepté un sink à chunks. Ce n’est pas le cas.

## 9. Affirmations non démontrées

- TerraDyne convient à l’île 4033².
- Le GPU OSS fonctionne.
- Les seams / LOD / save sont corrects.
- Un mesh 25 cm île entière est jouable.
- TerraDyne lit ou écrit le Store GF.
- Le manuel Fab décrit cet OSS.
- Option B est implémentable sans fissure.
- Les voxels sont nécessaires.
- F1, G5, C-19 sont résolus.
- Cette recommandation vaut validation 08.7.

## 10. Gouvernance et recommandation

**Recommandation technique** (confiance **moyenne**) :

1. Conserver le Store `Base+Delta` comme unique autorité — déjà VALIDÉ ; 3A ne le rouvre pas.
2. Ne pas adopter TerraDyne, ni comme autorité, ni comme architecture île.
3. Traiter TerraDyne OSS comme **preuve qu’un heightfield chunk CPU peut rester cohérent** (données / API / mesh / collision) à petite échelle, y compris à 25 cm **locaux**.
4. Pour 08.7 : écarter A-25cm-île comme premier choix (coût [CALC], non-alignement 107,5, pas de preuve perf). Garder B (base + détail local, sink à choisir) et « rester à 107,5 cm si le design l’accepte » comme candidates. C voxel : hors besoin actuel.

**La recommandation changerait si** : (a) le design abandonne le détail &lt; 107,5 cm ; (b) une preuve montre un sink île 25 cm dans le budget ; (c) 08.7 abandonne explicitement le Store — aujourd’hui interdit.

Statut : étude. Pas de 3B. Pas de VALIDÉ.

Design Gate : aucun statut modifié. F1 reste INVALIDÉ et 08.7 reste À DÉFINIR. Cette étude est une recommandation technique, pas une décision de conception validée.

# Phase 2B — résultats de précision spatiale

Exécution du protocole `Results/PHASE2_TEST_PLAN.md`.  
Prototype isolé uniquement. Plugin inchangé. GardenFervor inchangé.

## Statut

`12/12 cas exécutés. 0 abort. Aucun résidu HeightBuffer après Lower.`

Les mesures confirment les prédictions du plan sur ce terrain nul CPU. Elles ne démontrent ni un disque continu de 25 cm à l’échelle GardenFervor, ni une adoption TerraDyne.

## Fichiers

Modifiés (hôte uniquement) :
- `Source/TerraDyneOSS_Proto/TerraDyneOSS_ProtoGameMode.h`
- `Source/TerraDyneOSS_Proto/TerraDyneOSS_ProtoGameMode.cpp`

Ajoutés :
- `Results/phase2b_precision.log`
- `Results/PHASE2B_PRECISION_RESULTS.md`

Non modifiés : plugin TerraDyne, `DefaultGame.ini` (`bEnableGPUBrushes=false` déjà posé), Design Gate, GardenFervor.

## Compilation et lancement

- Game Win64 Development : Succeeded
- Editor Win64 Development : Succeeded
- Commande : `UnrealEditor-Cmd` OpenWorld `-game -unattended -nopause -nosplash -nullrhi`  
  `-AbsLog=Results/phase2b_precision.log`
- Module TerraDyne chargé. GPU : `bEnableGPUBrushes=false`, `CanEverRender=false`
- `bStreamingPaused=true`, outskirts désactivés
- `PHASE2B SUMMARY executed=12 aborted=0`

## Paramètres effectifs après init

| Config | Demandé | Effectif | Step | Samples | GPU | ZScale |
|---|---|---|---|---|---|---|
| A | 1000 / 41 | 1000.000000 / 41 | **25.000000000** | 1681 | false | 3000 |
| B | 1000 / 40 | 1000.000000 / 40 | **25.641025543** | 1600 | false | 3000 |
| C | 13545 / 127 | 13545.000000 / 127 | **107.500000000** | 16129 | false | 3000 |

Aucune config n’a été clampée hors des valeurs demandées.

Communs transmis à `ApplyGlobalBrush` :
- `Strength = 80` UU (`StrengthNorm = 80/3000 ≈ 0.02666667`)
- Rayon 25 cm de diamètre = **12.5 UU**
- Rayon 2 m de diamètre = **100 UU**
- Mode Raise puis Lower si delta CPU ; sinon pas de Lower
- Fonction `void` : le succès est le delta `HeightBuffer`

## Les 12 cas

`changed` = nombre d’échantillons `HeightBuffer` dont `|Δ| > KINDA_SMALL_NUMBER`.  
Délais : mesh ≈ 1 ms après Raise ; collision ≈ 160–180 ms (debounce 0.2 s).  
Après chaque Lower avec delta : `maxAbsAfterLower = 0`, `changed vs baseline = 0`.

### Config A — pas 25 UU

| Cas | Centre | R | CPU changed | Index / Δ | API monde | Mesh MaxZ | CompZ | Lower |
|---|---|---|---|---|---|---|---|---|
| A_25cm_vertex | `(0,0,0)` idx 840 | 12.5 | **1** | 840 : `0.02666667` | `80.000000` | `80.000000` | `80.000000` | reset 0 |
| A_25cm_quad | `(12.5,12.5,0)` | 12.5 | **0** | — | `0` | `0` | `0` | non |
| A_2m_vertex | `(0,0,0)` | 100 | **45** | max 840 : `0.02666667` sum `0.40268806` | `80.000000` | `80.000000` | `80.000000` | reset 0 |
| A_2m_quad | `(12.5,12.5,0)` | 100 | **52** | max 840 : `0.02446130` sum `0.40174377` | `73.383888` | `73.383888` | `73.383888` | reset 0 |

A_25cm_quad : distance aux 4 sommets `12.5√2 ≈ 17.678 > 12.5` → 0 sommet. Conforme au plan.

### Config B — pas ≈ 25.641 UU

| Cas | Centre | R | CPU changed | Index / Δ | API monde | Mesh MaxZ | CompZ | Lower |
|---|---|---|---|---|---|---|---|---|
| B_25cm_vertex | `(12.820,12.820,0)` idx 820 | 12.5 | **1** | 820 : `0.02666667` | `79.999847` | `80.000000` | `79.999947` | reset 0 |
| B_25cm_quad | `(0,0,0)` | 12.5 | **0** | — | `0` | `0` | `0` | non |
| B_2m_vertex | `(12.820,12.820,0)` | 100 | **45** | max 820 : `0.02666667` sum `0.38292679` | `79.999954` | `80.000000` | `79.999985` | reset 0 |
| B_2m_quad | `(0,0,0)` | 100 | **44** | max 820 : `0.02435470` sum `0.38112187` | `73.064095` | `73.064110` | `73.064095` | reset 0 |

B_25cm_quad au centre du chunk (centre de quad, pas un sommet) : 0 sommet. Conforme.  
`(0,0,0)` n’est pas un sommet en B.

Écarts `79.999847` / `79.999947` vs `80` : valeurs brutes, pas une tolérance inventée. CPU exact `80/3000`.

### Config C — pas 107.5 UU (analogie GF, données générées)

| Cas | Centre | R | CPU changed | Index / Δ | API monde | Mesh MaxZ | CompZ | Lower |
|---|---|---|---|---|---|---|---|---|
| C_25cm_vertex | `(0,0,0)` idx 8064 | 12.5 | **1** | 8064 : `0.02666667` | `80.000000` | `80.000000` | `80.000000` | reset 0 |
| C_25cm_quad | `(53.75,53.75,0)` | 12.5 | **0** | — | `0` | `0` | `0` | non |
| C_2m_vertex | `(0,0,0)` | 100 | **1** | 8064 : `0.02666667` | `80.000000` | `80.000000` | `80.000000` | reset 0 |
| C_2m_quad | `(53.75,53.75,0)` | 100 | **4** | 8064, 8065, 8191, 8192 : chacun `0.00386664` | `11.599926` | `11.599926` | `11.599926` | reset 0 |

C_2m_vertex : `100 < 107.5` → un seul sommet, identique au 25 cm sur sommet.  
C_2m_quad : 4 sommets du quad, falloff faible, API `11.599926` UU.

## Comparaison A / B / C

| Observation | A (25 cm) | B (≈25.64 cm) | C (107.5 cm) |
|---|---|---|---|
| 12.5 UU sur un sommet | 1 sample, API 80 | 1 sample, API ≈80 | 1 sample, API 80 |
| 12.5 UU entre sommets | 0 | 0 | 0 |
| 100 UU sur un sommet | 45 samples | 45 samples | **1 sample** |
| 100 UU entre sommets | 52 samples, API 73.38 | 44 samples, API 73.06 | **4 samples, API 11.60** |
| Mesh/collision suivent le CPU | oui | oui | oui |
| Reset Lower | oui si delta | oui si delta | oui si delta |

Différence 12.5 vs 100 : nette sur A/B (1 vs ~45 sommets). Sur C, 100 UU sur sommet ne s’élargit pas : le pas (107.5) est plus grand que le rayon.

## Couches

1. **CPU** — `HeightBuffer` public. Compte d’index et Δ mesurés. Raise `void` n’a jamais été pris comme preuve.
2. **API** — `GetHeightAtLocation` / `GetWorldHeightAtLocation` = sample interpolé × 3000. Sur sommet exact A/C : 80. Sur quad 2 m : inférieur (falloff).
3. **Mesh** — `GetLocalBounds` MaxZ suit le max API après apply async (~1 ms).
4. **Collision** — `LineTraceComponent` + trace monde sur `TerraDyneChunk_*`. CompZ suit MaxZ après ~0.16–0.18 s. Pas de hit Landscape une fois le chunk prêt.

Cas 0 sommet : API, MaxZ et CompZ restent 0 après 0.25 s d’observation. Un dirty interne possible n’a pas produit de variation de hauteur.

## Résidus et échecs

Aucun. `maxAbs` avant chaque cas = 0. Après chaque Lower = 0. Aucune config interrompue.

## Limites

- `-nullrhi` : pas de preuve visuelle / matériau.
- Mesh : bounds Z, pas une lecture sommet à sommet.
- Collision : traces aux XY du pinceau, pas un scan de surface.
- Terrain initial nul généré. Pas un import Landscape GardenFervor, pas le Store `Base+Delta`.
- Un seul chunk, streaming en pause. Pas de seams, LOD, save/load, GPU, perf.
- `KINDA_SMALL_NUMBER` sert uniquement à détecter un changement, pas une tolérance métier.
- Cette phase ne démontre ni les raccords entre chunks, ni le LOD, ni les performances à grande échelle, ni la compatibilité avec le stockage `Base + Delta` de GardenFervor.

## Conclusions strictement soutenues

1. 41, 40 et 127 sont acceptés par `InitializeChunk` ; les pas mesurés sont 25, ≈25.641 et 107.5 UU.
2. Un pinceau de diamètre 25 cm (`R=12.5`) ne sculpte un sommet que s’il est centré assez près de ce sommet. Entre sommets, sur A, B et C, **zéro sample** ne change.
3. Donc « 25 cm de diamètre » n’est pas un disque continu : c’est un test d’inclusion de sommets. Sur une grille à 107,5 cm, ce pinceau ne peut pas modeler une forme de 25 cm ; il ne peut que toucher un sommet déjà visé.
4. Un pinceau de 2 m (`R=100`) élargit A/B (~45–52 sommets) mais pas C-sur-sommet (toujours 1), parce que `100 < 107.5`.
5. Quand le CPU change, API, MaxZ mesh et CompZ collision suivent la même hauteur monde, après le cycle async habituel.
6. Lower identique ramène ce terrain nul à T0.

Non soutenu : précision continue 25 cm pour GardenFervor ; adoption TerraDyne ; remplacement du Store ; F1 ; 08.7.

## Prochaine étape (non lancée)

Si une suite est autorisée : décider si GardenFervor a besoin d’un pas ≤ 25 cm (type A) ou si un pinceau « 25 cm » sur une grille 107,5 cm (type C) est suffisant comme touche d’un seul sommet. Ne pas enchaîner seams / persist / GPU sans nouvelle autorisation.

Design Gate : aucun statut modifié. F1 reste INVALIDÉ et 08.7 reste À DÉFINIR. Ces mesures ne valident ni l’adoption de TerraDyne ni un choix architectural pour GardenFervor.

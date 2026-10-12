# AUDIT-02 — Comparaison architecturale TerraDyne / GardenFervor

| Champ | Valeur |
|---|---|
| Identifiant | `EXT-TERRADYNE-A02` |
| Date d’inspection et de rédaction | **2026-10-12** |
| Statut documentaire | `TECHNICAL_REVIEW_COMPLETE` |
| Verdict technique (recommandation d’audit) | **Option B recommandée** — GardenFervor conserve l’autorité du terrain |
| Décision d’architecture 08.7 | **Non prise** — DG-08.7 reste `À DÉFINIR` |
| Intégration TerraDyne | **Non autorisée** |
| Audit 3 (M4) | **Non exécuté** — verdict campagne **D** inchangé |

**Design Gate : aucune modification de contrat ou de statut — audit architectural et documentation uniquement.**

Ce rapport est le **document d’autorité** de l’audit 2 dans GardenFervor. La note proto `Results/PHASE3A_ARCHITECTURE_COMPARISON.md` est une **source de travail externe**. Elle n’est pas ce rapport. Ses affirmations ont été revérifiées sur les fichiers ; celles insuffisamment étayées sont corrigées ci-dessous.

Aucune décision n’est adoptée par ce texte. Aucune recommandation n’autorise une intégration.

---

## 1. Objet, périmètre et date

**Question :** qui doit détenir l’autorité des hauteurs et des opérations géométriques si TerraDyne OSS `270c311` est évalué pour GardenFervor — le plugin, ou le Store GardenFervor — et ce choix est-il une fondation viable pour sols, eau, écosystèmes, persistance et unités autonomes ?

**Inclus :** lecture des contrats et du Design Gate ; inspection C++ TerraDyne et GardenFervor ; confrontation PHASE3A ; options A (TerraDyne autoritaire) et B (GardenFervor autoritaire) ; chemin technique éventuel d’une représentation TerraDyne sans transfert d’autorité.

**Exclus :** compilation, packaging, tests de perf, audit 3 M4, implémentation, modification 08.7, intégration.

**Non réalisé par Cursor :** aucun test runtime. Les confirmations HoleFix restent des **validations manuelles de l’utilisateur**.

---

## 2. Versions, chemins et sources effectivement consultées

### 2.1 GardenFervor

Racine : `C:\Users\sahel\Documents\Unreal Projects\GardenFervor`

| Source | Statut constaté à l’inspection |
|---|---|
| `REGLES_PROJET.md` | Process §1bis — non modifié par cet audit |
| `ETAT_PROJET.md` | 08.6 VALIDÉ · 08.7 À DÉFINIR · F1 INVALIDÉ |
| `HISTORIQUE_MODIFICATIONS.md` | Landscape prouvé **4033×4033**, `cellQuads=16` |
| `GardenFervor_DesignGate_React/src/data/designGate.js` | **08.6 VALIDÉ** · **08.7 À DÉFINIR** · 00.5.S4 / 00.6.A2 / 00.5.G5 VALIDÉ |
| `CONTRATS/C-01_TERRAIN_RUNTIME.md` | **VALIDÉ** (documentaire) · D1–D14 · dettes E1–E8 listées |
| `CONTRATS/C-02_SUBSTRAT_SPATIAL.md` | **VALIDÉ** · F1 : C-02 ne possède jamais la hauteur |
| `CONTRATS/C-08_TERRAFORMER_OPERATIONNEL.md` | **VALIDÉ** · métier terrassement ; Case B suspendu |
| `CONTRATS/C-15_HYDROLOGIE.md` | **VALIDÉ** conception · C-01 = relief |
| `CONTRATS/C-16_SOL.md` | **VALIDÉ** conception · n’écrit pas la hauteur |
| `CONTRATS/C-17_VEGETATION_ECOSYSTEMES.md` | **VALIDÉ** conception · ≠ runtime |
| `CONTRATS/C-19_PERSISTANCE_SAUVEGARDE.md` | **VALIDÉ** conception · vérité hauteur = C-01 · persist OFF |
| `Source/.../GardenFervorRuntimeTerraformStore.h/.cpp` | Store Base + `HeightDeltaCm` + paint plat |
| `Source/.../GardenFervorLandscapeTerraformSubsystem.cpp` | `ApplyBrushAt` → Store puis LandscapeEdit `#if WITH_EDITOR` ; retour `bStoreOk` |
| `Source/.../GardenFervorGroundUtils.cpp` | Lecture Store si ready + base accurate |
| `Source/.../GardenFervorRuntimeTerraformCollision.h` | PMC grossier, stride 16, cooldown 0,35 s |
| `Source/.../GardenFervorDeveloperSettings.h` | overlay et persist **false** |

### 2.2 TerraDyne (lecture seule)

Racine : `C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311`  
Commit : `270c31117e20e491ef5333dd7e2f9dc133fcdc38` (`Results/TERRADYNE_COMMIT.txt`)

| Source | Usage |
|---|---|
| `Plugins/TerraDyne/.../TerraDyneChunk.h/.cpp` | Buffers, clamp 32–256, collision, `LoadFromData` |
| `Plugins/TerraDyne/.../TerraDyneManager.h/.cpp` | `ApplyGlobalBrush` (void), `ApplyAuthorizedBrush`, streaming, save |
| `Plugins/TerraDyne/.../TerraDyneSaveGame.h` | SaveGame v2, couches Base/Sculpt/Detail, 4 poids |
| `Plugins/TerraDyne/.../TerraDyneSettings.h` | Debounce collision, LOD, GPU, streaming |
| `Results/DEMO_TERRADYNE_M4.md` | HoleFix, M4 **D** |
| `Results/PHASE3A_ARCHITECTURE_COMPARISON.md` | Note externe, non canonique |
| `Results/PHASE2B_PRECISION_RESULTS.md` | Pas / échantillons (phases antérieures) |
| AUDIT-01 canonique GF | Fiabilité proto ; pas une comparaison 08.7 |

Build de référence manuelle : `Packaged/Win64ShippingHoleFix/TerraDyneOSS_Proto.exe` — **non relancé** ici.

### 2.3 Correction de PHASE3A

| Affirmation PHASE3A | Vérification 2026-10-12 |
|---|---|
| TerraDyne autorité ⇔ conflit 08.6 / 00.5.S4 / C-01 D1 | **Confirmé** sur `designGate.js` et C-01 D1 |
| Clamp runtime `InitializeChunk` 32–256 | **Confirmé** (`TerraDyneChunk.cpp` L775) |
| Settings éditeur ClampMax 1024 ≠ clamp `InitializeChunk` | **Confirmé** (`TerraDyneSettings.h` vs L775) |
| Pas de mesh adaptatif / upsample dans l’OSS inspecté | **Confirmé** (aucun API T-junction / refine) |
| `ApplyGlobalBrush` void | **Confirmé** |
| Debounce collision ~0,2 s | **Confirmé** (défaut code 0,2 ; settings `CollisionDebounceTime`) |
| Collision LOD ~50 000 UU `NoCollision` | **Confirmé** (`UpdateLOD`) |
| Pas Landscape GF = 107,5 UU | **Corrigé** : 107,5 UU = pas du **chunk démo TerraDyne** (AUDIT-01). Store GF : `QuadSizeUU` défaut **100** ; runtime = scale Landscape ou DataAsset. Grille prouvée **4033×4033**. Ne pas fusionner les deux pas. |
| Mémoire ~1,1 Gio île 25 cm | Ordre de grandeur **plausible en calcul** si on prend 4033² et un pas ~100 UU, puis 25 UU — **CALC**, pas une mesure. Chiffres exacts de PHASE3A (107,5 UU) **non repris** comme vérité GF. |
| Famille « B — base + détail local » PHASE3A | **N’est pas** l’option B de cet audit. Non adoptée ici. |

---

## 3. Résumé exécutif

TerraDyne OSS est un moteur **couplé** : les buffers de chunks sont à la fois vérité interne, source du DynamicMesh et source de la collision. Raise/Lower/peintures/collision après délai sont **VALIDÉ MANUELLEMENT PAR L’UTILISATEUR** sur HoleFix. Cela ne dit pas qui doit être l’autorité GardenFervor.

Les contrats **VALIDÉ** nomment déjà l’autorité : `RuntimeTerraformStore` (C-01 D1), réaffirmé par 08.6, 00.5.S4 et 00.6.A2. **08.7 n’a retenu aucune techno.** Faire de TerraDyne l’autorité (option A) **entre en conflit avec ces textes** — incompatibilité **contractuelle** démontrée, pas une impossibilité physique du plugin.

L’option B (GardenFervor reste l’autorité) est **alignée** sur les contrats. Ses limites actuelles sont réelles : visibilité shipping (E5 / F1 INVALIDÉ), collision PMC grossière et historiquement inactive, persist OFF (C-19 / E6), sols/eau stubs. Ce ne sont pas des raisons de transférer l’autorité au plugin.

Un usage TerraDyne **uniquement présentation**, alimenté depuis le Store, n’est **pas démontré**. Un chemin public existe (`LoadFromData` + mesh dirty) mais exige un adaptateur de grilles/unités/chunks **absent**. Tant que cet adaptateur n’est pas conçu et prouvé, l’architecture hybride n’est pas recommandée.

**Recommandation d’audit :** option B. **Pas une décision 08.7.**

---

## 4. Faits établis et niveaux de preuve

Légende de la matrice §7 : `DEMONTRÉ_MANUELLEMENT` · `INSPECTÉ_DANS_LE_CODE` · `POSSIBLE_SOUS_CONDITIONS` · `INCONNU` · `INCOMPATIBILITÉ_DÉMONTRÉE`.

### GardenFervor

- Store : `BaseHeightCm` + `HeightDeltaCm` ; `SampleWorldHeight` = base + delta. **INSPECTÉ_DANS_LE_CODE**
- `ApplyBrushAt` écrit le Store (Raise/Lower/Paint) puis LandscapeEdit seulement `WITH_EDITOR` ; retourne `bStoreOk`. **INSPECTÉ_DANS_LE_CODE** (la dette contractuelle E1 décrit encore un `\|\| bLandscapeOk` — écart doc/code **non tranché ici**)
- `GroundUtils` lit le Store si ready et base accurate. **INSPECTÉ_DANS_LE_CODE** (E2 contractuel peut être partiellement daté)
- Spatial `QueryTerrain` via Store. **INSPECTÉ_DANS_LE_CODE**
- Persist settings `bEnableRuntimeTerraformPersistence = false`. **INSPECTÉ_DANS_LE_CODE**
- Collision runtime : acteur PMC `SampleStride=16`, `MaxGridAxis=257`, cooldown 0,35 s. **INSPECTÉ_DANS_LE_CODE** ; non présenté comme VALIDÉ en jeu
- Sol / eau spatial : tableaux stub. **INSPECTÉ_DANS_LE_CODE**
- Unités : `UnitTaskAgent` appelle `ApplyBrushAt`. **INSPECTÉ_DANS_LE_CODE** — pas de preuve d’échelle RTS

### TerraDyne

- Couches `BaseBuffer` / `SculptBuffer` / `DetailBuffer` ; `HeightBuffer` = somme clampée `[-1,1]` après edit. **INSPECTÉ_DANS_LE_CODE**
- 4 `WeightBuffers`. **INSPECTÉ_DANS_LE_CODE**
- `InitializeChunk` : `Clamp(InRes, 32, 256)`. **INSPECTÉ_DANS_LE_CODE**
- `ApplyGlobalBrush` : void ; n’édite que les chunks **déjà** dans `ActiveChunkMap`. **INSPECTÉ_DANS_LE_CODE**
- `ApplyAuthorizedBrush` : bool (autorité + budgets), puis `ApplyGlobalBrush`. **INSPECTÉ_DANS_LE_CODE** — jamais exercé comme API GardenFervor
- Mesh async + collision debounce + max 1 collision/frame (settings). **INSPECTÉ_DANS_LE_CODE**
- Raise/Lower/peintures/collision après délai HoleFix. **DEMONTRÉ_MANUELLEMENT** (utilisateur, pas Cursor)
- `SaveWorld`/`LoadWorld` + cache chunks. **INSPECTÉ_DANS_LE_CODE** ; ronde-trip host **NON VALIDÉ**
- `LoadFromData` écrit les buffers et pose `bMeshDirty` / `bCollisionDirty`. **INSPECTÉ_DANS_LE_CODE**
- GPU brushes stub dans la baseline proto. **INSPECTÉ_DANS_LE_CODE** (AUDIT-01 TF-06)
- M4 **D**. Constat de campagne ; audit 3 non lancé

---

## 5. Option A — TerraDyne est l’autorité du terrain

GardenFervor commanderait les travaux ; TerraDyne détiendrait hauteurs et géométrie.

### Présent dans le code

API lecture : `GetWorldHeightAtLocation`, `GetHeightAtLocation`, `GetWorldWeightAtLocation`.  
API écriture : `ApplyGlobalBrush` (void), `ApplyAuthorizedBrush` (bool de validation, pas « Store GF mis à jour »).  
Couches Base/Sculpt/Detail + hauteur composée.  
Cohérence interne : edit → HeightBuffer → mesh async → collision debounce.  
Streaming : load/unload, `bStreamingPaused`.  
SaveGame v2 + cache.  
Undo/redo **par PlayerController** ; chunks streamés hors carte **ignorés** (commentaire Manager).  
Pas de file d’attente d’opérations C-04/C-11. Dernier pinceau gagne sur le sommet.  
Dépendance : vérité = layouts internes du plugin (`HeightBuffer` normalisé × `ZScale`).

### Observé HoleFix (utilisateur)

Un chunk CPU Raise/Lower/paint/collision après délai — **pas** multi-unités, **pas** île, **pas** save.

### Seulement envisageable

Wrapper C-01 autour de TerraDyne ; mapping Creuser/Remblayer/Aplanir ; persistance C-19 sur SaveGame TerraDyne ; sols/eau dérivés des 4 poids ; raffinement 25 cm hors clamp 256 (multi-chunks 256²) ; perf RTS.

### Conflits

| Contrat / item | Conflit |
|---|---|
| C-01 D1 | Autorité nommée = `RuntimeTerraformStore`, pas TerraDyne |
| 08.6 / 00.5.S4 / 00.6.A2 | Store Base+Delta = vérité après terraform ; affichage ≠ vérité |
| C-01 D2 | Succès = Store GF modifié ; `ApplyGlobalBrush` est void |
| C-01 D4 / C-15 / C-16 / C-17 | Relief seul vs poids/herbe/biome plugin mélangés |
| C-02 F1 | Substrat ne doit pas posséder la hauteur ; option A déplace l’autorité hors C-01 |
| C-19 | Politique save GF ; format TerraDyne ≠ `UGardenFervorTerraformSaveGame` |

Ce n’est **pas** une incompatibilité runtime du proto. C’est une **incompatibilité avec les contrats VALIDÉ** tels qu’écrits. Les lever exigerait une **modification explicite** de C-01 / 08.6 — hors périmètre, non faite.

Fondation sols/eau : les 4 weight layers ne sont pas C-16 ; aucun buffer hydrologique. Option A **contraint** à greffer C-15/C-16 à côté tout en lisant la hauteur dans TerraDyne — double vérité si le Store GF reste, ou rupture D1 s’il disparaît.

---

## 6. Option B — GardenFervor reste l’autorité du terrain

Le Store reste l’unique vérité de hauteur. La représentation (Landscape/M4 aujourd’hui, éventuellement autre) suit (C-01 D3, D6, D10).

### Supporté par le code actuel

- Autorité CPU île : `ApplyHeightBrush` / `ApplyPaintBrush` / `SampleWorldHeight`
- Commandes : agents → `ApplyBrushAt` (succès Store)
- Signal C-02 après Store ok
- Présentation PIE : LandscapeEdit ; shipping : Store-only (E5 ouvert)
- Collision prévue : PMC depuis Store (non démontrée comme G5 shipping)
- Persist : `SaveToSlot` existe, **flag OFF**
- Sol/eau : contrats séparés ; runtime stub — **l’autorité hauteur n’empêche pas** d’ajouter C-15/C-16 plus tard

### TerraDyne sans transfert d’autorité

Chemin identifié, **non implémenté** :

1. Après écriture Store, convertir la région dirty en `FTerraDyneChunkData` (rééchantillonnage XY, conversion cm → normalisé/`ZScale`, tuilage chunks).
2. Appeler `ATerraDyneChunk::LoadFromData` (public) qui salit mesh+collision.
3. Ne **pas** appeler `ApplyGlobalBrush` (sinon TerraDyne réécrit une seconde vérité).
4. Gameplay continue de lire le Store (D1).

Sans cet adaptateur, **on ne peut pas** affirmer qu’une représentation TerraDyne alimentée par GardenFervor est réalisable. Les buffers publics du chunk permettraient un poke interne, mais `RebuildMesh` est privé ; `LoadFromData` / `RebuildPhysicsMesh` sont les leviers publics. Grilles : Store ~4033² × `QuadSizeUU` (défaut 100) vs chunks TerraDyne **uniformes**, max **256** sommets/côté. Alignement exact 25 cm **non démontré**.

### Limites de B (ne pas l’idéaliser)

- F1 INVALIDÉ : visibilité shipping du relief modifié non close
- Collision Store-driven non VALIDÉE au sens 00.5.G5
- Persist non opérationnelle
- LandscapeEdit session-only
- Coût si l’on ajoute quand même TerraDyne en mesh : double pile, désync, maintenance plugin OSS

B reste **cohérente** avec C-01/C-02/C-15/C-16/C-19. Les trous sont des **dettes de représentation et de persist**, déjà nommées (E5, E6, 08.7), pas un motif de changer d’autorité.

---

## 7. Matrice comparative

| Critère | Option A — TerraDyne autoritaire | Option B — GardenFervor autoritaire | Preuves | Risques / coûts |
|---|---|---|---|---|
| Source de vérité de la hauteur | `HeightBuffer` (Base+Sculpt+Detail) interne plugin | `RuntimeTerraformStore` base+delta | A : code chunk ; B : Store.h + C-01 D1 | A : `INCOMPATIBILITÉ_DÉMONTRÉE` vs D1/08.6. B : `INSPECTÉ_DANS_LE_CODE` |
| API C++ et accès aux données | `ApplyGlobalBrush` void ; `ApplyAuthorizedBrush` bool plugin ; lecture world height | `ApplyBrushAt` / `ApplyHeightBrush` bool Store ; `SampleWorldHeight` | Code Manager vs Subsystem | A ne satisfait pas D2 tel quel. B : E1 doc vs code à réconcilier |
| Cohérence du maillage | DynamicMesh async après edit | LandscapeEdit éditeur ; visuel shipping ouvert (E5) | A : HoleFix manuel + code mesh dirty. B : E5 / F1 | A : `DEMONTRÉ_MANUELLEMENT` 1 chunk. B : `INSPECTÉ_DANS_LE_CODE` + F1 INVALIDÉ |
| Collision et délais | Debounce ~0,2 s, 1/frame, LOD NoCollision | PMC stride 16, cooldown 0,35 s ; historiquement inactif | A : utilisateur HoleFix + code. B : header collision | A intra-debounce `INCONNU`. B G5 `INCONNU` |
| Opérations concurrentes | Pas de réservation sommet ; chunks unload = edit ignoré ; undo par joueur | Store game-thread additif ; tâches C-04/C-11 hors vertex | Code | Multi-unités RTS `INCONNU` des deux côtés. A pire si streaming |
| Précision et résolution | Uniforme/chunk, clamp 32–256 ; démo ~107,5 UU | Grille Landscape/Store 4033² ; QuadSize défaut 100 | Code + HISTORIQUE | 25 cm île : A structurellement borné/chunk. B : pas 25 cm non plus **démontré**. `INCONNU` / CALC |
| Peintures et couches | 4 poids plugin ; overlay host démo | Paint plat Store, sémantique hors C-01 (D14) | TF-07 ; Store.cpp | A ≠ couches M4 (D). B paint ≠ sol (C-16) |
| Sauvegarde et migration | SaveGame v2 plugin, host démo non câblé | `SaveToSlot` Store, flag OFF, politique C-19 | SaveGame.h ; Settings GF | Tous deux **NON VALIDÉ** runtime. Formats **distincts** |
| Streaming et extension | LoadRadius / GridExtent / cache | Landscape île unique actuelle | Settings + Manager | A : edits hors chunks actifs perdues. B : pas de tuilage TerraDyne |
| Sols et propriétés physiques | Absents (poids/herbe ≠ C-16) | C-16 conception ; stub moisture | Contrats + Spatial.cpp | A : `INCOMPATIBILITÉ_DÉMONTRÉE` si on prend les poids pour du sol. B : `POSSIBLE_SOUS_CONDITIONS` (contrat existe, runtime non) |
| Simulation de l’eau | Absente | C-15 conception ; stub depth | Idem | A n’offre pas d’autorité eau. B préserve la frontière D4 |
| Évolutivité des écosystèmes | Grass/population plugin ≠ C-17 | C-17 conception uniquement | Contrats | Les deux `INCONNU` runtime. A mélange présentation végétale et relief |
| Dépendance au plugin | Totale (vérité = OSS) | Nulle pour l’autorité ; optionnelle pour le mesh | Architecture | A : lock-in OSS/GPU stub. B : coût si mesh TD ajouté |
| Compatibilité contrats VALIDÉ | Conflit D1, D2, 08.6, C-02 F1 | Alignée D1–D14, 08.6 ; 08.7 ouvert pour la **présentation** | designGate.js, C-01 | A : `INCOMPATIBILITÉ_DÉMONTRÉE`. B : `INSPECTÉ_DANS_LE_CODE` + textes VALIDÉ |
| Coût d’adaptation initial | Remplacer/wrap Store ; réécrire lecteurs ; save ; spatial | Continuer Store ; 08.7 pour E5/G5 | Inspection | **Non chiffré**. A = amendements de contrats + rewrite. B = travail déjà cadré E5/E6 |
| Coût de maintenance | Fork/comportement OSS, dual paint, streaming | Un store GF ; présentation pluggable (D3) | — | A élevé si GF avance sols/eau. B élevé si double mesh TD mal séparé |
| Risque de refonte future | Élevé : C-15/C-16 sur mauvaise autorité | Modéré : changer la **présentation** (08.7) sans changer D1 | — | Hybride non prouvé : risque de double vérité |

---

## 8. Confrontation aux contrats GardenFervor

| Id | Intitulé | Statut constaté | Portée pour cet audit |
|---|---|---|---|
| C-01 | Terrain runtime | VALIDÉ documentaire | D1 autorité Store ; D2 succès Store ; D3 présentation non imposée ; D4 relief seul ; D12 persistable via C-19 |
| C-02 | Substrat spatial | VALIDÉ | F1 jamais propriétaire de la hauteur ; queries via C-01 |
| C-04 / C-07 / C-08 / C-11 | Tâches, agents, terraformer, réservations | VALIDÉ | Commandent le travail ; ne sont pas l’autorité hauteur |
| C-15 | Hydrologie | VALIDÉ conception | Lit le relief C-01 ; n’écrit pas le Store |
| C-16 | Sol | VALIDÉ conception | Idem ; paint ≠ sol |
| C-17 | Végétation / écosystèmes | VALIDÉ conception | Consomme relief/sol/eau ; ≠ herbe TerraDyne |
| C-19 | Persistance | VALIDÉ conception | Politique ; donnée hauteur = C-01 |
| DG-08.6 | Principes Base+Delta | VALIDÉ | Store autorité après terraform |
| **DG-08.7** | Architecture technique | **À DÉFINIR** | **Aucune techno retenue** — cet audit n’y change rien |

**08.7 n’est pas une décision adoptée.** WPO, Landscape runtime, géométrie tuilée : explicitement **non choisis**.

Option A **ne peut pas** être retenue sans **réécrire** D1/08.6. Ce rapport ne le fait pas.

Écarts C-01 E1–E8 : informatifs. Le code lu pour E1/E2 peut avoir divergé de la table ; **cela n’autorise pas** à clôturer ces dettes ici.

---

## 9. Architecture conceptuelle recommandée (si les preuves la justifient)

**Autorité unique :** `RuntimeTerraformStore` (C-01).

**Séparation justifiée par les textes et le code :**

1. Données géométriques — Store GF  
2. Commandes / ordonnancement — C-04 / C-07 / C-08 / C-11  
3. Représentation — hors D1 (D3) ; Landscape+M4 = référence d’étude 08.7, pas un choix  
4. Collision — doit suivre le relief réel (00.5.G5) ; **techno non retenue**  
5. Peintures techniques — C-01 D14 ; ≠ sol  
6. Sol — C-16  
7. Eau / éco — C-15 / C-17  
8. Persist / reconstruction — C-19 sur données C-01 (et autres propriétaires)

TerraDyne, dans cette architecture, n’est **pas** l’autorité. Un rôle mesh/collision **éventuel** reste une candidate 08.7 **non démontrée** (adaptateur `Store → LoadFromData` manquant ; M4 D).

Aucun schéma implémentable n’est spécifié au-delà de cette séparation. **Aucun code créé.**

---

## 10. Verdict, réserves et risques

### Issue principale : **Option B recommandée**

GardenFervor doit **conserver** l’autorité de terrain.

1. **Preuves déterminantes :** C-01 D1, 08.6, 00.5.S4, 00.6.A2, C-02 F1 d’une part ; moteur TerraDyne couplé buffers/mesh/collision et `ApplyGlobalBrush` void d’autre part. HoleFix prouve un proto jouable, **pas** une autorité GF.
2. **Compromis :** B laisse E5/F1/G5/persist ouverts. A offrirait un mesh/collision déjà vus sur un chunk, au prix d’un conflit de contrats et d’un lock-in OSS.
3. **Sauvegarde et multi-unités :** aucun des deux n’a de ronde-trip persist VALIDÉ ni de preuve RTS. A perd les edits sur chunks unload ; undo plugin ≠ graphe C-04.
4. **Sols et eau :** B conserve les frontières D4. A n’a pas de modèle C-15/C-16 ; utiliser les poids comme sol serait une confusion démontrable.
5. **Conditions préalables à une intégration TerraDyne (si un jour) :** décision humaine 08.7 ; adaptateur Store→chunk **sans** `ApplyGlobalBrush` ; preuve G5 ; politique C-19 ; audit 3 M4 si visuel M4 requis. **Non ouvertes par ce rapport.**
6. **Vérifications qui pourraient faire bouger le verdict d’autorité :** uniquement une **amendement explicite** de C-01 D1 / 08.6 par l’humain. Des tests perf ou 25 cm **ne suffisent pas** à légitimer l’option A tant que D1 nomme le Store.
7. **Hybride strictement séparé :** unicité d’autorité = oui **si** Store seul écrit. Faisabilité de la séparation mesh = **non démontrée**. Donc **non recommandée** comme issue principale.

**Réserves :** recommandation d’audit ≠ 08.7 VALIDÉ ≠ autorisation E5/E6 ≠ PASS F1.

**Cinq risques majeurs :**

1. Option A = double vérité ou violation D1.  
2. Option B + TerraDyne mesh sans adaptateur = désynchronisation.  
3. Persist : deux formats, aucun VALIDÉ.  
4. Streaming TerraDyne vs travaux autonomes hors chunks chargés.  
5. Confondre paint TerraDyne / M4 / sol C-16.

---

## 11. Tests ou vérifications restant à effectuer

Aucun n’est exécuté par cet audit.

| Id | Objectif | Peut changer quoi |
|---|---|---|
| FUT-04 | Save/Load Store GF et, séparément, SaveWorld proto | Pas l’autorité D1 ; éclaire C-19 |
| FUT-05 | Collision intra-debounce HoleFix | G5, pas D1 |
| FUT-06 | Budget frame île | 08.7 présentation, pas D1 |
| FUT-07 | 25 cm multi-chunks | Faisabilité détail, pas transfert d’autorité |
| A02-V1 | Spécifier (doc seulement, si autorisé) l’adaptateur Store→`FTerraDyneChunkData` | Hybride présentation |
| A02-V2 | Réconcilier table E1/E2 C-01 vs code actuel | Dette documentaire GF |
| AUDIT-03 | M4 | Reste **D** jusqu’à preuve contraire |

---

## 12. Limites de l’audit et sources inaccessibles

- Pas de relance HoleFix ni de nouveau log.  
- Host démo TerraDyne non ré-instrumenté (PaintStore) : s’appuie sur AUDIT-01 / DEMO.  
- QuadSize **réel** de l’île GF en éditeur : lu comme défaut 100 + bake ; la valeur cooked exacte du DataAsset n’a pas été ouverte ici.  
- PHASE2B : résultats de précision **sur le proto**, pas sur GardenFervor Island.  
- Produit TerraDyne commercial / VHM : hors OSS inspecté (déjà noté PHASE3A).  
- Les deux dépôts étaient accessibles. Aucune conclusion n’a été limitée pour inaccessibilité.

---

## 13. Périmètre de modification documentaire

Autorisé et effectué : rapport canonique AUDIT-02, README d’audit, registre, README TerraDyne, findings, tests de suivi, plus ETAT / HISTORIQUE / REGISTRE de maintenance si §1bis l’exige.

**Non modifié :** code, assets, config gameplay, proto TerraDyne, `designGate.js`, contrats, `REGLES_PROJET.md`. Pas de commit.

`Design Gate : aucune modification de contrat ou de statut — audit architectural et documentation uniquement.`

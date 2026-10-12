# TerraDyne — périmètre et baseline

Date de ce document : **2026-10-12**. Aucune mesure nouvelle n’a été effectuée pour le rédiger.

## Prototype (externe à GardenFervor)

| Champ | Valeur |
|---|---|
| Racine | `C:\Users\sahel\Documents\Unreal Projects\TerraDyneOSS_Proto_270c311` |
| Plugin | TerraDyne OSS commit **`270c311`** |
| Engine | Unreal Engine **5.8.3** (CL 58210709, session de travail du proto) |
| Host | `TerraDyneGardenDemoHost` + GameMode de démo |
| GPU brushes plugin | `bEnableGPUBrushes=false` (stub GPU) |

## Builds conservés (ne pas écraser)

Les chemins sont relatifs à la racine du proto. **Aucun de ces dossiers n’a été copié ici.**

| Dossier | Rôle |
|---|---|
| `Packaged/Win64Shipping` | Témoin crash graphique Shipping |
| `Packaged/Win64ShippingFixed` | Premier boot GPU après correctif vertex color |
| `Packaged/Win64ShippingLowerFix` | Self-test API Lower (clamp HeightBuffer) |
| **`Packaged/Win64ShippingHoleFix`** | **Build final déclaré** — `TerraDyneOSS_Proto.exe` |

Un résultat d’un build antérieur n’est pas une preuve du build HoleFix, sauf mention explicite.

## Hors périmètre de l’évaluation actuelle

- Intégration dans le projet GardenFervor
- Modification du plugin ou du host proto (cette organisation documentaire ne les touche pas)
- Audit 2 : **exécuté le 2026-10-12** (documentaire) — voir `01_Audits/AUDIT-02/REPORT.md` ; 08.7 **non** modifié
- Audit 3 : **exécuté le 2026-10-12** — `01_Audits/AUDIT-03/REPORT.md` ; M4 **D conservé** ; 08.7 **non** modifié
- Performance île RTS complète
- Persist runtime GardenFervor / ODC-F1
- Simulations sol / eau / végétation

## Design Gate (rappel, non modifié)

- **08.6** : Store = autorité hauteur après terraform (principes VALIDÉ)
- **08.7** : architecture technique **À DÉFINIR** — aucune techno retenue
- ODC-F1 : **INVALIDÉ / À REFAIRE**

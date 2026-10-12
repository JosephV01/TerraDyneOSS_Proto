# M4_Eval — copies locales (pas un junction GardenFervor)

Copies fichiers uniquement, destinées à l'essai **MIC Landscape M4 sur DynamicMesh TerraDyne**.

## Copié depuis GardenFervor (lecture, copie, aucune écriture GF)

- `Island_UE5Preset5_Inst.uasset` — MIC île (`/Game/M4/Materials/PresetInstances/Island/Island_UE5Preset5_Inst`)
- Layer Info whitelist GardenFervor + Auto, depuis `Content/M4/Maps/PaintWeights/Island_Demo_sharedassets/` :
  Auto, Grass, Dirt, Sand, Mud, Forest, Stones, Jagged_Rock, Flat_Ice, Custom01, Custom02

## Non copié volontairement

- Plugin M4 / `M4UE5MasterMat` / fonctions PaintLayers / textures lourdes
- Un MIC Landscape M4 **n'est pas prévu** pour un `UDynamicMeshComponent` (doc officielle : peinture = Landscape Paint Layers)

Les soft-refs du MIC pointent encore vers `/Game/M4/...`. Sans plugin M4 dans ce proto, le chargement doit échouer ou produire un matériau cassé (rose/noir). C'est une **preuve d'incompatibilité directe**, pas un défaut à « réparer ».

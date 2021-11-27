## TODO

### 0.5.x
### Add

- Display hovered index and selection length in the informationLabel
    - Index: x, Selection length: y
- Warn about modified files before closing them.

- Implement backup files (.bak) when editing PAL/TRN
    - A copy of the PAL/TRN file is created automatically (e.g. redv.trn.bak)
        - Only if it does not exist and only when saving
    - Add setting to enable/disable the feature (enabled by default)

- Add export options for level tiles to be compatible with Tiled
    - https://github.com/diasurgical/modding-tools/tree/master/Tiled
    - Special tiles must be merged into the spritesheet (for the right tiles)
    - Adjust line/column number
- Add export options for monsters compatible with Tiled
    - https://github.com/sergi4ua/opensource-ami/blob/master/gen_monsters_tileset.sh
    - https://github.com/diasurgical/modding-tools/blob/3620efe13a62792a5b02f107cda86d68d19f5cdf/Tiled/extensions/diablo-dun.js#L257
- Add export options for item
    - https://github.com/sergi4ua/opensource-ami/blob/master/gen_objects_tileset.sh
    - https://github.com/diasurgical/modding-tools/blob/3620efe13a62792a5b02f107cda86d68d19f5cdf/Tiled/extensions/diablo-dun.js#L330

- Add CI/CD with appimage build for Linux (and .msi for Windows?).
    - Update PowerShell release script to take a Qt build folder as parameter.
    - https://github.com/mapeditor/tiled/blob/master/.github/workflows/ubuntu-20.04.yml
    - https://github.com/mapeditor/tiled/blob/master/.github/workflows/packages.yml


- Add CEL/CL2/PAL/TRN mappings in a JSON file for diablo and hellfire (retail version).
- Implement shadow selection when picking translation.

- Add settings
    - Background color? (grey, green, magenta, cyan?)
    - Default zoom level (depending on CEL/CL2 type?)
        - By default: x2
        - Automatic dezoom when opening a bigger image?

### Change

- Remove .pcx and .gif from the open dialog until it is implemented.
- Rewrite level CEL frame type detection to leverage associated MIN file when available.
- Port to .NET 6??

### Fix

- File export for Linux
- Last color of PAL/TRN not displayed as transparent??

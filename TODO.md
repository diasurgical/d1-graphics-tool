## TODO

### 0.5.x
### Add

- Warn about modified files before closing them.

- Add export options for level tiles to be compatible with Tiled
    - Special tiles must be merged into the spritesheet (for the right tiles)
    - Adjust line/column number
- Add export options for monsters compatible with Tiled

- Add CI/CD with appimage build for Linux (and .msi for Windows?).
    - Update PowerShell release script to take a Qt build folder as parameter.

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

### Fix

- File export for Linux
- Last color of PAL/TRN not displayed as transparent??

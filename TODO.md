## TODO

### Add
- PowerShell release script which takes a Qt build folder as parameter
- Preferences dialog (with JSON config file?)
    - Default work folder
    - Background color? (grey, green, magenta, cyan?)
    - Default zoom level (depending on CEL/CL2 type?)
        - By default: x2
        - Automatic dezoom when opening a bigger image?
    - Default palette size? (192px, 256px)
- Application icon
- PCX support
- GIF viewing support

### Change
- Rework PAL and TRN support
    - Add 3 additional PalView with selector to choose for frame, sub-tiles, tiles or whole cel/cl2 file.
    - Build default.pal with first 128 bytes with a specific color.
    - Build additional palettes
        - Last color with specific color to identify transparent zones
        - Second half with specific color?
        - One for each hue in a specific color?
    - Include cel/cl2 to trn mapping in the program, especially for monsters.
- Rewrite level CEL frame type detection to leverage associated MIN file when available.

### Fix
- Last color of pal/trn not displayed as transparent

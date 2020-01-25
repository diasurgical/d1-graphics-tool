## TODO

### Add
- PowerShell release script which takes a Qt build folder as parameter
- Add settings
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
    - Build additional palettes
        - Last color with specific color to identify transparent zones
        - Second half with specific color?
        - One for each hue in a specific color?
    - Include cel/cl2 to trn mapping in the program, especially for monsters.
- Remove CelView and LevelCelView dependencies from PalView by leveraging signals/slots.
- Rewrite level CEL frame type detection to leverage associated MIN file when available.

### Fix
- Last color of pal/trn not displayed as transparent??

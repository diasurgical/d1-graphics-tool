## TODO

### Add
- Write a PowerShell release script which takes a Qt build folder as parameter
- Add preferences dialog (with JSON config file?)
    - Default work folder
    - Background color? (grey, green, magenta, cyan?)
    - Default zoom level (depending on CEL/CL2 type?)
        - By default: x2
        - Automatic dezoom when opening a bigger image?
    - Default palette size? (192px, 256px)

- Add PCX support
- Add GIF viewing support

### Change
- Rework PAL and TRN support
    - Add 2x3 additional PalView.
        - For frame palette hits
        - For global palette hits
    - Build default.pal with first 128 bytes with a specific color.
    - Include cel/cl2 to trn mapping in the program, especially for monsters.
- Rewrite level CEL frame type detection to leverage associated MIN file when available.

### Fix

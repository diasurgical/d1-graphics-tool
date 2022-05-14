## TODO

### 0.5.0
1. Implement multi-color selection in palette widgets
2. Implement undo/redo stack
3. Show all colors when clicking "Pick" button
4. Warn about modified files before closing them

### 0.6.0


### Add
- Add settings
    - Background color? (grey, green, magenta, cyan?)
    - Default zoom level (depending on CEL/CL2 type?)
        - By default: x2
        - Automatic dezoom when opening a bigger image?
- PowerShell release script which takes a Qt build folder as parameter
- Application icon
- PCX support
- GIF support

### Change
- Include CEL/CL2 to TRN mapping in the program, especially for monsters.
- Remove CelView and LevelCelView dependencies from PalView by leveraging signals/slots.
- Rewrite level CEL frame type detection to leverage associated MIN file when available.

### Fix
- Last color of PAL/TRN not displayed as transparent??

## TODO

### 0.5.x
### Add

- Display hovered index and selection length in the informationLabel
    - Index: x, Selection length: y
- Warn about modified files before closing them.
- Implement backup files (.bak) when editing PAL/TRN
    - A copy of the PAL/TRN file is created automatically (e.g. redv.trn.bak)
        - Only if it does not exist and only when saving
    - Add associated setting in the settings dialog
- Add CEL/CL2/PAL/TRN mappings in a JSON file for diablo and hellfire (retail version).
- Implement shadow selection when picking translation.

- Add settings
    - Background color? (grey, green, magenta, cyan?)
    - Default zoom level (depending on CEL/CL2 type?)
        - By default: x2
        - Automatic dezoom when opening a bigger image?
- PowerShell release script which takes a Qt build folder as parameter

### Change
- Rewrite level CEL frame type detection to leverage associated MIN file when available.

### Fix
- Last color of PAL/TRN not displayed as transparent??

## TODO

### Add
- PAL write support
    - Individual color selection (click the color)
        - First color of the palette is selected by default
    - Color hue selection
        - Checkbox next to the hue line?
        - Make hue offsets configurable in the JSON config file
    - Display color hex values when selecting a color
    - Display selected color offset
    - Open color picker when double-clicking a color
    - "Save" and "Save as" items in the "Palette" menu
- TRN write support
    - Inherit palette view (with selection capabilities)
    - Display tranlated color offset
    - Add "Reset" button
    - Add "Translate" button
    - Translate individual color by double-clicking (or single-click plus "Translate" button) the color to translate and clicking the target color
    - Translate hue by selecting the hue to translate, click the "Translate" button and click the target hue (hue must be the same size)
    - "Save translation 1", "Save translation 2", "Save translation 1 as", "Save translation 2 as" items in the "Palette" menu
- Rescale the palette when changing screen depending on DPI
- Add settings
    - Background color? (grey, green, magenta, cyan?)
    - Default zoom level (depending on CEL/CL2 type?)
        - By default: x2
        - Automatic dezoom when opening a bigger image?
    - Default palette size? (192px, 256px)
- PowerShell release script which takes a Qt build folder as parameter
- Application icon
- PCX support
- GIF support

### Change
- Remove palette hit views, instead add this functionality in the default palette view (dropdown?).
- Rename "Load" to "Open" in the "Palette" menu.
- Include CEL/CL2 to TRN mapping in the program, especially for monsters.
- Remove CelView and LevelCelView dependencies from PalView by leveraging signals/slots.
- Rewrite level CEL frame type detection to leverage associated MIN file when available.

### Fix
- Last color of PAL/TRN not displayed as transparent??

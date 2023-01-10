# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
- Configurable playback speed
- Palette cycling animation of Diablo 1 and Hellfire
- Save menu option to store the current graphics
- Save as menu option to convert between cel/cl2, add new groups or store to a different location
- Open as menu option to open files with parameters
  1. use width 96 to open wlbat.cl2, whbat.cl2 and wmbat.cl2 graphics of the warrior
  2. use height of 8 to open the hellfire town tileset
- New menu option to open a clean cel, cl2 or tileset graphics
- Drag and drop support
- Edit fields to modify subtiles of tiles, frames of subtiles, frame-types
- Subtile height is editable
- Button to apply trn-adjustments of the game (done to normal monster-trns)
- Context menu + menu option to add, insert, delete or replace frames
- Context menu + menu option to create, add, insert, delete or replace tiles and subtiles
- Context menu to undo/redo the modifications of the palette/translation
- Icon buttons to create/load/save palette/translation in place
- File dialogs start from the last used folder/file (even after restart)
- Export to any kind of image format which are supported by Qt (JPEG, WEBP, etc...)
- Option to limit the range of the exported items

### Fixed
- Memory leaks
- Bunch of bugfixes

## 0.5.0 - 2021-08-12
### Added
- Color palette (PAL) write support.
- Color translation (TRN) write support.
- Multi-selection support in the palette widgets.
- Color editing in the palette widget.
- Translation editing in the palette widgets.
- "Show translated colors" display filter for color translations palette widgets.
- CEL level tiles can now be clicked to select the corresponding sub-tile.
- CEL level sub-tiles can now be clicked to select the corresponding frame.
- CEL/CL2 frames can now be clicked to select the corresponding color in the palette widgets.
- Cycling through tiles, sub-tiles, frame groups and frames is now allowed when clicking previous/next on first/last item.
- New setting for palette default color.
- New setting for selection border color.
- Tooltip to display full path of PAL/TRN files when hovering the path dropdown list.
- Application icon.

### Changed
- Qt Framework updated to 6.1.2.
- Palette view is replaced by three palette widgets (one for the palette and two for translations).
- Palette hits are now displayed in the same graphic view as colors through a display mask mechanism.
- Translation 1 and 2 have been swapped and renamed "Translation" and "Unique translation"; unique translation applies first.

### Removed
- town.pal (_town.pal) from the application resource file.

### Fixed
- CEL/CL2 group and frame button alignments.
- Level CEL tile, sub-tile and frame button alignments.

## 0.4.1 - 2021-03-11
### Changed
- Qt Framework updated to 5.15.2 LTS.

### Fixed
- CL2 loading issue, the top pixel line of CL2 frames was not loaded nor rendered.


## 0.4.0 - 2020-01-08
### Added
- Palette hits view for all frames and current frame.
- Palette translation hits view for all frames and current frame.
- Palette hits view for current tile and current sub-tile when displaying a level CEL.
- JSON configuration file and corresponding settings dialog.
- Working folder setting.
- Status bar message when opening file.

### Changed
- Default palette from town.pal to builtin _default.pal.
- Default palette translation to _null.trn.

### Fixed
- Export dialog button height.

## 0.3.2 - 2020-01-08
### Changed
- Qt Framework updated to 5.12.6 LTS.
- Rewrite changelog.

### Fixed
- Fix palette display bug (unexpected crop).

## 0.3.1 - 2018-03-09
### Changed
- Qt Framework updated to 5.9 LTS.
- Code cleaning.

## 0.3.0
### Added
- Automatic TRN listing.
- BMP and PNG export support (multi-file or sprites).

### Changed
- Cleaned CelFrameBase constructor.
- Optimized TIL QImage rendering by adding and using tile width and pixel width/height.

### Fixed
- Bug fix for Type 2, 3, 4, 5 frames rendering.
- Bug fix for automatic PAL loading.
- Bug fix for mono-group CEL/CL2 files.

## 0.2.4
### Added
- Zoom support.
- CEL/CL2 group based playing support.
- Incomplete export support (only GUI).

## 0.2.3
### Added
- MIN and TIL viewing support for level CEL files.

### Fixed
- Bug fix for Type 2 and 3 frames detection.

## 0.2.2
### Added
- Double TRN support.
- Automatic PAL listing/loading.

## 0.2.1
### Added
- Full CL2 viewing support.

## 0.2.0
### Added
- Full CEL viewing support (including level CEL files).

### Changed
- Object model modified so D1Cel and D1Cl2 classes both inherit D1CelBase.
- CelView modified to allow CEL compilations and CL2 groups browsing.

## 0.1.3
### Added
- Incomplete CEL viewing support , new algorithm, only level CEL files are not displayed.

## 0.1.2
### Added
- Full TRN viewing support.

## 0.1.1
### Added
- Incomplete CEL viewing support, new algorithm, only level CEL files are not displayed.

## 0.1.0
### Added
- Full PAL viewing support.
- Incomplete CEL viewing support.

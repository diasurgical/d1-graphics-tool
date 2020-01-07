# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## 0.3.2 - 2020-01-07
### Changed
- Start using Qt Framework 5.12 LTS.
- Rewrite changelog.

## 0.3.1 - 2018-03-09
### Changed
- Qt Framework 5.9 LTS.
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

# libavif-container

[![Unit tests](https://github.com/link-u/libavif-container/workflows/Unit%20tests/badge.svg)](https://github.com/link-u/libavif-container/actions?query=workflow%3A%22Unit+tests%22)

A library to parsing and writing avif container.

(It is not for neither decoding nor encoding.)

## Related repositories

- [link-u/davif](https://github.com/link-u/davif) - avif decoder, using dav1d directly.
- [link-u/cavif](https://github.com/link-u/cavif) - avif encoder, using libaom directly.
- [link-u/avif-sample-images](https://github.com/link-u/avif-sample-images) - sample images from us.
- [AOMediaCodec/av1-avif](https://github.com/AOMediaCodec/av1-avif/tree/master/testFiles) - test files from Microsoft and Netflix.

# How to use

## To clone this repo

```bash
git clone --recurse-submodules --recursive https://github.com/link-u/libavif-container.git
```

## From CMake

We suggest you to vendor this library to your repo using `git submodule add`.

```cmake
# example: git submodule add <this repo> external/libavif-container
add_subdirectory(external/libavif-container)

# libavif-container depends on C++17 features,
# so we strongly recommended you to use C++17 or higher.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# link to your library or executable
target_link_libraries(<your-target> libavif-container)
```

# License

MIT

# References

## AV1 / AVIF
- AV1
  - [AV1 Bitstream & Decoding Process Specification](https://aomediacodec.github.io/av1-spec/)
- AVIF
  - [AV1 Image File Format (AVIF)](https://aomediacodec.github.io/av1-avif/)
  - [AV1 Codec ISO Media File Format Binding](https://aomediacodec.github.io/av1-isobmff/)
- Others
  - [AVIF2018.pdf](https://people.xiph.org/~negge/AVIF2018.pdf)
  - [A Technical Overview of AV1 | IEEE](https://ieeexplore.ieee.org/document/9363937)

## ISOBMFF
- [ISO - ISO/IEC 14496-12:2015  
  Information technology - Coding of audio-visual objects  
  Part 12: ISO base media file format](https://www.iso.org/standard/68960.html)
- [ISO BMFF Byte Stream Format -  
  W3C Working Group Note 04 October 2016](https://www.w3.org/TR/mse-byte-stream-format-isobmff/)

## HEIF - MPEG-H Part 12 (ISO/IEC 23008-12)
- [Information technology — High efficiency coding and media delivery in heterogeneous environments — Part 12: Image File Format](https://www.iso.org/standard/66067.html)
- [HEIF Technical Information - High Efficiency Image File Format](https://nokiatech.github.io/heif/technical.html)
- [nokiatech/heif: High Efficiency Image File Format](https://github.com/nokiatech/heif/)

## MIAF - MPEG-A Part 22 (ISO/IEC 23000-22:2019)
- [ISO/IEC 23000-22:2019<br>Information technology — Multimedia application format (MPEG-A) — Part 22: Multi-image application format (MIAF)](https://www.iso.org/standard/74417.html)
  - [ISO/IEC 23000-22:2019/Amd 1:2021](https://www.iso.org/standard/80757.html)
  - [ISO/IEC 23000-22:2019/Amd 2:2021](https://www.iso.org/standard/81634.html)

## ComplianceWarden

- [gpac/ComplianceWarden: A pluggable ISOBMFF (MP4/IFF) compliance checker](https://github.com/gpac/ComplianceWarden)
  - [MPEG MIAF Conformance Software Wasm demo](https://gpac.github.io/ComplianceWarden-wasm/)

# libavif-container

A library to manipulate avif container. It's for neither decoding nor encoding.

Currently, it just supports **parsing** avif container. However, it also is planned to support **writing** avif files (not yet...).

# How to use

## From CMake

We suggest you to vendor this library to your repo using `git submodule add`.

```cmake
# example: git submodule add <this repo> external/libavif-container
add_subdirectory(external/libavif-container)

# libavif-container depends on C++17 features,
# so we strongly recommended you to use C++17 or higher.
set(CMAKE_CXX_STANDARD 17)

# link to your library or executable
target_link_libraries(<your-target> libavif-container)
```

# License

MIT

# References

## ISOBMFF
 - ISO - ISO/IEC 14496-12:2015  
   Information technology - Coding of audio-visual objects  
   Part 12: ISO base media file format  
   https://www.iso.org/standard/68960.html
 - ISO BMFF Byte Stream Format -  
   W3C Working Group Note 04 October 2016  
   https://www.w3.org/TR/mse-byte-stream-format-isobmff/
 - avif.js/mov.js - Kagami/avif.js
   https://github.com/Kagami/avif.js/blob/500e3357e5d56750dbd4e40b3aee74b13207721e/mov.js
 - MP4のデータ構造 - Qiita  
   https://qiita.com/daisukeoda/items/87f6b5e9427c3feeaabf

## AV1
 - AV1 Image File Format (AVIF)  
   https://aomediacodec.github.io/av1-avif/
 - AV1 Codec ISO Media File Format Binding  
   https://aomediacodec.github.io/av1-isobmff/

## HEIF - MPEG-H Part 12 (ISO/IEC 23008-12)
 - nokiatech/heif: High Efficiency Image File Format  
   https://github.com/nokiatech/heif/
 - Text of ISO/IEC CD 23008-12 Image File Format | MPEG  
   https://mpeg.chiariglione.org/standards/mpeg-h/image-file-format/text-isoiec-cd-23008-12-image-file-format

## AVIF
 - [AVIF2018.pdf](https://people.xiph.org/~negge/AVIF2018.pdf)

cmake_minimum_required(VERSION 3.10)
project(libavif-container)

add_library(
        libavif-container
        # 3rd party
        external/tinyformat/tinyformat.h
        # sources
        src/avif/util/Logger.hpp
        src/avif/util/Logger.cpp
        src/avif/util/FileLogger.cpp
        src/avif/util/FileLogger.hpp
        src/avif/Parser.cpp
        src/avif/Parser.hpp
        src/avif/Box.hpp
        src/avif/FullBox.hpp
        src/avif/FileBox.hpp
        src/avif/FileTypeBox.hpp
        src/avif/MetaBox.hpp
        src/avif/HandlerBox.hpp
        src/avif/MediaDataBox.hpp
        src/avif/ItemLocationBox.hpp
        src/avif/ItemPropertiesBox.hpp
        src/avif/ItemPropertyContainer.hpp
        src/avif/PixelAspectRatioBox.hpp
        src/avif/ImageSpatialExtentsProperty.hpp
        src/avif/PixelInformationProperty.hpp
        src/avif/ItemPropertyAssociation.hpp
        src/avif/AV1CodecConfigurationBox.hpp
        src/avif/ItemInfoBox.hpp src/avif/ItemInfoEntry.hpp src/avif/ItemInfoExtension.hpp)

set_property(TARGET libavif-container PROPERTY CXX_STANDARD 17)
set_property(TARGET libavif-container PROPERTY CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")

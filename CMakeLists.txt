cmake_minimum_required(VERSION 3.10)
project(libavif-container LANGUAGES CXX)

add_library(
    libavif-container
    # sources
    src/avif/util/File.cpp
    src/avif/util/File.hpp
    src/avif/util/Logger.hpp
    src/avif/util/Logger.cpp
    src/avif/util/FileLogger.cpp
    src/avif/util/FileLogger.hpp
    src/avif/util/FourCC.cpp
    src/avif/util/FourCC.hpp
    src/avif/util/StreamReader.cpp
    src/avif/util/StreamReader.hpp
    src/avif/util/StreamWriter.cpp
    src/avif/util/StreamWriter.cpp

    src/avif/img/color/Math.hpp
    src/avif/img/color/Constants.hpp
    src/avif/img/color/Matrix.hpp

    src/avif/img/Image.hpp
    src/avif/img/Conversion.hpp
    src/avif/img/Transform.hpp
    src/avif/img/TransformImpl.hpp
    src/avif/img/Crop.hpp

    src/avif/math/Fraction.hpp

    src/avif/Constants.hpp
    src/avif/Constants.cpp

    src/avif/Box.hpp
    src/avif/FullBox.hpp
    src/avif/FileBox.hpp
    src/avif/FileTypeBox.hpp
    src/avif/MetaBox.hpp
    src/avif/HandlerBox.hpp
    src/avif/MediaDataBox.hpp
    src/avif/ItemLocationBox.hpp
    src/avif/PrimaryItemBox.hpp
    src/avif/ItemPropertiesBox.hpp
    src/avif/ItemPropertyContainer.hpp
    src/avif/PixelAspectRatioBox.hpp
    src/avif/ImageSpatialExtentsProperty.hpp
    src/avif/PixelInformationProperty.hpp
    src/avif/RelativeLocationProperty.hpp
    src/avif/AuxiliaryTypeProperty.hpp
    src/avif/CleanApertureBox.hpp
    src/avif/ItemPropertyAssociation.hpp
    src/avif/ImageRotationBox.hpp
    src/avif/ImageMirrorBox.hpp
    src/avif/ColourInformationBox.hpp
    src/avif/ContentLightLevelBox.hpp
    src/avif/MasteringDisplayColourVolumeBox.hpp
    src/avif/AV1CodecConfigurationBox.hpp
    src/avif/ItemInfoBox.hpp
    src/avif/ItemInfoEntry.hpp
    src/avif/ItemInfoExtension.hpp
    src/avif/ItemReferenceBox.hpp

    src/avif/Parser.cpp
    src/avif/Parser.hpp
    src/avif/Writer.cpp
    src/avif/Writer.hpp
    src/avif/Query.hpp

    src/avif/av1/Header.hpp
    src/avif/av1/SequenceHeader.hpp
    src/avif/av1/TemporalDelimiter.hpp
    src/avif/av1/Padding.hpp

    src/avif/av1/Parser.cpp
    src/avif/av1/Parser.hpp
    src/avif/av1/Query.hpp
    src/avif/av1/BitStreamReader.cpp
    src/avif/av1/BitStreamReader.hpp
)

# https://cmake.org/cmake/help/latest/manual/cmake-compile-features.7.html#requiring-language-standards
# https://stackoverflow.com/questions/45688522/how-to-enable-c17-in-cmake
target_compile_features(libavif-container PRIVATE cxx_std_17)
set_property(TARGET libavif-container PROPERTY CXX_STANDARD 17)
set_property(TARGET libavif-container PROPERTY CXX_STANDARD_REQUIRED ON)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set_property(TARGET libavif-container PROPERTY CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # FIXME(ledyba-z): workaround for gcc-8
  target_link_libraries(libavif-container PRIVATE stdc++fs)
endif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
###############################################################################
option(LIBAVIF_CONTAINER_EMBED_FMT "Embed fmt::fmt" ON)
IF(LIBAVIF_CONTAINER_EMBED_FMT)
  # fmt
  set(FMT_TEST OFF CACHE BOOL "" FORCE)
  set(FMT_FUZZ OFF CACHE BOOL "" FORCE)
  add_subdirectory(external/fmt EXCLUDE_FROM_ALL)
  target_link_libraries(libavif-container PUBLIC fmt::fmt)
  target_include_directories(libavif-container PUBLIC fmt::fmt)
ELSE(LIBAVIF_CONTAINER_EMBED_FMT)
  #FIXME(ledyba-z): How should we do?
ENDIF(LIBAVIF_CONTAINER_EMBED_FMT)
###############################################################################
option(LIBAVIF_CONTAINER_BUILD_TESTS "Build the tests" OFF)
if(LIBAVIF_CONTAINER_BUILD_TESTS)
  enable_testing()
  include(GoogleTest)
  add_subdirectory(external/gtest EXCLUDE_FROM_ALL)

  file(GLOB_RECURSE SRC_FILES src/*.hpp)
  add_executable(libavif-container-tests
      ${SRC_FILES}
      test/av1/ParseTest.cpp
      test/math/FractionTest.cpp
      test/ColorTest.cpp
  )
  target_link_libraries(libavif-container-tests PRIVATE libavif-container)
  target_link_libraries(libavif-container-tests PRIVATE gtest)
  target_link_libraries(libavif-container-tests PRIVATE gtest_main)
  target_include_directories(libavif-container-tests PRIVATE external/gtest/googletest/include)
  set_property(TARGET libavif-container-tests PROPERTY CXX_STANDARD 17)
  set_property(TARGET libavif-container-tests PROPERTY CXX_FLAGS_DEBUG "-g3 -O0 -fno-omit-frame-pointer")

  gtest_add_tests(
      TARGET      libavif-container-tests
      #TEST_SUFFIX .noArgs
      TEST_LIST   ALL_TESTS
  )
  set_tests_properties(${ALL_TESTS} PROPERTIES TIMEOUT 10)
  #add_test(NAME libavif-container-tests COMMAND libavif-container-tests)
endif()

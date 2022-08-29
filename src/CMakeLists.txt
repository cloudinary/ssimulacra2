# Copyright (c) the JPEG XL Project Authors. All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

# Ubuntu bionic ships with cmake 3.10.
cmake_minimum_required(VERSION 3.10)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Honor VISIBILITY_INLINES_HIDDEN on all types of targets.
if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()
# Pass CMAKE_EXE_LINKER_FLAGS to CC and CXX compilers when testing if they work.
if(POLICY CMP0065)
  cmake_policy(SET CMP0065 NEW)
endif()

# Set PIE flags for POSITION_INDEPENDENT_CODE targets, added in 3.14.
if(POLICY CMP0083)
  cmake_policy(SET CMP0083 NEW)
endif()

project(LIBJXL LANGUAGES C CXX)

include(CheckCXXSourceCompiles)
check_cxx_source_compiles(
   "int main() {
      #if !defined(__EMSCRIPTEN__)
      static_assert(false, \"__EMSCRIPTEN__ is not defined\");
      #endif
      return 0;
    }"
  JPEGXL_EMSCRIPTEN
)

message(STATUS "CMAKE_SYSTEM_PROCESSOR is ${CMAKE_SYSTEM_PROCESSOR}")
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fsanitize=fuzzer-no-link" CXX_FUZZERS_SUPPORTED)
check_cxx_compiler_flag("-Xclang -mconstructor-aliases" CXX_CONSTRUCTOR_ALIASES_SUPPORTED)
check_cxx_compiler_flag("-fmacro-prefix-map=OLD=NEW" CXX_MACRO_PREFIX_MAP)
check_cxx_compiler_flag("-fno-rtti" CXX_NO_RTTI_SUPPORTED)

# Enabled PIE binaries by default if supported.
include(CheckPIESupported OPTIONAL RESULT_VARIABLE CHECK_PIE_SUPPORTED)
if(CHECK_PIE_SUPPORTED)
  check_pie_supported(LANGUAGES CXX)
  if(CMAKE_CXX_LINK_PIE_SUPPORTED)
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
  endif()
endif()

find_package(PkgConfig)
if(NOT APPLE AND NOT WIN32 AND NOT HAIKU AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  pkg_check_modules(TCMallocMinimalVersionCheck QUIET IMPORTED_TARGET
      libtcmalloc_minimal)
  if(TCMallocMinimalVersionCheck_FOUND AND
     NOT TCMallocMinimalVersionCheck_VERSION VERSION_EQUAL 2.8.0)
    # Enabled by default except on Windows and Apple builds for
    # tcmalloc != 2.8.0. tcmalloc 2.8.1 already has a fix for this issue.
    set(ENABLE_TCMALLOC_DEFAULT true)
  else()
    message(STATUS
        "tcmalloc version ${TCMallocMinimalVersionCheck_VERSION} -- "
        "tcmalloc 2.8.0 disabled due to "
        "https://github.com/gperftools/gperftools/issues/1204")
  endif()
endif()

check_cxx_source_compiles(
   "int main() {
      #if !defined(HWY_DISABLED_TARGETS)
      static_assert(false, \"HWY_DISABLED_TARGETS is not defined\");
      #endif
      return 0;
    }"
  JXL_HWY_DISABLED_TARGETS_FORCED
)

set(WARNINGS_AS_ERRORS_DEFAULT false)

set(BUNDLE_LIBPNG_DEFAULT NO)


# Standard cmake naming for building shared libraries.
get_property(SHARED_LIBS_SUPPORTED GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static ones" false) #${SHARED_LIBS_SUPPORTED})

set(JPEGXL_ENABLE_FUZZERS false CACHE BOOL
    "Build JPEGXL fuzzer targets.")
set(JPEGXL_ENABLE_DEVTOOLS false CACHE BOOL
    "Build JPEGXL developer tools.")
set(JPEGXL_ENABLE_TOOLS true CACHE BOOL
    "Build JPEGXL user tools: cjxl and djxl.")
set(JPEGXL_ENABLE_DOXYGEN false CACHE BOOL
    "Generate C API documentation using Doxygen.")
set(JPEGXL_ENABLE_MANPAGES false CACHE BOOL
    "Build and install man pages for the command-line tools.")
set(JPEGXL_ENABLE_BENCHMARK false CACHE BOOL
    "Build JPEGXL benchmark tools.")
set(JPEGXL_ENABLE_EXAMPLES false CACHE BOOL
    "Build JPEGXL library usage examples.")
set(JPEGXL_BUNDLE_LIBPNG false CACHE BOOL
    "Build libpng from source and link it statically.")
set(JPEGXL_ENABLE_JNI false CACHE BOOL
    "Build JPEGXL JNI Java wrapper, if Java dependencies are installed.")
set(JPEGXL_ENABLE_SJPEG false CACHE BOOL
    "Build JPEGXL with support for encoding with sjpeg.")
set(JPEGXL_ENABLE_OPENEXR true CACHE BOOL
    "Build JPEGXL with support for OpenEXR if available.")
set(JPEGXL_ENABLE_SKCMS false CACHE BOOL
    "Build with skcms instead of lcms2.")
set(JPEGXL_BUNDLE_SKCMS false CACHE BOOL
    "When building with skcms, bundle it into libjxl.a.")
set(JPEGXL_ENABLE_VIEWERS false CACHE BOOL
    "Build JPEGXL viewer tools for evaluation.")
set(JPEGXL_ENABLE_TCMALLOC ${ENABLE_TCMALLOC_DEFAULT} CACHE BOOL
    "Build JPEGXL using gperftools (tcmalloc) allocator.")
set(JPEGXL_ENABLE_PLUGINS false CACHE BOOL
    "Build third-party plugins to support JPEG XL in other applications.")
set(JPEGXL_ENABLE_COVERAGE false CACHE BOOL
    "Enable code coverage tracking for libjxl. This also enables debug and disables optimizations.")
set(JPEGXL_ENABLE_PROFILER false CACHE BOOL
    "Builds in support for profiling (printed by tools if extra flags given)")
set(JPEGXL_ENABLE_SIZELESS_VECTORS false CACHE BOOL
    "Builds in support for SVE/RVV vectorization")
set(JPEGXL_ENABLE_TRANSCODE_JPEG true CACHE BOOL
    "Builds in support for decoding transcoded JXL files back to JPEG,\
 disabling it makes the decoder reject JXL_DEC_JPEG_RECONSTRUCTION events,\
 (default enabled)")
set(JPEGXL_STATIC false CACHE BOOL
    "Build tools as static binaries.")
set(JPEGXL_WARNINGS_AS_ERRORS ${WARNINGS_AS_ERRORS_DEFAULT} CACHE BOOL
    "Treat warnings as errors during compilation.")
set(JPEGXL_DEP_LICENSE_DIR "" CACHE STRING
    "Directory where to search for system dependencies \"copyright\" files.")
set(JPEGXL_FORCE_NEON false CACHE BOOL
    "Set flags to enable NEON in arm if not enabled by your toolchain.")


# Force system dependencies.
set(JPEGXL_FORCE_SYSTEM_BROTLI true CACHE BOOL
    "Force using system installed brotli instead of third_party/brotli source.")
set(JPEGXL_FORCE_SYSTEM_GTEST true CACHE BOOL
    "Force using system installed googletest (gtest/gmock) instead of third_party/googletest source.")
set(JPEGXL_FORCE_SYSTEM_LCMS2 true CACHE BOOL
    "Force using system installed lcms2 instead of third_party/lcms source.")
set(JPEGXL_FORCE_SYSTEM_HWY true CACHE BOOL
    "Force using system installed highway (libhwy-dev) instead of third_party/highway source.")

# Check minimum compiler versions. Older compilers are not supported and fail
# with hard to understand errors.
if (NOT CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID)
  message(FATAL_ERROR "Different C/C++ compilers set: "
          "${CMAKE_C_COMPILER_ID} vs ${CMAKE_CXX_COMPILER_ID}")
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Android NDK's toolchain.cmake fakes the clang version in
  # CMAKE_CXX_COMPILER_VERSION with an incorrect number, so ignore this.
  if (NOT CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION MATCHES "clang"
      AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
    message(FATAL_ERROR
      "Minimum Clang version required is Clang 5, please update.")
  endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
    message(FATAL_ERROR
      "Minimum GCC version required is 7, please update.")
  endif()
endif()



  find_package(HWY 0.15.0)
  if (NOT HWY_FOUND)
    message(FATAL_ERROR
        "Highway library (hwy) not found. Install libhwy-dev or download it "
        "to third_party/highway from https://github.com/google/highway . "
        "Highway is required to build JPEG XL. You can run "
        "${PROJECT_SOURCE_DIR}/deps.sh to download this dependency.")
  endif()
#  find_package(Brotli)
#  if (NOT Brotli_FOUND)
#    message(FATAL_ERROR
#        "Brotli not found, install brotli-dev or download brotli source code to"
#        " third_party/brotli from https://github.com/google/brotli. You can use"
#        " ${PROJECT_SOURCE_DIR}/deps.sh to download this dependency.")
#  endif ()
    find_package(LCMS2 2.13)
    if ( NOT LCMS2_FOUND )
      message(FATAL_ERROR "Please install lcms2 or run git submodule update --init")
    endif ()


# codec input, color management, image ops, etc
add_subdirectory(lib)

# The actual ssimulacra 2 implementation
add_executable(ssimulacra2 ssimulacra2_main.cc ssimulacra2.cc)
target_link_libraries(ssimulacra2 jxl jxl_extras-static hwy)

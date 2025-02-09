cmake_minimum_required(VERSION 3.10)
project(mp4als)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Default definitions and flags
add_definitions(-DWARN_BUFFERSIZEDB_OVER_24BIT -DPERMIT_SAMPLERATE_OVER_16BIT)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")

# Include directories
include_directories(
    src
    src/AlsImf
    src/AlsImf/Mp4
)

# Option to enable LPC_ADAPT
option(WARN_BUFFERSIZE "Warn buffer size over 24bit" ON)
option(PERMIT_SAMPLERATE "Permit samplerate over 16bit" ON)
option(ENABLE_LPC_ADAPT "Enable LPC adaptive support" OFF)

if (WARN_BUFFERSIZE)
    add_definitions(-DWARN_BUFFERSIZEDB_OVER_24BIT)
endif()

if (PERMIT_SAMPLERATE)
    add_definitions(-DPERMIT_SAMPLERATE_OVER_16BIT)
endif()

if (ENABLE_LPC_ADAPT)
    if ((CMAKE_SYSTEM_NAME STREQUAL "Windows" AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
        (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86" OR CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")) OR
        (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i686")) OR
        (CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND (CMAKE_SYSTEM_PROCESSOR MATCHES "i386|ppc|x86_64|ppc64")) OR
        (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" AND (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i386")))
        
        add_definitions(-DLPC_ADAPT)
        message(STATUS "LPC ADAPT is enabled for the current platform and processor.")
        
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "Building in Debug mode")
            set(LPC_ADAPT_SUFFIX "debug")
        else()
            message(STATUS "Building in Release mode")
            set(LPC_ADAPT_SUFFIX "release")
        endif()

        # Link binary object file of LPC adaptive for supported operating systems.
        if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
            if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86")
                target_link_libraries(mp4als PRIVATE "${CMAKE_SOURCE_DIR}/lib/win/lpc_adapt_vc9_win32_${LPC_ADAPT_SUFFIX}.obj")
            elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "AMD64")
                target_link_libraries(mp4als PRIVATE "${CMAKE_SOURCE_DIR}/lib/win/lpc_adapt_vc9_x64_${LPC_ADAPT_SUFFIX}.obj")
            endif()
        elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
            if (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
                target_link_libraries(mp4als PRIVATE "${CMAKE_SOURCE_DIR}/lib/linux/lpc_adapt_x86_64_${LPC_ADAPT_SUFFIX}.o")
            elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
                target_link_libraries(mp4als PRIVATE "${CMAKE_SOURCE_DIR}/lib/linux/lpc_adapt_i386_${LPC_ADAPT_SUFFIX}.o")
            endif()
        elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            target_link_libraries(mp4als PRIVATE "${CMAKE_SOURCE_DIR}/lib/mac/lpc_adapt_${LPC_ADAPT_SUFFIX}.o")
        endif()
        message(WARNING "LPC adaption is not supported on your OS, compiler or architecture type.")
    endif()
endif()

# Source files
set(SOURCES
    src/als2mp4.cpp
    src/audiorw.cpp
    src/cmdline.cpp
    src/crc.cpp
    src/decoder.cpp
    src/profiles.cpp
    src/ec.cpp
    src/encoder.cpp
    src/floating.cpp
    src/lms.cpp
    src/lpc.cpp
    src/mcc.cpp
    src/mlz.cpp
    src/mp4als.cpp
    src/rn_bitio.cpp
    src/stream.cpp
    src/wave.cpp
    src/AlsImf/ImfBox.cpp
    src/AlsImf/ImfDescriptor.cpp
    src/AlsImf/ImfFileStream.cpp
    src/AlsImf/ImfPrintStream.cpp
    src/AlsImf/ImfSampleEntry.cpp
    src/AlsImf/Mp4/Mp4aFile.cpp
    src/AlsImf/Mp4/Mp4Box.cpp
)

set(HEADERS
    src/als2mp4.h
    src/audiorw.h
    src/bitio.h
    src/cmdline.h
    src/crc.h
    src/decoder.h
    src/profiles.h
    src/ec.h
    src/encoder.h
    src/floating.h
    src/lms.h
    src/lpc.h
    src/lpc_adapt.h
    src/mcc.h
    src/mlz.h
    src/rn_bitio.h
    src/stream.h
    src/wave.h
    src/AlsImf/ImfBox.h
    src/AlsImf/ImfDescriptor.h
    src/AlsImf/ImfFileStream.h
    src/AlsImf/ImfPrintStream.h
    src/AlsImf/ImfSampleEntry.h
    src/AlsImf/Mp4/Mp4aFile.h
    src/AlsImf/Mp4/Mp4Box.h
)

# Add executable
add_executable(mp4als ${SOURCES} ${HEADERS})

# Output directory
set_target_properties(mp4als PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
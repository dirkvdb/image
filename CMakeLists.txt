cmake_minimum_required(VERSION 2.8)
enable_testing()

option(STANDALONE "Not used as a submodule of another project" OFF)
option(ENABLE_TESTS "build unit tests" ON)

if (STANDALONE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../utils "${CMAKE_CURRENT_BINARY_DIR}/utils")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif ()
endif ()

find_package(JPEG)
if (JPEG_FOUND)
    option(HAVE_JPEG "Jpeg support" ON)
endif ()

find_package(PNG)
if (PNG_FOUND)
    option(HAVE_PNG "Png support" ON)
endif ()

set(IMAGE_SYS_INCLUDE_DIRS)

set(IMAGE_SRC_LIST
    inc/image/image.h
    inc/image/imagefactory.h src/imagefactory.cpp
    inc/image/imageloadstoreinterface.h
)

if (HAVE_JPEG)
    list(APPEND IMAGE_SRC_LIST src/imageloadstorejpeg.h src/imageloadstorejpeg.cpp)
    list(APPEND IMAGE_SYS_INCLUDE_DIRS ${JPEG_INCLUDE_DIR})
    list(APPEND IMAGE_LIBRARY_DIRS ${JPEG_LIBRARY_DIRS})
    list(APPEND IMAGE_LIBRARIES ${JPEG_LIBRARIES})
endif ()

if (HAVE_PNG)
    list(APPEND IMAGE_SRC_LIST src/imageloadstorepng.h src/imageloadstorepng.cpp)
    list(APPEND IMAGE_SYS_INCLUDE_DIRS ${PNG_INCLUDE_DIR})
    list(APPEND IMAGE_LIBRARY_DIRS ${PNG_LIBRARY_DIRS})
    list(APPEND IMAGE_LIBRARIES ${PNG_LIBRARIES})
endif ()

add_library(image STATIC ${IMAGE_SRC_LIST})

target_include_directories(image PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:include>
    PRIVATE ${CMAKE_BINARY_DIR}
)

target_include_directories(image SYSTEM PRIVATE
    ${IMAGE_SYS_INCLUDE_DIRS}
)

target_link_libraries(image utils ${IMAGE_LIBRARIES})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/imageconfig.h.in ${CMAKE_BINARY_DIR}/imageconfig.h)

if (ENABLE_TESTS)
    add_subdirectory(test)
endif ()

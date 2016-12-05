include_directories(SYSTEM
    ${CMAKE_CURRENT_SOURCE_DIR}/gmock
)

include_directories(${CMAKE_BINARY_DIR})

add_executable(imagetest
    gmock-gtest-all.cpp
    main.cpp
    imageloadingtest.cpp
)

target_link_libraries(imagetest
    image
)

set(IMAGE_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/imagetestconfig.h.in ${CMAKE_BINARY_DIR}/imagetestconfig.h)

add_test(NAME ImageTests COMMAND imagetest)

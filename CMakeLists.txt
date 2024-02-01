cmake_minimum_required(VERSION 3.24)
project(IEML_CBOR_cpp)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

option(IEML_CBOR_ENABLE_INSTALL "Enable install rule" ON)

FetchContent_Declare(
        IEML_cpp
        GIT_REPOSITORY https://github.com/Hedgehogo/IEML-cpp.git
        GIT_TAG        main
        FIND_PACKAGE_ARGS NAMES IEML_cpp
)

FetchContent_Declare(
        cbor_cpp
        GIT_REPOSITORY https://github.com/Hedgehogo/cbor-cpp.git
        GIT_TAG        master
        FIND_PACKAGE_ARGS NAMES cbor_cpp
)

FetchContent_MakeAvailable(IEML_cpp cbor_cpp abseil)


file(GLOB_RECURSE src "lib/*.hpp" "lib/*.cpp")

add_library(${PROJECT_NAME} ${src})

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PUBLIC IEML_cpp cbor_cpp::cbor_cpp)

target_include_directories(${PROJECT_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<BUILD_INTERFACE:${ctre_SOURCE_DIR}/include>
        )


file(GLOB_RECURSE test_src "tests/*.hpp" "tests/*.cpp")

add_executable(${PROJECT_NAME}_tests ${test_src})

target_link_libraries(${PROJECT_NAME}_tests PRIVATE gtest gtest_main ${PROJECT_NAME})

if (IEML_CBOR_ENABLE_INSTALL)
        install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/lib/IEML_CBOR DESTINATION ${CMAKE_INSTALL_PREFIX}/include PATTERN "*.hpp")

        install(TARGETS ${PROJECT_NAME}
                EXPORT ${PROJECT_NAME}
                LIBRARY DESTINATION lib
                PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
                )

        install(EXPORT ${PROJECT_NAME}
                FILE ${PROJECT_NAME}Config.cmake
                NAMESPACE ${PROJECT_NAME}::
                DESTINATION lib/cmake/${PROJECT_NAME}
                )
endif ()
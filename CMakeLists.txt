cmake_minimum_required(VERSION 3.15)

include_guard(GLOBAL)

project(price_server
        VERSION 1.0.0
        DESCRIPTION ""
        HOMEPAGE_URL ""
        LANGUAGES C CXX
)

if (NOT CMAKE_CXX_STANDARD)
    message(STATUS "[${PROJECT_NAME}] setting c++ standard to c++20")
    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

set(PROJECT_NAMESPACE ${PROJECT_NAME}::)

set(COMPANY "Mels")
set(COPYRIGHT "Copyright (c) 2024 ${COMPANY}. All rights reserved.")
set(IDENTIFIER "${PROJECT_NAME}")

find_package(spdlog REQUIRED)
find_package(gRPC REQUIRED)
find_package(asio REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/c++/main.cc> $<INSTALL_INTERFACE:src/c++/main.cc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/c++/tools/logger/logger_storage.h> $<INSTALL_INTERFACE:src/c++/tools/logger/logger_storage.h>

        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/c++/session.h> $<INSTALL_INTERFACE:src/c++/session.h> src/c++/session.cc
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/c++/server.h> $<INSTALL_INTERFACE:src/c++/server.h> src/c++/server.cc
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/c++/session_manager.h> $<INSTALL_INTERFACE:src/c++/session_manager.h> src/c++/session_manager.cc
)

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/c++
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        spdlog::spdlog
        grpc::grpc
        asio::asio
)

target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        -DPROJECT_NAME="${PROJECT_NAME}"
        -DPROJECT_VERSION="${PROJECT_VERSION}"
        -DPROJECT_COMPANY="${COMPANY}"
        -DPROJECT_AUTHOR="Mels ilia2577@gmail.com"
)

message(STATUS "[${PROJECT_NAME}] installing ${PROJECT_NAME} in namespace ${PROJECT_NAMESPACE}")
include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
message(STATUS "[${PROJECT_NAME}] installing in prefix: ${CMAKE_INSTALL_PREFIX}")

set(PROJECT_CONFIG_FILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in")
set(PROJECT_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake")
set(CONFIG_TARGETS_FILE "${PROJECT_NAME}-config-targets.cmake")
set(CONFIG_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake")
set(INSTALL_EXPORT_DESTINATION_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

export(TARGETS ${PROJECT_NAME} NAMESPACE ${PROJECT_NAMESPACE} FILE "${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_TARGETS_FILE}")
install(EXPORT ${PROJECT_NAME}
        DESTINATION ${INSTALL_EXPORT_DESTINATION_DIR}
        NAMESPACE ${PROJECT_NAMESPACE}
        FILE ${CONFIG_TARGETS_FILE}
)

message(STATUS "[${PROJECT_NAME}] configuring ${PROJECT_NAME} done!")


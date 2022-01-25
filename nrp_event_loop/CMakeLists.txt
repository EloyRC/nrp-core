set(PROJECT_NAME "NRPEventLoop")
set(HEADER_DIRECTORY "nrp_event_loop")

set(NAMESPACE_NAME "${PROJECT_NAME}")

set(LIBRARY_NAME "${PROJECT_NAME}")
set(PYTHON_MODULE_NAME "event_loop")
set(EXECUTABLE_NAME "${PROJECT_NAME}Exec")
set(TEST_NAME "${PROJECT_NAME}Tests")

set(LIB_EXPORT_NAME "${LIBRARY_NAME}Targets")
set(LIB_CONFIG_NAME "${LIBRARY_NAME}Config")
set(LIB_VERSION_NAME "${LIB_CONFIG_NAME}Version")

cmake_minimum_required(VERSION 3.16)
project("${PROJECT_NAME}" VERSION ${NRP_VERSION})

include(GNUInstallDirs)
include(ExternalProject)

# List library build files
set(LIB_SRC_FILES
        nrp_event_loop/computational_graph/computational_node.cpp
        nrp_event_loop/computational_graph/port.cpp
        nrp_event_loop/computational_graph/computational_graph.cpp

        nrp_event_loop/computational_graph/input_port.cpp
        nrp_event_loop/computational_graph/output_port.cpp

        nrp_event_loop/computational_graph/functional_node.cpp
        nrp_event_loop/computational_graph/input_node.cpp
        nrp_event_loop/computational_graph/output_node.cpp

        nrp_event_loop/computational_graph/computational_graph_manager.cpp
        nrp_event_loop/computational_graph/functional_node_factory.cpp

        nrp_event_loop/event_loop/event_loop.cpp

        nrp_event_loop/python/functional_node.cpp
        nrp_event_loop/python/input_edge.cpp
        nrp_event_loop/python/output_edge.cpp

        nrp_event_loop/nodes/dummy/input_dummy.cpp
        nrp_event_loop/nodes/dummy/output_dummy.cpp
        nrp_event_loop/nodes/engine/input_node.cpp
        nrp_event_loop/nodes/engine/output_node.cpp

        nrp_event_loop/utils/graph_utils.cpp
        nrp_event_loop/utils/data_conversion.cpp
)

set(ROS_SRC_FILES
        nrp_event_loop/nodes/ros/input_node.cpp
        nrp_event_loop/nodes/ros/output_node.cpp
        )

# List of python module build files
set(PYTHON_MODULE_SRC_FILES
        python_module/python_module.cpp
)

# List executable build files
set(EXEC_SRC_FILES
)

# List testing build files
set(TEST_SRC_FILES
        tests/test_files/helper_classes.cpp
        tests/test_ports.cpp
        tests/test_nodes.cpp
        tests/test_computational_graph.cpp
        tests/test_python_nodes.cpp
        tests/test_event_loop.cpp
)

set(ROS_TEST_SRC_FILES
        tests/test_python_ros_nodes.cpp
        )


##########################################
## Dependencies

if(ENABLE_ROS STREQUAL FULL)
    find_package(catkin REQUIRED COMPONENTS roscpp nrp_ros_msgs ${NRP_CATKIN_PACKAGES})
endif()

find_package(Boost COMPONENTS
        system
        REQUIRED
        )

##########################################
## Header configuration

configure_file("nrp_event_loop/config/cmake_constants.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/${HEADER_DIRECTORY}/config/cmake_constants.h" @ONLY)

# Configures ros_edge_factory.h
if(ENABLE_ROS STREQUAL FULL)
    file(READ "${CMAKE_BINARY_DIR}/nrp_ros_msg_headers.txt" NRP_ROS_MSG_HEADERS)
    set(ROS_MSG_TYPES_INCLUDE "")
    foreach(MSG ${NRP_ROS_MSG_HEADERS})
        string(APPEND ROS_MSG_TYPES_INCLUDE "#include \"${MSG}\"\n")
    endforeach()

    file(READ "${CMAKE_BINARY_DIR}/nrp_ros_msg_types.txt" NRP_ROS_MSG_TYPES)
    set(ROS_MSG_TYPES_CHECK "")
    foreach(MSG ${NRP_ROS_MSG_TYPES})
        string(APPEND ROS_MSG_TYPES_CHECK "if (rosType == \"${MSG}\")\n        {\n            if(_isInput)\n                return pySetupInput<${MSG}>(obj);\n            else\n                return pySetupOutput<${MSG}>(obj);\n        }\n        else ")
    endforeach()

    configure_file("nrp_event_loop/nodes/ros/ros_edge_factory.h.in" "${CMAKE_CURRENT_BINARY_DIR}/include/${HEADER_DIRECTORY}/nodes/ros/ros_edge_factory.h" @ONLY)
endif()

# Python Module defines
configure_file("python_module/__init__.py.in" "${CMAKE_CURRENT_BINARY_DIR}/src/__init__.py" @ONLY)

##########################################
# Library
add_library("${LIBRARY_NAME}" SHARED ${LIB_SRC_FILES})
add_library(${NAMESPACE_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME})
target_compile_options(${LIBRARY_NAME} PUBLIC $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra>)
target_compile_options(${LIBRARY_NAME} PUBLIC $<$<CXX_COMPILER_ID:GNU>:-fconcepts>)

set(NRP_ELE_LIB_TARGET "${NAMESPACE_NAME}::${LIBRARY_NAME}" CACHE INTERNAL "NRP Event Loop Engine Library target")

set_target_properties(${LIBRARY_NAME} PROPERTIES PREFIX "")

target_link_libraries(${LIBRARY_NAME}
    PUBLIC
        ${NRP_GEN_LIB_TARGET}
        ${Boost_LIBRARIES}
        ${catkin_LIBRARIES}

    PRIVATE
)

target_include_directories(${LIBRARY_NAME} BEFORE
    PUBLIC 
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"

        ${Boost_INCLUDE_DIRS}
        ${catkin_INCLUDE_DIRS}

    PRIVATE
)

# ROS nodes target
if(ENABLE_ROS STREQUAL FULL)
    set(ROS_LIBRARY_NAME "${LIBRARY_NAME}ROSNodes")
    add_library(${ROS_LIBRARY_NAME} SHARED ${ROS_SRC_FILES})
    add_library(${NAMESPACE_NAME}::${ROS_LIBRARY_NAME} ALIAS ${ROS_LIBRARY_NAME})
    target_compile_options(${ROS_LIBRARY_NAME} PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra>)

    target_include_directories(${ROS_LIBRARY_NAME} BEFORE
            PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"

            ${catkin_INCLUDE_DIRS}
            ${Boost_INCLUDE_DIRS}
            )

    target_link_libraries(${ROS_LIBRARY_NAME}
            PUBLIC
            ${NAMESPACE_NAME}::${LIBRARY_NAME}
            ${NRP_GEN_LIB_TARGET}
            ${NRP_ROS_PROXY_TARGET}
            ${catkin_LIBRARIES}
            ${Boost_LIBRARIES}
            )
endif()

##########################################
## Python Module
if(NOT ${PYTHON_MODULE_SRC_FILES} STREQUAL "")
    add_library(${PYTHON_MODULE_NAME} SHARED ${PYTHON_MODULE_SRC_FILES})
    add_library(${NAMESPACE_NAME}::${PYTHON_MODULE_NAME} ALIAS ${PYTHON_MODULE_NAME})
    target_compile_options(${PYTHON_MODULE_NAME} PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>:-Wall -Wextra>)
    set_target_properties(${PYTHON_MODULE_NAME} PROPERTIES PREFIX "")

    target_include_directories(${PYTHON_MODULE_NAME}
        PUBLIC
    )

    target_link_libraries(${PYTHON_MODULE_NAME}
        PUBLIC
            ${NAMESPACE_NAME}::${LIBRARY_NAME}
    )

    if(ENABLE_ROS STREQUAL FULL)
        target_link_libraries(${PYTHON_MODULE_NAME}
                PUBLIC
                ${NAMESPACE_NAME}::${ROS_LIBRARY_NAME}
                )
    endif()
endif()


##########################################
## NRPNestExecutable
if(NOT "${EXEC_SRC_FILES}" STREQUAL "")
    add_executable(${EXECUTABLE_NAME} ${EXEC_SRC_FILES})
    target_link_libraries(${EXECUTABLE_NAME} ${LIBRARY_NAME})
endif()


##########################################
## Append Doxygen files
add_doxygen_source("${CMAKE_CURRENT_SOURCE_DIR}" ${EXEC_SRC_FILES} ${LIB_SRC_FILES} ${PYTHON_MODULE_SRC_FILES} ${HEADER_DIRECTORY})


##########################################
## Tests
if(${ENABLE_TESTING} AND NOT "${TEST_SRC_FILES}" STREQUAL "")

    # Create testing executable
    enable_testing()
    add_executable(${TEST_NAME} ${TEST_SRC_FILES})
    target_link_libraries(${TEST_NAME}
        PUBLIC
        ${NAMESPACE_NAME}::${LIBRARY_NAME}
        GTest::GTest
        GTest::Main)

    gtest_discover_tests(${TEST_NAME}
        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
        EXTRA_ARGS -VV)

    if(ENABLE_ROS STREQUAL FULL AND NOT "${ROS_TEST_SRC_FILES}" STREQUAL "")
        enable_testing()
        set(ROS_TEST_NAME "${TEST_NAME}ROS")
        add_executable(${ROS_TEST_NAME} ${ROS_TEST_SRC_FILES})
        target_link_libraries(${ROS_TEST_NAME}
                PUBLIC
                ${NAMESPACE_NAME}::${LIBRARY_NAME}
                ${NAMESPACE_NAME}::${ROS_LIBRARY_NAME}
                GTest::GTest
                GTest::Main)

        gtest_discover_tests(${ROS_TEST_NAME}
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
                EXTRA_ARGS -VV)
    endif()

endif()


##########################################
## Installation

set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

# Install library files
install(TARGETS
        ${LIBRARY_NAME}
    EXPORT
        ${LIB_EXPORT_NAME}
    LIBRARY DESTINATION ${NRP_PLUGIN_INSTALL_DIR}
    ARCHIVE DESTINATION ${NRP_PLUGIN_INSTALL_DIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}

    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${HEADER_DIRECTORY}
)

# Install export target
install(EXPORT ${LIB_EXPORT_NAME}
    DESTINATION
        ${INSTALL_CONFIGDIR}
    FILE
        "${LIB_EXPORT_NAME}.cmake"
    NAMESPACE
        "${NAMESPACE_NAME}::"
)

# Install headers
install(DIRECTORY "${HEADER_DIRECTORY}" "${CMAKE_CURRENT_BINARY_DIR}/include/${HEADER_DIRECTORY}"
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
)

# Install python module
if(TARGET ${PYTHON_MODULE_NAME})
    install(TARGETS ${PYTHON_MODULE_NAME}
        DESTINATION "${PYTHON_INSTALL_DIR_REL}/${NRP_PYTHON_MODULE_NAME}/${PYTHON_MODULE_NAME}")

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/__init__.py"
        DESTINATION "${PYTHON_INSTALL_DIR_REL}/${NRP_PYTHON_MODULE_NAME}/${PYTHON_MODULE_NAME}")
endif()

# Install executable files
if(TARGET ${EXECUTABLE_NAME})
    install(TARGETS ${EXECUTABLE_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# create cmake version and config files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${LIB_VERSION_NAME}.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/cmake/ProjectConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${LIB_CONFIG_NAME}.cmake"
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

# Install cmake version and config files
install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${LIB_CONFIG_NAME}.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${LIB_VERSION_NAME}.cmake"
    DESTINATION ${INSTALL_CONFIGDIR}
)
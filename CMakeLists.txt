################################################################################
#                                                                              #
# Copyright (C) 2019 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# @author Luca Tricerri <luca.tricerri@iit.it>  

cmake_minimum_required(VERSION 3.5)
#Module creation
set(PROJECT_NAME blocktestcustomplugins)
project(${PROJECT_NAME})

project(blocktestcustomplugins
        LANGUAGES C CXX
        VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

#Thrift for clock
set(YARP_MINIMUM_REQUIRED_VERSION 3.0.102)
find_package(YARP REQUIRED COMPONENTS OS conf dev sig math rosmsg name idl_tools)
find_package(blocktestcore 2.0.0 REQUIRED)
if(${YARP_VERSION} VERSION_LESS ${YARP_MINIMUM_REQUIRED_VERSION})
    message(FATAL_ERROR "YARP version ${YARP_VERSION} not sufficient, at least version ${YARP_MINIMUM_REQUIRED_VERSION} is required.")
endif()

file(GLOB allSrc "src/*.h" "src/*.cpp")

add_library(${PROJECT_NAME} MODULE ${allSrc})
target_include_directories(${PROJECT_NAME} PRIVATE ${YARP_HELPERS_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/src ${blocktestcore_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${blocktestcore_LIBRARIES} YARP::YARP_OS YARP::YARP_init YARP::YARP_dev YARP::YARP_conf YARP::YARP_sig YARP::YARP_math YARP::YARP_rosmsg)

#xml template folder

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_LIST_DIR}/xmltemplate
    ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/$<CONFIG>/xmltemplate)
    else()
    file(COPY ${CMAKE_CURRENT_LIST_DIR}/xmltemplate
            DESTINATION ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
endif()

install(DIRECTORY xmltemplate DESTINATION ${CMAKE_INSTALL_BINDIR})

# Add install target
set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME})

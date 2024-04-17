cmake_minimum_required(VERSION 3.20)
project(CepGenPython)

set(CEPGEN_PATH ${CEPGEN_PATH} $ENV{CEPGEN_PATH})
set(EPIC_DIRS ${EPIC_DIRS} $ENV{EPIC_DIR})
set(PARTONS_DIRS ${PARTONS_DIRS} $ENV{PARTONS_DIR})

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/cmake ${CEPGEN_PATH}/cmake ${PARTONS_DIR}/cmake/Modules ${PARTONS_DIR}/partons/cmake/Modules)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(CepGen REQUIRED)
find_path(EPIC_SOURCES_DIR NAMES Epic.cpp HINTS ${EPIC_DIRS} PATH_SUFFIXES src)
find_path(EPIC_DATA_DIR NAMES version.txt HINTS ${EPIC_DIRS} PATH_SUFFIXES data)
find_path(EPIC_INCLUDE NAMES Epic.h HINTS ${EPIC_DIRS} PATH_SUFFIXES include)
if(NOT EPIC_SOURCES_DIR OR NOT EPIC_INCLUDE)
  return()
endif()

find_path(PARTONS_DIR NAMES include/partons/Partons.h HINTS ${PARTONS_DIRS} PATH_SUFFIXES partons)
find_package(PARTONS REQUIRED)
if(NOT PARTONS_FOUND)
  message(STATUS "EpIC was found, but not PARTONS.")
  return()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 COMPONENTS Core Sql XmlPatterns)
find_package(Qt5Widgets)
message("-- Found Qt5: " ${Qt5Widgets_VERSION})
set(QT_LIBRARY_CORE "Qt5::Core")
set(QT_LIBRARY_SQL "Qt5::Sql")
set(QT_LIBRARY_XML "Qt5::XmlPatterns")

find_package(GSL REQUIRED)
find_package(SFML COMPONENTS system REQUIRED)
find_package(ElementaryUtils REQUIRED)
find_package(NumA++ REQUIRED)
find_package(Apfel++ REQUIRED)
find_package(ROOT COMPONENTS Core Hist Minuit Foam REQUIRED)
find_package(HepMC3 REQUIRED)

file(GLOB cepgen_epic_sources src/*.cpp)
file(GLOB_RECURSE epic_sources ${EPIC_SOURCES_DIR}/*.cpp)

#----- copy the input cards
file(GLOB_RECURSE input_cards RELATIVE ${PROJECT_SOURCE_DIR} cards/*)
foreach(_files ${input_cards})
  configure_file(${_files} ${_files} COPYONLY)
endforeach()

#----- copy the configuration files
configure_file(data/partons.properties.in ${CMAKE_CURRENT_BINARY_DIR}/data/partons.properties @ONLY)
configure_file(data/logger.properties.in ${CMAKE_CURRENT_BINARY_DIR}/data/logger.properties @ONLY)
file(GLOB_RECURSE epic_data_files RELATIVE ${EPIC_DATA_DIR} ${EPIC_DATA_DIR}/*)
foreach(_f ${epic_data_files})
  configure_file(${EPIC_DATA_DIR}/${_f} ${CMAKE_CURRENT_BINARY_DIR}/data/${_f} COPYONLY)
endforeach()

#----- build the object
add_library(CepGenEpIC SHARED ${cepgen_epic_sources} ${epic_sources})
target_link_libraries(CepGenEpIC PRIVATE
    ${CepGen_LIBRARIES}
    ${GSL_LIBRARIES} ${SFML_LIBRARIES} ${ElementaryUtils_LIBRARIES} ${NumA++_LIBRARIES} ${PARTONS_LIBRARIES} ${Apfel++_LIBRARIES} ${ROOT_LIBRARIES} ${HEPMC3_LIBRARIES}
    ${QT_LIBRARIES} ${QT_LIBRARY_CORE} ${QT_LIBRARY_SQL} ${QT_LIBRARY_XML})
target_include_directories(CepGenEpIC PRIVATE
    ${CepGen_INCLUDE_DIRS}
    ${EPIC_INCLUDE} ${GSL_INCLUDE_DIRS} ${SFML_INCLUDE_DIR} ${ElementaryUtils_INCLUDE_DIR} ${NumA++_INCLUDE_DIR} ${PARTONS_INCLUDE_DIR} ${Apfel++_INCLUDE_DIR} ${ROOT_INCLUDE_DIRS} ${HEPMC3_INCLUDE_DIR}
    ${QT_INCLUDE_DIRS})
target_compile_options(CepGenEpIC PRIVATE "-Wno-deprecated-copy")

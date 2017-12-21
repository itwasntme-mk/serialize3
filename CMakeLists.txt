cmake_minimum_required (VERSION 2.6)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
enable_language(CXX)
enable_language(C)
SET(CMAKE_CXX_COMPILER_LOADED 1)
SET(CMAKE_C_COMPILER_LOADED 1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

SET(BOOST_COMPONENTS)
LIST(APPEND BOOST_COMPONENTS system
                             filesystem
                             program_options)
FIND_PACKAGE(Boost 1.58 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

include_directories(SYSTEM "./")
                             
project (serialize3)
add_executable( serialize3
                main.cpp
                application.cpp
                apps_factory.cpp
                castxml_app.cpp
                castxml-gccxml_app.cpp
                gccxml_app.cpp
                codegenerator.cpp
                external_app_launcher.cpp
                serializablemap.cpp
                xml_element.cpp
                xml_reader_boost_property_tree.cpp )
target_link_libraries(serialize3 ${Boost_LIBRARIES})

message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
message(status "** Boost Libraries: ${Boost_LIBRARY_DIRS}")
message(status "** Boost Libraries: ${Boost_LIBRARIES}")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14 -g -DSERIALIZABLE_BOOST_CONTAINERS")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
endif()

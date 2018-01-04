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
                             program_options
                             iostreams)
FIND_PACKAGE(Boost 1.58 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

project (serialize3)

SET(FILE_LIST)
LIST(APPEND FILE_LIST
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
     xml_reader_boost_property_tree.cpp
     file_comparator.cpp)

SET(HEADER_LIST)
LIST(APPEND HEADER_LIST
     application.h
     apps_factory.h
     codegenerator.h
     constants.h
     external_app_launcher.h
     file_comparator.h
     logger.h
     serializablemap.h
     str_less.h
     xml_element.h
     xml_reader_boost_property_tree.h

     h/client_code/serialize_macros.h
     h/client_code/serialize_ptrwrapper.h
     h/client_code/serialize_utils.h

     h/gen_code/dumpertemplates.h
     h/gen_code/loadertemplates.h
     h/gen_code/serializable_std_type_includes.h
     h/gen_code/serializable_boost_cntrs_includes.h

     h/storage/primitivedumper.h
     h/storage/primitiveloader.h
     h/storage/serializedumper.h
     h/storage/serializeloader.h)
     
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  LIST(APPEND FILE_LIST test1_injected.cpp test2_injected.cpp test3_injected.cpp)
endif()

add_executable( serialize3 ${FILE_LIST} ${HEADER_LIST})
target_link_libraries( serialize3 ${Boost_LIBRARIES} )
#include_directories(../)
target_include_directories( serialize3 PUBLIC
                            "${CMAKE_CURRENT_SOURCE_DIR}/../"
                            "${Boost_INCLUDE_DIR}" )

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++14 -g -DSERIALIZABLE_BOOST_CONTAINERS")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
   #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
endif()

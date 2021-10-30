cmake_minimum_required(VERSION 3.16)

set(DEPDIR "${CMAKE_SOURCE_DIR}/Dependencies")

set(PROJECTNAME KHRay)
set(PROJECTDIR "${CMAKE_CURRENT_SOURCE_DIR}/Engine")
set(Main "main.cpp")

file(GLOB_RECURSE inc ${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.h)
file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Main})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${inc})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${src})

add_executable(
	${PROJECTNAME}
	${Main}
	${inc}
	${src})

if (MSVC)
	target_compile_options(${PROJECTNAME} PRIVATE "/W3") # warning level 3
	target_compile_options(${PROJECTNAME} PRIVATE "/MP") # Multi-processor compilation
endif()

set_property(TARGET ${PROJECTNAME} PROPERTY CXX_STANDARD 23)
target_precompile_headers(${PROJECTNAME} PRIVATE pch.h)

target_include_directories(${PROJECTNAME} PUBLIC ${PROJECTDIR})

target_include_directories(${PROJECTNAME} PUBLIC "${DEPDIR}")
target_include_directories(${PROJECTNAME} PUBLIC "${DEPDIR}/assimp/include")
target_link_libraries(${PROJECTNAME} ${DEPDIR}/assimp/lib/Release/assimp-vc142-mt.lib)
target_include_directories(${PROJECTNAME} PUBLIC "${DEPDIR}/embree/include")
target_link_libraries(${PROJECTNAME} ${DEPDIR}/embree/lib/embree3.lib)
target_link_libraries(${PROJECTNAME} ${DEPDIR}/embree/lib/tbb.lib)

# Linking

# DLL
add_custom_command(
	TARGET ${PROJECTNAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy 
		${DEPDIR}/assimp/bin/release/assimp-vc142-mt.dll

		${DEPDIR}/embree/bin/embree3.dll
		${DEPDIR}/embree/bin/tbb.dll

		$<TARGET_FILE_DIR:${PROJECTNAME}>
	DEPENDS ${PROJECTNAME})

# Copy assets
add_custom_command(
	TARGET ${PROJECTNAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/Assets

		$<TARGET_FILE_DIR:${PROJECTNAME}>/Assets
	DEPENDS ${PROJECTNAME})

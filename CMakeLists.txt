cmake_minimum_required(VERSION 3.21)

project(bomberman LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include(cmake/dependencies.cmake)

add_executable(bomberman
	src/main.cpp
	src/AssetPath.cpp
	src/Game.cpp
	src/Scene.cpp
	src/TileMap.cpp
	src/Player.cpp
	src/Enemy.cpp
	src/EnemyParachute.cpp
	src/Sprite.cpp
	src/Texture.cpp
	src/Shader.cpp
	src/ShaderProgram.cpp
	src/AnimKeyframes.h
	src/AssetPath.h
	src/Bomb.h
	src/Enemy.h
	src/EnemyChaser.h
	src/EnemyParachute.h
	src/Game.h
	src/Player.h
	src/Scene.h
	src/Shader.h
	src/ShaderProgram.h
	src/Sprite.h
	src/Texture.h
	src/TileMap.h
)

target_include_directories(bomberman PRIVATE src third_party)

target_link_libraries(bomberman PRIVATE
	glfw
	glm::glm
	libglew_static
	OpenGL::GL
)

if(MSVC)
	target_compile_options(bomberman PRIVATE /W3)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		PROPERTY VS_STARTUP_PROJECT bomberman)
else()
	target_compile_options(bomberman PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

add_custom_command(TARGET bomberman POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_CURRENT_SOURCE_DIR}/assets"
		"$<TARGET_FILE_DIR:bomberman>/assets"
	COMMENT "Copying assets next to the executable"
	VERBATIM
)

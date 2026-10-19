include(FetchContent)

find_package(OpenGL REQUIRED)

set(FETCHCONTENT_QUIET OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
	GIT_SHALLOW TRUE
)

FetchContent_Declare(glm
	GIT_REPOSITORY https://github.com/g-truc/glm.git
	GIT_TAG 1.0.1
	GIT_SHALLOW TRUE
)

set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ONLY_LIBS ON CACHE BOOL "" FORCE)
FetchContent_Declare(glew
	GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
	GIT_TAG glew-cmake-2.2.0
	GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw glm glew)

cmake_minimum_required(VERSION 3.10)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLM REQUIRED glm)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${GLM_INCLUDE_DIRS})

# Add GLAD source files
set(GLAD_SOURCES
    src/glad.c
)

# Add main source files
set(SOURCES
    simple_sphere.cpp
    ${GLAD_SOURCES}
)

# Create executable
add_executable(SolarSystem ${SOURCES})

# Link libraries
target_link_libraries(SolarSystem 
    OpenGL::GL
    glfw
    ${GLM_LIBRARIES}
)

# Compiler flags
if(APPLE)
    target_link_libraries(SolarSystem "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# Set compiler flags
target_compile_options(SolarSystem PRIVATE ${GLM_CFLAGS_OTHER})

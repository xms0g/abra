cmake_minimum_required(VERSION 3.23.0)
project(abra VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)

if (APPLE)
    add_compile_options(-gdwarf-4)
endif ()

find_package(OpenGL REQUIRED)

find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIR})
endif()

find_package(ASSIMP REQUIRED)
if(ASSIMP_FOUND)
    include_directories(${ASSIMP_INCLUDE_DIR})
endif()

set(LIBS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(GLAD
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/glad.c
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/glad/glad.h)
set(STB_IMAGE
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/image/stb_image.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/image/stb_image.h)

file(GLOB_RECURSE SOURCES "src/*")
file(GLOB_RECURSE IMGUI "libs/imgui/*")

set(COMPILATION_UNIT src/main.cpp ${SOURCES} ${GLAD} ${STB_IMAGE} ${IMGUI})

add_executable(${PROJECT_NAME} ${COMPILATION_UNIT})

target_include_directories(${PROJECT_NAME} PUBLIC
        ${LIBS_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} OpenGL::GL ${SDL2_LIBRARIES} ${ASSIMP_LIBRARIES})
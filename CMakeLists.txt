cmake_minimum_required(VERSION 3.13)
set(NAME project)
project(${NAME})

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
#file(COPY ${PROJECT_SOURCE_DIR}/shaders DESTINATION ${PROJECT_SOURCE_DIR}/bin)

if (APPLE)
    set(CMAKE_CXX_FLAGS "-framework Cocoa -framework IOKit")
    # -framework OpenGL -framework CoreVideo
    add_compile_definitions(GL_SILENCE_DEPRECATION)
endif ()

add_executable(${PROJECT_NAME} src/main.cpp)

find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
endif ()
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/headers)
include_directories(${PROJECT_SOURCE_DIR}/src/modules)
include_directories(${PROJECT_SOURCE_DIR}/src/shaders)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLM REQUIRED glm)
include_directories(${GLM_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLM_LIBRARY_DIRS})

set(BREW_PATH /opt/homebrew/)
include_directories(/usr/local/include)
include_directories(${BREW_PATH}/include)
target_link_libraries(${PROJECT_NAME} ${BREW_PATH}lib/libglfw.3.4.dylib)

file(COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${PROJECT_SOURCE_DIR}/bin/assets/shaders/)

set(SHADER_DIR ${PROJECT_SOURCE_DIR}/assets/shaders/src)
file(GLOB SHADER_FILES ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag)

# compile shaders
foreach (SHADER_FILE ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    add_custom_command(TARGET ${PROJECT_NAME}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR}/bin/
            COMMAND glslc ${SHADER_FILE} -o ${SHADER_DIR}/bin/${SHADER_NAME}.spv
    )
endforeach ()

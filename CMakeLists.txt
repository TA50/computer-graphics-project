if (APPLE)
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
    if (Vulkan_FOUND)
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
else ()
    cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
    set(NAME main)
    set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1)

    set(CMAKE_CXX_STANDARD 20)


    project(${NAME})


    set(SRC_DIRECTORY "${PROJECT_SOURCE_DIR}/src")


    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "Running in Debug mode")
        set(OUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Debug")
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Running in Release mode")
        set(OUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Release")
    else ()
        message("Unknown mode: " ${CMAKE_BUILD_TYPE})
    endif ()


    message(STATUS "Output directory: " ${OUT_DIRECTORY})
    set(SHADER_DIR "${OUT_DIRECTORY}/assets/shaders")

    # Copy Assets
    file(COPY ${PROJECT_SOURCE_DIR}/assets DESTINATION ${OUT_DIRECTORY})


    set(GLFW_PATH "C:/VulkanSDK/glfw-3.4.bin.WIN64")
    set(VULKAN_PATH "C:/VulkanSDK/1.3.275.0")


    include_directories(${VULKAN_PATH}/Include)
    include_directories(${GLFW_PATH}/include)

    include_directories(${SRC_DIRECTORY}/headers)
    include_directories(${SRC_DIRECTORY})
    include_directories(${PROJECT_SOURCE_DIR}/assets)

    link_directories(
            ${GLFW_PATH}/lib-vc2022
            ${VULKAN_PATH}/Lib
    )


    add_executable(${PROJECT_NAME} ${SRC_DIRECTORY}/main.cpp)

    target_link_libraries(${PROJECT_NAME}
            gdi32.lib opengl32.lib kernel32.lib user32.lib shell32.lib glfw3.lib vulkan-1.lib

    )


    # Build Shaders


    FILE(GLOB SHADERS "${SHADER_DIR}/src/*")
    message(STATUS "Building Shaders")
    foreach (SHADER ${SHADERS})
        message(STATUS "Building Shader: " ${SHADER})
        get_filename_component(SHADER_NAME ${SHADER} NAME)
        add_custom_command(TARGET ${PROJECT_NAME}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR}/bin
                COMMAND glslc ${SHADER} -o ${SHADER_DIR}/bin/${SHADER_NAME}.spv
        )
    ENDFOREACH ()

    message(STATUS "Finish Building Shaders")

endif ()
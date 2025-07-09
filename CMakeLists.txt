cmake_minimum_required(VERSION 3.16)

# Установка vcpkg toolchain ПЕРЕД project()
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Pasha/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TruckLoadingSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти пакеты через vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Создать папки для внешних библиотек
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/imgui)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/external/stb)

# Скачать ImGui если не существует
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp)
    message(STATUS "Downloading Dear ImGui...")
    file(DOWNLOAD
            "https://github.com/ocornut/imgui/archive/refs/tags/v1.90.1.zip"
            "${CMAKE_BINARY_DIR}/imgui.zip"
            SHOW_PROGRESS
    )
    execute_process(
            COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_BINARY_DIR}/imgui.zip"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    file(COPY ${CMAKE_BINARY_DIR}/imgui-1.90.1/ DESTINATION ${CMAKE_SOURCE_DIR}/external/imgui/)
endif()

# Скачать stb_image.h если не существует
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/stb/stb_image.h)
    message(STATUS "Downloading stb_image.h...")
    file(DOWNLOAD
            "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
            "${CMAKE_SOURCE_DIR}/external/stb/stb_image.h"
            SHOW_PROGRESS
    )
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui)
include_directories(${CMAKE_SOURCE_DIR}/external/stb)

# ImGui sources
set(IMGUI_SOURCES
        external/imgui/imgui.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/backends/imgui_impl_opengl3.cpp
)

# Основные исходники проекта
set(PROJECT_SOURCES
        src/main.cpp
        src/graphics/Camera.cpp
        src/graphics/Shader.cpp
        src/graphics/Mesh.cpp
        src/graphics/Model.cpp
        src/core/PerformanceManager.cpp
)

# Создаем исполняемый файл
add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        ${IMGUI_SOURCES}
)

# Линкуем библиотеки из vcpkg
target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        glad::glad
        assimp::assimp
        glm::glm
)

# Компилятор-специфичные настройки для максимальной производительности
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
                /O2 /Ob2 /Oi /Ot /Oy /GL /DNDEBUG /arch:AVX2)
        target_link_options(${PROJECT_NAME} PRIVATE /LTCG /OPT:REF /OPT:ICF)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE
                -O3 -march=native -DNDEBUG -ffast-math -flto)
        target_link_options(${PROJECT_NAME} PRIVATE -flto)
    endif()
endif()

# Копируем ресурсы в папку сборки
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Создаем папки если их нет
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src/graphics)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/src/core)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/shaders)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/models)

message(STATUS "Project configured successfully!")
message(STATUS "Libraries found:")
message(STATUS "  - GLFW: ${glfw3_VERSION}")
message(STATUS "  - GLAD: Found")
message(STATUS "  - Assimp: ${assimp_VERSION}")
message(STATUS "  - GLM: ${glm_VERSION}")
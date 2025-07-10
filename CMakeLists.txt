cmake_minimum_required(VERSION 3.16)

# ВАЖНО: Установка vcpkg toolchain ПЕРЕД project()
set(CMAKE_TOOLCHAIN_FILE "C:/Users/Pasha/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(TruckLoadingSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Найти пакеты через vcpkg
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Загрузка ImGui если не существует
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp)
message(STATUS "Downloading Dear ImGui...")
include(FetchContent)
FetchContent_Declare(
imgui
GIT_REPOSITORY https://github.com/ocornut/imgui.git
GIT_TAG v1.90.1
)
FetchContent_MakeAvailable(imgui)
file(COPY ${imgui_SOURCE_DIR}/ DESTINATION ${CMAKE_SOURCE_DIR}/external/imgui/)
endif()

# Загрузка stb_image.h если не существует
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/external/stb/stb_image.h)
message(STATUS "Downloading stb_image.h...")
file(DOWNLOAD
"https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
"${CMAKE_SOURCE_DIR}/external/stb/stb_image.h"
SHOW_PROGRESS
)
endif()

# Создаем папки assets если не существуют
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/shaders)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/models)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/textures)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets/fonts)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui/backends)
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

# ИСПРАВЛЕНО: Все исходные файлы проекта
set(PROJECT_SOURCES
src/main.cpp
src/core/Application.cpp
src/core/Window.cpp
src/core/Renderer.cpp
src/scene/Scene.cpp
src/scene/Transform.cpp
src/scene/GameObject.cpp
src/graphics/Camera.cpp
src/graphics/Shader.cpp
src/graphics/Model.cpp
src/graphics/Mesh.cpp
src/graphics/Material.cpp
)

# Проверяем наличие всех файлов
foreach(source_file ${PROJECT_SOURCES})
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${source_file})
message(WARNING "File does not exist: ${source_file}")
else()
message(STATUS "Found source file: ${source_file}")
endif()
endforeach()

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

# Компилятор-специфичные настройки
if(MSVC)
target_compile_options(${PROJECT_NAME} PRIVATE /W4)
# Отключаем некоторые предупреждения для ImGui и внешних библиотек
target_compile_options(${PROJECT_NAME} PRIVATE /wd4267 /wd4244 /wd4701 /wd4996)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
target_compile_options(${PROJECT_NAME} PRIVATE /O2 /Ob2 /DNDEBUG)
endif()
else()
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -DNDEBUG)
endif()
endif()

# Копируем ресурсы в папку сборки
if(EXISTS ${CMAKE_SOURCE_DIR}/assets)
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
endif()

message(STATUS "Project configured successfully!")
message(STATUS "Vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Libraries found:")
message(STATUS "  - GLFW: ${glfw3_VERSION}")
message(STATUS "  - GLAD: Found")
message(STATUS "  - Assimp: ${assimp_VERSION}")
message(STATUS "  - GLM: ${glm_VERSION}")

# Создание пустых файлов моделей для тестирования (если не существуют)
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/assets/models/lorry.obj)
message(STATUS "Creating placeholder model files...")
file(WRITE ${CMAKE_SOURCE_DIR}/assets/models/lorry.obj "# Placeholder OBJ file for truck model\n")
file(WRITE ${CMAKE_SOURCE_DIR}/assets/models/weel.obj "# Placeholder OBJ file for wheel model\n")
endif()
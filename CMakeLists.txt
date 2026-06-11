cmake_minimum_required(VERSION 3.15)
project(Programowanie_projekt)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML CONFIG REQUIRED COMPONENTS System Window Graphics)
find_package(assimp CONFIG REQUIRED)

add_executable(app
    main.cpp
    Camera.cpp
    Mesh.cpp
    Light.h
    Debug.cpp
    input.cpp
    Import.cpp
    Primitives.cpp
    Transform.cpp
    Triangle.cpp
)

target_link_libraries(app PRIVATE
    SFML::System
    SFML::Window
    SFML::Graphics
    assimp::assimp
)
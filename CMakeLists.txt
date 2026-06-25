cmake_minimum_required(VERSION 3.24)

# Proje adı ve sürüm tanımlaması
project(HolyLevelFactory VERSION 1.0.0 LANGUAGES CXX)

# Geode SDK kütüphanesini projeye dahil ediyoruz
find_package(Geode REQUIRED)

# Derleme için modern C++20 standardını zorunlu kılıyoruz
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ana kaynak dosyamızı (main.cpp) Geode moduna kaydediyoruz
add_geode_mod(
    ${PROJECT_NAME}
    ${PROJECT_VERSION}
    SOURCES main.cpp
)

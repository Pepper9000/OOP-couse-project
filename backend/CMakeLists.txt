cmake_minimum_required(VERSION 3.16)

project(SmartWardrobeDomain LANGUAGES CXX)

add_library(smart_wardrobe_domain
    src/Clothing.cpp
    src/Top.cpp
    src/Bottom.cpp
    src/Shoes.cpp
)

target_include_directories(smart_wardrobe_domain PUBLIC include)
target_compile_features(smart_wardrobe_domain PUBLIC cxx_std_17)
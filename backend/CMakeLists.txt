cmake_minimum_required(VERSION 3.16)

project(SmartWardrobeDomain LANGUAGES CXX)

add_library(smart_wardrobe_domain
    src/Clothing.cpp
    src/Top.cpp
    src/Bottom.cpp
    src/Shoes.cpp
    src/ClothingFactory.cpp
    src/WardrobeManager.cpp
    src/CSVRepository.cpp
    src/Outfit.cpp
    src/RecommendationEngine.cpp
    src/JsonSerialization.cpp
)

target_include_directories(smart_wardrobe_domain PUBLIC include)
target_compile_features(smart_wardrobe_domain PUBLIC cxx_std_17)

add_executable(smart_wardrobe_server
    src/main.cpp
    src/Server.cpp
)
target_link_libraries(smart_wardrobe_server PRIVATE smart_wardrobe_domain)
target_include_directories(smart_wardrobe_server PRIVATE include)
if(WIN32)
    target_link_libraries(smart_wardrobe_server PRIVATE ws2_32)
endif()

enable_testing()

add_executable(test_wardrobe tests/test_wardrobe.cpp)
target_link_libraries(test_wardrobe PRIVATE smart_wardrobe_domain)
target_include_directories(test_wardrobe PRIVATE include)
add_test(NAME WardrobeTests COMMAND test_wardrobe)

add_executable(test_csv_repository tests/test_csv_repository.cpp)
target_link_libraries(test_csv_repository PRIVATE smart_wardrobe_domain)
target_include_directories(test_csv_repository PRIVATE include)
add_test(NAME CSVRepositoryTests COMMAND test_csv_repository)

add_executable(test_recommendation_engine tests/test_recommendation_engine.cpp)
target_link_libraries(test_recommendation_engine PRIVATE smart_wardrobe_domain)
target_include_directories(test_recommendation_engine PRIVATE include)
add_test(NAME RecommendationEngineTests COMMAND test_recommendation_engine)

add_executable(test_wardrobe_integration tests/test_wardrobe_integration.cpp)
target_link_libraries(test_wardrobe_integration PRIVATE smart_wardrobe_domain)
target_include_directories(test_wardrobe_integration PRIVATE include)
add_test(NAME WardrobeIntegrationTests COMMAND test_wardrobe_integration)

add_executable(test_json_serialization tests/test_json_serialization.cpp)
target_link_libraries(test_json_serialization PRIVATE smart_wardrobe_domain)
target_include_directories(test_json_serialization PRIVATE include)
add_test(NAME JsonSerializationTests COMMAND test_json_serialization)

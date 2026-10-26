#include "ClothingFactory.h"
#include "JsonSerialization.h"
#include "Outfit.h"
#include "RecommendationEngine.h"
#include "WardrobeManager.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace smartwardrobe;

static int testCount = 0;
static int passCount = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        ++testCount; \
        if (!(cond)) { \
            std::cerr << "FAIL [line " << __LINE__ << "]: " << msg << std::endl; \
            assert(false); \
        } else { \
            ++passCount; \
        } \
    } while (0)

void testClothingSerialization() {
    std::cout << "Running testClothingSerialization..." << std::endl;

    auto item = ClothingFactory::create("T-Shirt", 1, "White", {"Casual", "Summer"}, {"All Season"});
    TEST_ASSERT(item != nullptr, "Factory item created");

    auto j = JsonSerialization::clothingToJson(*item);
    TEST_ASSERT(j["id"] == 1, "ID serialized");
    TEST_ASSERT(j["type"] == "Top", "Type serialized");
    TEST_ASSERT(j["subtype"] == "T-Shirt", "Subtype serialized");
    TEST_ASSERT(j["color"] == "White", "Color serialized");
    TEST_ASSERT(j["occasions"].size() == 2, "Occasions array size 2");
    TEST_ASSERT(j["seasons"].size() == 1, "Seasons array size 1");

    // List serialization
    std::vector<const Clothing*> list = { item.get() };
    auto jList = JsonSerialization::clothingListToJson(list);
    TEST_ASSERT(jList.is_array() && jList.size() == 1, "List serialized as array of size 1");
}

void testClothingDeserialization() {
    std::cout << "Running testClothingDeserialization..." << std::endl;

    nlohmann::json valid = {
        {"type", "Bottom"},
        {"subtype", "Jeans"},
        {"color", "Blue"},
        {"occasions", {"Casual", "Smart Casual"}},
        {"seasons", {"All Season"}}
    };

    std::string err;
    auto item = JsonSerialization::jsonToClothing(valid, 42, &err);
    TEST_ASSERT(item != nullptr, "Valid JSON converts to Clothing: " + err);
    if (item) {
        TEST_ASSERT(item->getId() == 42, "Assigned ID matches 42");
        TEST_ASSERT(item->getType() == "Bottom", "Type matches Bottom");
        TEST_ASSERT(item->getSubtype() == "Jeans", "Subtype matches Jeans");
        TEST_ASSERT(item->getColor() == "Blue", "Color matches Blue");
    }

    // Pipe string tags handling
    nlohmann::json pipeTags = {
        {"type", "Shoes"},
        {"subtype", "Sneakers"},
        {"color", "White"},
        {"occasions", "Active|Casual"},
        {"seasons", "Summer"}
    };
    auto pipeItem = JsonSerialization::jsonToClothing(pipeTags, 43, &err);
    TEST_ASSERT(pipeItem != nullptr, "Pipe-delimited tag string parsed correctly");
    if (pipeItem) {
        TEST_ASSERT(pipeItem->getOccasions().size() == 2, "2 occasions extracted from pipe string");
    }

    // Rejection: Invalid subtype
    nlohmann::json invalidSubtype = valid;
    invalidSubtype["subtype"] = "Sombrero";
    auto badItem = JsonSerialization::jsonToClothing(invalidSubtype, 44, &err);
    TEST_ASSERT(badItem == nullptr, "Unsupported subtype rejected");
    TEST_ASSERT(err.find("Unsupported clothing subtype") != std::string::npos, "Error reports unsupported subtype");

    // Rejection: Category mismatch
    nlohmann::json mismatched = valid;
    mismatched["type"] = "Top"; // Jeans is a Bottom!
    auto badTypeItem = JsonSerialization::jsonToClothing(mismatched, 45, &err);
    TEST_ASSERT(badTypeItem == nullptr, "Category mismatch rejected");
    TEST_ASSERT(err.find("belongs to category 'Bottom', not 'Top'") != std::string::npos, "Error reports category mismatch");

    // Rejection: Missing color
    nlohmann::json noColor = valid;
    noColor.erase("color");
    auto badColorItem = JsonSerialization::jsonToClothing(noColor, 46, &err);
    TEST_ASSERT(badColorItem == nullptr, "Missing color rejected");

    // Rejection: Empty occasions
    nlohmann::json noOcc = valid;
    noOcc["occasions"] = nlohmann::json::array();
    auto badOccItem = JsonSerialization::jsonToClothing(noOcc, 47, &err);
    TEST_ASSERT(badOccItem == nullptr, "Empty occasions rejected");
}

void testOutfitSerialization() {
    std::cout << "Running testOutfitSerialization..." << std::endl;

    WardrobeManager mgr;
    mgr.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    mgr.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"Summer"}));
    mgr.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Casual"}, {"Summer"}));

    auto outfits = RecommendationEngine::recommend(mgr);
    TEST_ASSERT(!outfits.empty(), "Outfits generated");

    auto jOutfits = JsonSerialization::outfitsToJson(outfits);
    TEST_ASSERT(jOutfits.is_array() && !jOutfits.empty(), "Serialized to JSON array");

    const auto& first = jOutfits[0];
    TEST_ASSERT(first.contains("score") && first["score"].is_number(), "Contains numeric score");
    TEST_ASSERT(first["top"] == "White T-Shirt", "Top description matches expected");
    TEST_ASSERT(first["bottom"] == "Blue Jeans", "Bottom description matches expected");
    TEST_ASSERT(first["shoes"] == "White Sneakers", "Shoes description matches expected");
    TEST_ASSERT(first["reason"].is_array() && !first["reason"].empty(), "Contains reason array");
}

void testCriteriaParsing() {
    std::cout << "Running testCriteriaParsing..." << std::endl;

    nlohmann::json j = {
        {"occasion", "Casual"},
        {"season", "Winter"},
        {"maxResults", 5}
    };
    auto crit = JsonSerialization::parseCriteria(j);
    TEST_ASSERT(crit.occasion == "Casual", "Occasion parsed");
    TEST_ASSERT(crit.season == "Winter", "Season parsed");
    TEST_ASSERT(crit.maxResults == 5, "maxResults parsed");

    // Defaults on empty object
    auto emptyCrit = JsonSerialization::parseCriteria(nlohmann::json::object());
    TEST_ASSERT(emptyCrit.occasion.empty(), "Default occasion is empty");
    TEST_ASSERT(emptyCrit.season.empty(), "Default season is empty");
    TEST_ASSERT(emptyCrit.maxResults == 10, "Default maxResults is 10");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Smart Wardrobe - JSON Serialization Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    testClothingSerialization();
    testClothingDeserialization();
    testOutfitSerialization();
    testCriteriaParsing();

    std::cout << "========================================" << std::endl;
    std::cout << "Summary: " << passCount << " / " << testCount << " assertions passed." << std::endl;
    std::cout << "ALL JSON SERIALIZATION TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

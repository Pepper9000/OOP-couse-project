#include "ClothingFactory.h"
#include "WardrobeManager.h"
#include "Top.h"
#include "Bottom.h"
#include "Shoes.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

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

void testFactorySupportedSubtypes() {
    std::cout << "Running testFactorySupportedSubtypes..." << std::endl;

    struct SubtypeExpectation {
        std::string subtype;
        std::string expectedType;
    };

    std::vector<SubtypeExpectation> subtypes = {
        {"T-Shirt", "Top"},
        {"Shirt", "Top"},
        {"Hoodie", "Top"},
        {"Sweater", "Top"},
        {"Jeans", "Bottom"},
        {"Cargo", "Bottom"},
        {"Trousers", "Bottom"},
        {"Sweatpants", "Bottom"},
        {"Sneakers", "Shoes"},
        {"Boots", "Shoes"},
        {"Formal Shoes", "Shoes"}
    };

    int id = 1;
    for (const auto& entry : subtypes) {
        TEST_ASSERT(ClothingFactory::isSupportedSubtype(entry.subtype),
                    "Subtype should be reported as supported: " + entry.subtype);
        TEST_ASSERT(ClothingFactory::getTypeForSubtype(entry.subtype) == entry.expectedType,
                    "Type for subtype " + entry.subtype + " should match " + entry.expectedType);

        auto item = ClothingFactory::create(entry.subtype, id++, "Blue", {"Casual"}, {"Summer"});
        TEST_ASSERT(item != nullptr, "Factory should instantiate " + entry.subtype);
        if (item) {
            TEST_ASSERT(item->getType() == entry.expectedType, "getType() must match expected");
            TEST_ASSERT(item->getSubtype() == entry.subtype, "getSubtype() must match expected");
            TEST_ASSERT(item->getColor() == "Blue", "getColor() must match");
            TEST_ASSERT(item->getOccasions().size() == 1 && item->getOccasions()[0] == "Casual", "Occasion matches");
            TEST_ASSERT(item->getSeasons().size() == 1 && item->getSeasons()[0] == "Summer", "Season matches");
        }
    }
}

void testFactoryRejections() {
    std::cout << "Running testFactoryRejections..." << std::endl;

    // Unknown subtypes
    TEST_ASSERT(!ClothingFactory::isSupportedSubtype("Hat"), "Hat is not supported");
    TEST_ASSERT(!ClothingFactory::isSupportedSubtype(""), "Empty string is not supported");
    TEST_ASSERT(ClothingFactory::getTypeForSubtype("Unknown") == "", "Unknown subtype should return empty type");

    auto unknown1 = ClothingFactory::create("Hat", 100, "Black", {"Casual"}, {"All Season"});
    TEST_ASSERT(unknown1 == nullptr, "Factory must reject unknown subtype 'Hat'");

    auto unknown2 = ClothingFactory::create("", 101, "Red", {"Party"}, {"Winter"});
    TEST_ASSERT(unknown2 == nullptr, "Factory must reject empty subtype");

    // Mismatched type and subtype
    auto mismatch = ClothingFactory::create("Shoes", "T-Shirt", 102, "White", {"Casual"}, {"Summer"});
    TEST_ASSERT(mismatch == nullptr, "Factory must reject subtype 'T-Shirt' with mismatched category 'Shoes'");

    // Matched type and subtype
    auto matched = ClothingFactory::create("Top", "T-Shirt", 103, "White", {"Casual"}, {"Summer"});
    TEST_ASSERT(matched != nullptr, "Factory must accept matched category and subtype");
}

void testManagerAddAndRetrieve() {
    std::cout << "Running testManagerAddAndRetrieve..." << std::endl;

    WardrobeManager manager;
    TEST_ASSERT(manager.empty(), "Manager should be empty initially");
    TEST_ASSERT(manager.size() == 0, "Manager size should be 0");

    auto item = ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"});
    bool added = manager.addItem(std::move(item));
    TEST_ASSERT(added, "addItem should return true for valid item");
    TEST_ASSERT(!manager.empty(), "Manager should not be empty after add");
    TEST_ASSERT(manager.size() == 1, "Manager size should be 1");
    TEST_ASSERT(manager.contains(1), "Manager should contain ID 1");

    // Retrieve item
    Clothing* found = manager.findById(1);
    TEST_ASSERT(found != nullptr, "findById should locate item with ID 1");
    if (found) {
        TEST_ASSERT(found->getId() == 1, "ID must match");
        TEST_ASSERT(found->getSubtype() == "T-Shirt", "Subtype must match");
        TEST_ASSERT(found->getColor() == "White", "Color must match");
    }

    // Const retrieval
    const WardrobeManager& constMgr = manager;
    const Clothing* constFound = constMgr.findById(1);
    TEST_ASSERT(constFound != nullptr, "const findById should work");
}

void testManagerListItems() {
    std::cout << "Running testManagerListItems..." << std::endl;

    WardrobeManager manager;
    manager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    manager.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Casual", "Active"}, {"All Season"}));

    auto allItems = manager.getAllItems();
    TEST_ASSERT(allItems.size() == 3, "getAllItems should return 3 items");

    const auto& internalItems = manager.getItems();
    TEST_ASSERT(internalItems.size() == 3, "getItems reference should have 3 items");
}

void testManagerRemoveItem() {
    std::cout << "Running testManagerRemoveItem..." << std::endl;

    WardrobeManager manager;
    manager.addItem(ClothingFactory::create("Jeans", 10, "Black", {"Formal"}, {"Winter"}));
    manager.addItem(ClothingFactory::create("Boots", 11, "Brown", {"Formal"}, {"Winter"}));

    TEST_ASSERT(manager.size() == 2, "Size should be 2 before remove");
    bool removed = manager.removeItem(10);
    TEST_ASSERT(removed, "removeItem(10) should succeed");
    TEST_ASSERT(manager.size() == 1, "Size should be 1 after remove");
    TEST_ASSERT(!manager.contains(10), "ID 10 should no longer exist");
    TEST_ASSERT(manager.findById(10) == nullptr, "findById(10) should return nullptr");

    // Remove non-existent ID
    bool removedAgain = manager.removeItem(10);
    TEST_ASSERT(!removedAgain, "Removing already removed item should return false");

    bool removedMissing = manager.removeItem(999);
    TEST_ASSERT(!removedMissing, "Removing non-existent ID 999 should return false");
}

void testManagerEdgeCases() {
    std::cout << "Running testManagerEdgeCases..." << std::endl;

    WardrobeManager manager;

    // Null input
    TEST_ASSERT(!manager.addItem(nullptr), "addItem(nullptr) must return false");
    TEST_ASSERT(!manager.updateItem(nullptr), "updateItem(nullptr) must return false");
    TEST_ASSERT(manager.size() == 0, "Size should remain 0 after rejected nulls");

    // Missing ID
    TEST_ASSERT(manager.findById(404) == nullptr, "findById(404) should return nullptr");
    TEST_ASSERT(!manager.contains(404), "contains(404) should return false");

    // Duplicate IDs
    manager.addItem(ClothingFactory::create("Hoodie", 5, "Grey", {"Casual"}, {"Winter"}));
    TEST_ASSERT(manager.size() == 1, "Size should be 1");

    auto duplicate = ClothingFactory::create("Sweater", 5, "Navy", {"Casual"}, {"Winter"});
    bool dupAdded = manager.addItem(std::move(duplicate));
    TEST_ASSERT(!dupAdded, "addItem with duplicate ID 5 must return false");
    TEST_ASSERT(manager.size() == 1, "Size must remain 1 after duplicate rejection");

    // Original item is unchanged
    Clothing* item = manager.findById(5);
    TEST_ASSERT(item != nullptr && item->getSubtype() == "Hoodie",
                "Original item should remain unchanged when duplicate is rejected");

    // Update item
    auto replacement = ClothingFactory::create("Hoodie", 5, "Black", {"Casual"}, {"Winter"});
    bool updated = manager.updateItem(std::move(replacement));
    TEST_ASSERT(updated, "updateItem with existing ID 5 must return true");
    TEST_ASSERT(manager.findById(5)->getColor() == "Black", "Color should now be Black");

    // Update non-existent item
    auto ghost = ClothingFactory::create("Hoodie", 999, "Red", {"Casual"}, {"Winter"});
    bool ghostUpdated = manager.updateItem(std::move(ghost));
    TEST_ASSERT(!ghostUpdated, "updateItem with non-existent ID 999 must return false");
}

void testManagerFiltering() {
    std::cout << "Running testManagerFiltering..." << std::endl;

    WardrobeManager manager;
    manager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    manager.addItem(ClothingFactory::create("Shirt", 2, "White", {"Formal", "Smart Casual"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Jeans", 3, "Blue", {"Casual"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Trousers", 4, "Black", {"Formal"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Sneakers", 5, "White", {"Casual", "Active"}, {"Summer"}));
    manager.addItem(ClothingFactory::create("Formal Shoes", 6, "Black", {"Formal"}, {"All Season"}));

    // Filter by type
    auto tops = manager.filterByType("Top");
    TEST_ASSERT(tops.size() == 2, "Should find 2 Tops");

    auto bottoms = manager.filterByType("Bottom");
    TEST_ASSERT(bottoms.size() == 2, "Should find 2 Bottoms");

    auto shoes = manager.filterByType("Shoes");
    TEST_ASSERT(shoes.size() == 2, "Should find 2 Shoes");

    // Filter by subtype
    auto jeansList = manager.filterBySubtype("Jeans");
    TEST_ASSERT(jeansList.size() == 1 && jeansList[0]->getId() == 3, "Should find 1 Jeans with ID 3");

    // Filter by color
    auto whiteItems = manager.filterByColor("White");
    TEST_ASSERT(whiteItems.size() == 3, "Should find 3 White items");

    // Filter by occasion
    auto casualItems = manager.filterByOccasion("Casual");
    TEST_ASSERT(casualItems.size() == 3, "Should find 3 Casual items (IDs 1, 3, 5)");

    auto formalItems = manager.filterByOccasion("Formal");
    TEST_ASSERT(formalItems.size() == 3, "Should find 3 Formal items (IDs 2, 4, 6)");

    // Filter by season
    auto summerItems = manager.filterBySeason("Summer");
    TEST_ASSERT(summerItems.size() == 2, "Should find 2 Summer items (IDs 1, 5)");

    // Multi-criteria filter
    FilterCriteria criteria;
    criteria.type = "Top";
    criteria.color = "White";
    criteria.occasion = "Casual";
    auto combined = manager.filter(criteria);
    TEST_ASSERT(combined.size() == 1 && combined[0]->getId() == 1,
                "Multi-criteria filter should find exactly item 1");
}

void testLiveCountAndRAII() {
    std::cout << "Running testLiveCountAndRAII..." << std::endl;

    std::size_t initialLive = Clothing::liveCount();

    {
        WardrobeManager manager;
        manager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
        manager.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"Summer"}));
        manager.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Casual"}, {"Summer"}));

        TEST_ASSERT(Clothing::liveCount() == initialLive + 3, "liveCount should increase by 3");

        manager.removeItem(2);
        TEST_ASSERT(Clothing::liveCount() == initialLive + 2, "liveCount should decrease by 1 after remove");
    }

    // After manager goes out of scope, all items should be destructed
    TEST_ASSERT(Clothing::liveCount() == initialLive, "liveCount should return to initialLive after manager destruction");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Smart Wardrobe - Phase 1 Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    testFactorySupportedSubtypes();
    testFactoryRejections();
    testManagerAddAndRetrieve();
    testManagerListItems();
    testManagerRemoveItem();
    testManagerEdgeCases();
    testManagerFiltering();
    testLiveCountAndRAII();

    std::cout << "========================================" << std::endl;
    std::cout << "Summary: " << passCount << " / " << testCount << " assertions passed." << std::endl;
    std::cout << "ALL TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

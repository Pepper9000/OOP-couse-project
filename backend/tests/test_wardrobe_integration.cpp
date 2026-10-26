#include "ClothingFactory.h"
#include "CSVRepository.h"
#include "Outfit.h"
#include "RecommendationEngine.h"
#include "WardrobeManager.h"

#include <cassert>
#include <cstdio>
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

void testEndToEndComponentChain() {
    std::cout << "Running testEndToEndComponentChain..." << std::endl;

    const std::string integrationFile = "temp_integration_test.csv";
    std::remove(integrationFile.c_str());

    // Step 1 & 2: Construct clothing objects with ClothingFactory and add to WardrobeManager
    WardrobeManager sourceManager;
    sourceManager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer", "All Season"}));
    sourceManager.addItem(ClothingFactory::create("Shirt", 2, "Light Blue", {"Formal", "Smart Casual"}, {"All Season"}));
    sourceManager.addItem(ClothingFactory::create("Jeans", 3, "Blue", {"Casual", "Smart Casual"}, {"All Season"}));
    sourceManager.addItem(ClothingFactory::create("Trousers", 4, "Black", {"Formal"}, {"All Season"}));
    sourceManager.addItem(ClothingFactory::create("Sneakers", 5, "White", {"Casual", "Smart Casual"}, {"All Season"}));
    sourceManager.addItem(ClothingFactory::create("Formal Shoes", 6, "Black", {"Formal"}, {"All Season"}));

    TEST_ASSERT(sourceManager.size() == 6, "Source manager contains 6 items");

    // Step 3: Persist items using CSVRepository
    std::string saveErr;
    bool saved = CSVRepository::save(sourceManager, integrationFile, saveErr);
    TEST_ASSERT(saved, "Saving to CSV should succeed: " + saveErr);

    // Step 4: Load the saved data into a fresh manager
    WardrobeManager reloadedManager;
    std::string loadErr;
    bool loaded = CSVRepository::load(integrationFile, reloadedManager, loadErr);
    TEST_ASSERT(loaded, "Loading from CSV should succeed: " + loadErr);
    TEST_ASSERT(reloadedManager.size() == 6, "Reloaded manager contains 6 items");

    // Step 5: Generate recommendations from the reloaded wardrobe
    // A. Casual recommendations
    RecommendationCriteria casualCrit;
    casualCrit.occasion = "Casual";
    casualCrit.season = "Summer";
    auto casualOutfits = RecommendationEngine::recommend(reloadedManager, casualCrit);

    TEST_ASSERT(!casualOutfits.empty(), "Casual recommendations generated from reloaded wardrobe");
    const Outfit& topCasual = casualOutfits[0];
    TEST_ASSERT(topCasual.getTop()->getSubtype() == "T-Shirt", "Top casual outfit uses T-Shirt");
    TEST_ASSERT(topCasual.getBottom()->getSubtype() == "Jeans", "Top casual outfit uses Jeans");
    TEST_ASSERT(topCasual.getShoes()->getSubtype() == "Sneakers", "Top casual outfit uses Sneakers");
    TEST_ASSERT(topCasual.getScore() >= 90, "Top casual outfit has high score");
    TEST_ASSERT(topCasual.getTopDescription() == "White T-Shirt", "Top description matches expected");

    // B. Formal recommendations
    RecommendationCriteria formalCrit;
    formalCrit.occasion = "Formal";
    auto formalOutfits = RecommendationEngine::recommend(reloadedManager, formalCrit);

    TEST_ASSERT(!formalOutfits.empty(), "Formal recommendations generated from reloaded wardrobe");
    const Outfit& topFormal = formalOutfits[0];
    TEST_ASSERT(topFormal.getTop()->getSubtype() == "Shirt", "Top formal outfit uses Shirt");
    TEST_ASSERT(topFormal.getBottom()->getSubtype() == "Trousers", "Top formal outfit uses Trousers");
    TEST_ASSERT(topFormal.getShoes()->getSubtype() == "Formal Shoes", "Top formal outfit uses Formal Shoes");
    TEST_ASSERT(topFormal.getScore() >= 85, "Top formal outfit has high score");

    // Step 6: Verify determinism across multiple runs
    auto formalOutfitsRun2 = RecommendationEngine::recommend(reloadedManager, formalCrit);
    TEST_ASSERT(formalOutfits.size() == formalOutfitsRun2.size(), "Deterministic count across runs");
    TEST_ASSERT(formalOutfits[0].getScore() == formalOutfitsRun2[0].getScore(), "Deterministic scores across runs");
    TEST_ASSERT(formalOutfits[0].getTop()->getId() == formalOutfitsRun2[0].getTop()->getId(), "Deterministic IDs across runs");

    // Step 7: Clean up temporary file
    std::remove(integrationFile.c_str());
}

void testDemoWardrobeIntegration() {
    std::cout << "Running testDemoWardrobeIntegration..." << std::endl;

    const std::string demoPath = "backend/data/demo_wardrobe.csv";
    WardrobeManager demoManager;
    std::string err;
    bool loaded = CSVRepository::load(demoPath, demoManager, err);
    TEST_ASSERT(loaded, "Demo wardrobe loaded successfully: " + err);
    TEST_ASSERT(demoManager.size() == 11, "Demo wardrobe contains exactly 11 items");

    // Recommend casual outfits
    RecommendationCriteria casualCrit;
    casualCrit.occasion = "Casual";
    casualCrit.maxResults = 5;
    auto casualOutfits = RecommendationEngine::recommend(demoManager, casualCrit);
    TEST_ASSERT(!casualOutfits.empty(), "Demo wardrobe generates casual outfits");
    TEST_ASSERT(casualOutfits.size() <= 5, "Result count respects maxResults limit");

    // Recommend formal outfits
    RecommendationCriteria formalCrit;
    formalCrit.occasion = "Formal";
    auto formalOutfits = RecommendationEngine::recommend(demoManager, formalCrit);
    TEST_ASSERT(!formalOutfits.empty(), "Demo wardrobe generates formal outfits");
    TEST_ASSERT(formalOutfits[0].getTop()->getSubtype() == "Shirt", "Formal outfit top is Shirt");
    TEST_ASSERT(formalOutfits[0].getBottom()->getSubtype() == "Trousers", "Formal outfit bottom is Trousers");
    TEST_ASSERT(formalOutfits[0].getShoes()->getSubtype() == "Formal Shoes", "Formal outfit shoes is Formal Shoes");

    // Recommend winter outfits
    RecommendationCriteria winterCrit;
    winterCrit.season = "Winter";
    auto winterOutfits = RecommendationEngine::recommend(demoManager, winterCrit);
    TEST_ASSERT(!winterOutfits.empty(), "Demo wardrobe generates winter outfits");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Smart Wardrobe - End-to-End Integration Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    testEndToEndComponentChain();
    testDemoWardrobeIntegration();

    std::cout << "========================================" << std::endl;
    std::cout << "Summary: " << passCount << " / " << testCount << " assertions passed." << std::endl;
    std::cout << "ALL INTEGRATION TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

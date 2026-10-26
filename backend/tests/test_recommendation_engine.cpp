#include "ClothingFactory.h"
#include "Outfit.h"
#include "RecommendationEngine.h"
#include "WardrobeManager.h"

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

void testCandidateGeneration() {
    std::cout << "Running testCandidateGeneration..." << std::endl;

    WardrobeManager emptyManager;
    auto rEmpty = RecommendationEngine::recommend(emptyManager);
    TEST_ASSERT(rEmpty.empty(), "Empty wardrobe produces no recommendations");

    // Only Tops present
    WardrobeManager onlyTops;
    onlyTops.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    auto rOnlyTops = RecommendationEngine::recommend(onlyTops);
    TEST_ASSERT(rOnlyTops.empty(), "Wardrobe with only Tops produces no recommendations");

    // Only Tops and Bottoms (missing Shoes)
    WardrobeManager topsAndBottoms;
    topsAndBottoms.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    topsAndBottoms.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"Summer"}));
    auto rTopsBottoms = RecommendationEngine::recommend(topsAndBottoms);
    TEST_ASSERT(rTopsBottoms.empty(), "Missing category produces no recommendations");

    // Exactly one item in each category
    WardrobeManager singleCombo;
    singleCombo.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    singleCombo.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"Summer"}));
    singleCombo.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Casual"}, {"Summer"}));
    auto rSingle = RecommendationEngine::recommend(singleCombo);
    TEST_ASSERT(rSingle.size() == 1, "One item per category produces exactly 1 outfit");
    TEST_ASSERT(rSingle[0].getTop()->getId() == 1, "Top matches ID 1");
    TEST_ASSERT(rSingle[0].getBottom()->getId() == 2, "Bottom matches ID 2");
    TEST_ASSERT(rSingle[0].getShoes()->getId() == 3, "Shoes match ID 3");

    // Multiple items: 2 Tops, 2 Bottoms, 2 Shoes = 8 combinations
    WardrobeManager multiCombo;
    multiCombo.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    multiCombo.addItem(ClothingFactory::create("Shirt", 2, "Blue", {"Casual"}, {"Summer"}));
    multiCombo.addItem(ClothingFactory::create("Jeans", 3, "Blue", {"Casual"}, {"Summer"}));
    multiCombo.addItem(ClothingFactory::create("Cargo", 4, "Brown", {"Casual"}, {"Summer"}));
    multiCombo.addItem(ClothingFactory::create("Sneakers", 5, "White", {"Casual"}, {"Summer"}));
    multiCombo.addItem(ClothingFactory::create("Boots", 6, "Brown", {"Casual"}, {"Summer"}));

    RecommendationCriteria allCriteria;
    allCriteria.maxResults = 20;
    auto rMulti = RecommendationEngine::recommend(multiCombo, allCriteria);
    TEST_ASSERT(rMulti.size() == 8, "2x2x2 wardrobe yields 8 outfit combinations");

    // Category membership and absence of duplicate items in each outfit
    for (const auto& o : rMulti) {
        TEST_ASSERT(o.getTop() != nullptr && o.getTop()->getType() == "Top", "Outfit must have Top");
        TEST_ASSERT(o.getBottom() != nullptr && o.getBottom()->getType() == "Bottom", "Outfit must have Bottom");
        TEST_ASSERT(o.getShoes() != nullptr && o.getShoes()->getType() == "Shoes", "Outfit must have Shoes");
        TEST_ASSERT(o.getTop()->getId() != o.getBottom()->getId(), "Top and Bottom must have distinct IDs");
        TEST_ASSERT(o.getBottom()->getId() != o.getShoes()->getId(), "Bottom and Shoes must have distinct IDs");
    }

    // Verify manager was NOT mutated
    TEST_ASSERT(multiCombo.size() == 6, "Manager item count remains unchanged after generation");
}

void testContextFiltering() {
    std::cout << "Running testContextFiltering..." << std::endl;

    WardrobeManager manager;
    // Casual Summer items
    manager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    manager.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"Summer", "All Season"}));
    manager.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Casual"}, {"All Season"}));

    // Formal Winter items
    manager.addItem(ClothingFactory::create("Shirt", 4, "Light Blue", {"Formal"}, {"Winter"}));
    manager.addItem(ClothingFactory::create("Trousers", 5, "Black", {"Formal"}, {"Winter"}));
    manager.addItem(ClothingFactory::create("Formal Shoes", 6, "Black", {"Formal"}, {"Winter"}));

    // Filter by Casual
    RecommendationCriteria casualCrit;
    casualCrit.occasion = "Casual";
    auto rCasual = RecommendationEngine::recommend(manager, casualCrit);
    TEST_ASSERT(!rCasual.empty(), "Casual criteria finds outfits");
    for (const auto& o : rCasual) {
        TEST_ASSERT(o.getTop()->getId() == 1, "Only casual top selected");
        TEST_ASSERT(o.getBottom()->getId() == 2, "Only casual bottom selected");
        TEST_ASSERT(o.getShoes()->getId() == 3, "Only casual shoes selected");
    }

    // Filter by Formal (case-insensitive test)
    RecommendationCriteria formalCrit;
    formalCrit.occasion = "formal";
    auto rFormal = RecommendationEngine::recommend(manager, formalCrit);
    TEST_ASSERT(!rFormal.empty(), "Case-insensitive 'formal' finds outfits");
    for (const auto& o : rFormal) {
        TEST_ASSERT(o.getTop()->getId() == 4, "Only formal top selected");
        TEST_ASSERT(o.getBottom()->getId() == 5, "Only formal bottom selected");
        TEST_ASSERT(o.getShoes()->getId() == 6, "Only formal shoes selected");
    }

    // Season filter: Winter (Jeans and Sneakers have All Season so they qualify, Shirt, Trousers, Formal Shoes have Winter)
    RecommendationCriteria winterCrit;
    winterCrit.season = "Winter";
    auto rWinter = RecommendationEngine::recommend(manager, winterCrit);
    TEST_ASSERT(!rWinter.empty(), "Winter criteria produces combinations");

    // Both occasion and season together
    RecommendationCriteria combinedCrit;
    combinedCrit.occasion = "Formal";
    combinedCrit.season = "Summer"; // Formal items are Winter, not Summer!
    auto rCombinedNone = RecommendationEngine::recommend(manager, combinedCrit);
    TEST_ASSERT(rCombinedNone.empty(), "No Formal Summer items exist, returns empty");

    // Tag matching with internal spaces: "Smart Casual"
    Clothing::TagList smartTags = {"Smart Casual", "Formal"};
    TEST_ASSERT(RecommendationEngine::matchesTag(smartTags, "smart casual", false),
                "Internal space preserved during case-insensitive match");
    TEST_ASSERT(!RecommendationEngine::matchesTag(smartTags, "Casual", false),
                "Partial substring 'Casual' does not match 'Smart Casual'");
}

void testColorAndSubtypeRules() {
    std::cout << "Running testColorAndSubtypeRules..." << std::endl;

    // Color harmony testing
    std::vector<std::string> reasons;
    int neutralScore = RecommendationEngine::scoreColorHarmony("White", "Black", reasons);
    TEST_ASSERT(neutralScore > 0, "White and Black receives positive neutral bonus");
    TEST_ASSERT(!reasons.empty(), "Reasons populated for neutral pairing");

    reasons.clear();
    int clashScore = RecommendationEngine::scoreColorHarmony("Red", "Green", reasons);
    TEST_ASSERT(clashScore < 0, "Red and Green receives a color clash penalty");

    reasons.clear();
    int unknownColorScore = RecommendationEngine::scoreColorHarmony("Magenta", "Cyan", reasons);
    TEST_ASSERT(unknownColorScore >= 0, "Unknown colors handled gracefully without crash");

    // Subtype compatibility testing
    reasons.clear();
    int strongSubtype = RecommendationEngine::scoreSubtypeCompatibility("T-Shirt", "Jeans", "Sneakers", reasons);
    TEST_ASSERT(strongSubtype > 30, "T-Shirt + Jeans + Sneakers has high compatibility");

    reasons.clear();
    int weakSubtype = RecommendationEngine::scoreSubtypeCompatibility("Shirt", "Sweatpants", "Formal Shoes", reasons);
    TEST_ASSERT(weakSubtype < -20, "Shirt + Sweatpants + Formal Shoes has heavy penalty");

    // Shoe integration
    reasons.clear();
    int brownBlackClash = RecommendationEngine::scoreShoeColorIntegration("Brown", "Black", "Grey", reasons);
    TEST_ASSERT(brownBlackClash < 0, "Brown shoes with Black trousers receives a penalty");
}

void testRankingAndDeterminism() {
    std::cout << "Running testRankingAndDeterminism..." << std::endl;

    WardrobeManager manager;
    // High compatibility casual: White T-Shirt + Blue Jeans + White Sneakers
    manager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Casual"}, {"All Season"}));

    // Incompatible casual: White T-Shirt + Grey Sweatpants + Black Formal Shoes
    manager.addItem(ClothingFactory::create("Sweatpants", 4, "Grey", {"Casual"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Formal Shoes", 5, "Black", {"Casual"}, {"All Season"}));

    RecommendationCriteria crit;
    crit.occasion = "Casual";
    crit.maxResults = 10;
    auto results = RecommendationEngine::recommend(manager, crit);

    TEST_ASSERT(results.size() >= 2, "Expected multiple candidate outfits");
    // Verify scores are within [0, 100]
    for (const auto& o : results) {
        TEST_ASSERT(o.getScore() >= 0 && o.getScore() <= 100, "Score must be bounded in [0, 100]");
        TEST_ASSERT(!o.getReasons().empty(), "Rationale list must not be empty");
    }

    // The top result should be T-Shirt + Jeans + Sneakers, NOT Sweatpants + Formal Shoes
    TEST_ASSERT(results[0].getBottom()->getSubtype() == "Jeans", "Top outfit pairs Jeans, not Sweatpants");
    TEST_ASSERT(results[0].getShoes()->getSubtype() == "Sneakers", "Top outfit pairs Sneakers, not Formal Shoes");
    TEST_ASSERT(results[0].getScore() > results.back().getScore(), "Compatible outfit outranks mismatched outfit");

    // Deterministic ranking: running again gives exact same order and scores
    auto results2 = RecommendationEngine::recommend(manager, crit);
    TEST_ASSERT(results.size() == results2.size(), "Result counts match across runs");
    for (std::size_t i = 0; i < results.size(); ++i) {
        TEST_ASSERT(results[i].getScore() == results2[i].getScore(), "Scores match exactly");
        TEST_ASSERT(results[i].getTop()->getId() == results2[i].getTop()->getId(), "Top IDs match");
        TEST_ASSERT(results[i].getBottom()->getId() == results2[i].getBottom()->getId(), "Bottom IDs match");
        TEST_ASSERT(results[i].getShoes()->getId() == results2[i].getShoes()->getId(), "Shoes IDs match");
    }

    // Test maxResults limit
    crit.maxResults = 1;
    auto limited = RecommendationEngine::recommend(manager, crit);
    TEST_ASSERT(limited.size() == 1, "maxResults = 1 limits results to 1");

    // maxResults <= 0 returns all
    crit.maxResults = 0;
    auto allOutfits = RecommendationEngine::recommend(manager, crit);
    TEST_ASSERT(allOutfits.size() == results.size(), "maxResults <= 0 returns all outfits");
}

void testOutfitAccessorsAndDescriptions() {
    std::cout << "Running testOutfitAccessorsAndDescriptions..." << std::endl;

    WardrobeManager manager;
    manager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    manager.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual"}, {"Summer"}));
    manager.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Casual"}, {"Summer"}));

    auto results = RecommendationEngine::recommend(manager);
    TEST_ASSERT(results.size() == 1, "One outfit generated");

    const Outfit& outfit = results[0];
    TEST_ASSERT(outfit.getTopDescription() == "White T-Shirt", "Top description matches format '<Color> <Subtype>'");
    TEST_ASSERT(outfit.getBottomDescription() == "Blue Jeans", "Bottom description matches format");
    TEST_ASSERT(outfit.getShoesDescription() == "White Sneakers", "Shoes description matches format");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Smart Wardrobe - Recommendation Engine Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    testCandidateGeneration();
    testContextFiltering();
    testColorAndSubtypeRules();
    testRankingAndDeterminism();
    testOutfitAccessorsAndDescriptions();

    std::cout << "========================================" << std::endl;
    std::cout << "Summary: " << passCount << " / " << testCount << " assertions passed." << std::endl;
    std::cout << "ALL RECOMMENDATION TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

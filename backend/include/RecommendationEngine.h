#pragma once

#include "Outfit.h"
#include "WardrobeManager.h"

#include <string>
#include <vector>

namespace smartwardrobe {

/**
 * Filter and search criteria for outfit recommendations.
 * Empty string fields indicate no restriction on that attribute.
 */
struct RecommendationCriteria {
    std::string occasion;   // Desired occasion (e.g. "Casual", "Formal", or empty for any)
    std::string season;     // Desired season (e.g. "Summer", "Winter", or empty for any)
    int maxResults = 10;    // Maximum recommendations to return (<= 0 means return all)
};

/**
 * Deterministic, rule-based recommendation engine.
 *
 * Evaluates candidate combinations of Top + Bottom + Shoes from a WardrobeManager,
 * applies occasion and season context filters, scores combinations based on
 * color harmony, subtype style compatibility, and shoe integration, and
 * returns ranked outfits with human-readable rationale.
 */
class RecommendationEngine {
public:
    RecommendationEngine() = default;

    /**
     * Generates a ranked list of outfit recommendations based on the wardrobe
     * and specified criteria.
     *
     * @param manager The WardrobeManager containing available clothing items.
     * @param criteria Target occasion, season, and result limit.
     * @return Sorted list of Outfit objects, ordered highest score first.
     */
    static std::vector<Outfit> recommend(
        const WardrobeManager& manager,
        const RecommendationCriteria& criteria = RecommendationCriteria{});

    /**
     * Calculates the compatibility score for a specific (Top, Bottom, Shoes) combination.
     * Appends human-readable explanations to `reasons`.
     *
     * @return Clamped integer score between 0 and 100.
     */
    static int scoreOutfit(const Top* top, const Bottom* bottom, const Shoes* shoes,
                           const RecommendationCriteria& criteria,
                           std::vector<std::string>& reasons);

    /**
     * Evaluates top and bottom color harmony.
     * Returns score delta and appends explanatory rationale.
     */
    static int scoreColorHarmony(const std::string& topColor, const std::string& bottomColor,
                                 std::vector<std::string>& reasons);

    /**
     * Evaluates shoe color integration with the outfit.
     * Returns score delta and appends explanatory rationale.
     */
    static int scoreShoeColorIntegration(const std::string& shoeColor,
                                         const std::string& bottomColor,
                                         const std::string& topColor,
                                         std::vector<std::string>& reasons);

    /**
     * Evaluates style compatibility among the clothing subtypes.
     * Returns score delta and appends explanatory rationale.
     */
    static int scoreSubtypeCompatibility(const std::string& topSubtype,
                                         const std::string& bottomSubtype,
                                         const std::string& shoeSubtype,
                                         std::vector<std::string>& reasons);

    /**
     * Tests whether a tag list contains a target tag, case-insensitively.
     * When isSeason is true, items tagged "All Season" match any requested season.
     */
    static bool matchesTag(const Clothing::TagList& tags, const std::string& targetTag, bool isSeason = false);
};

} // namespace smartwardrobe

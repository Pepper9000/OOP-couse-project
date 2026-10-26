#include "RecommendationEngine.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <utility>

namespace smartwardrobe {

namespace {

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string toLower(const std::string& str) {
    std::string lower = str;
    for (char& c : lower) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

bool isNeutral(const std::string& colorLower) {
    static const std::set<std::string> neutrals = {
        "white", "black", "grey", "gray", "beige", "brown", "navy"
    };
    return neutrals.find(colorLower) != neutrals.end();
}

} // namespace

bool RecommendationEngine::matchesTag(const Clothing::TagList& tags, const std::string& targetTag, bool isSeason) {
    std::string target = toLower(trim(targetTag));
    if (target.empty()) {
        return true;
    }

    for (const auto& tag : tags) {
        std::string current = toLower(trim(tag));
        if (current == target) {
            return true;
        }
        if (isSeason) {
            if (current == "all season" || target == "all season") {
                return true;
            }
        }
    }
    return false;
}

int RecommendationEngine::scoreColorHarmony(const std::string& topColor, const std::string& bottomColor,
                                            std::vector<std::string>& reasons) {
    std::string top = toLower(trim(topColor));
    std::string bottom = toLower(trim(bottomColor));

    if (top.empty() || bottom.empty()) {
        reasons.push_back("Baseline color evaluation");
        return 4;
    }

    bool topNeutral = isNeutral(top);
    bool bottomNeutral = isNeutral(bottom);

    // Both neutral
    if (topNeutral && bottomNeutral) {
        if (top == bottom) {
            reasons.push_back("Cohesive monochromatic palette (" + topColor + ")");
            return 8;
        }
        if ((top == "brown" && bottom == "black") || (top == "black" && bottom == "brown")) {
            reasons.push_back("Subtle color clash: pairing brown with black");
            return -5;
        }
        reasons.push_back("Timeless neutral combination (" + topColor + " and " + bottomColor + ")");
        return 12;
    }

    // One neutral, one color
    if (topNeutral || bottomNeutral) {
        reasons.push_back("Harmonious balance of neutral tone with color (" + topColor + " and " + bottomColor + ")");
        return 14;
    }

    // Both non-neutrals
    if ((top == "blue" && bottom == "light blue") || (top == "light blue" && bottom == "blue")) {
        reasons.push_back("Complementary tonal harmony (" + topColor + " and " + bottomColor + ")");
        return 10;
    }

    if ((top == "red" && bottom == "green") || (top == "green" && bottom == "red")) {
        reasons.push_back("High-contrast or potentially clashing colors (" + topColor + " and " + bottomColor + ")");
        return -10;
    }

    reasons.push_back("Expressive color pairing: " + topColor + " with " + bottomColor);
    return 4;
}

int RecommendationEngine::scoreShoeColorIntegration(const std::string& shoeColor,
                                                    const std::string& bottomColor,
                                                    const std::string& topColor,
                                                    std::vector<std::string>& reasons) {
    std::string shoe = toLower(trim(shoeColor));
    std::string bottom = toLower(trim(bottomColor));
    std::string top = toLower(trim(topColor));

    if (shoe.empty()) {
        return 4;
    }

    if (shoe == "white" || shoe == "black" || shoe == "grey" || shoe == "gray") {
        reasons.push_back("Versatile " + shoeColor + " shoes anchor the outfit cleanly");
        return 8;
    }

    if (shoe == "brown") {
        if (bottom == "black") {
            reasons.push_back("Brown shoes contrast awkwardly with black bottoms");
            return -8;
        }
        reasons.push_back("Brown footwear adds a warm, classic tone to the outfit");
        return 8;
    }

    if (shoe == bottom || shoe == top) {
        reasons.push_back("Shoe color (" + shoeColor + ") echoes the clothing palette");
        return 6;
    }

    reasons.push_back("Shoe color (" + shoeColor + ") complements the look");
    return 4;
}

int RecommendationEngine::scoreSubtypeCompatibility(const std::string& topSubtype,
                                                    const std::string& bottomSubtype,
                                                    const std::string& shoeSubtype,
                                                    std::vector<std::string>& reasons) {
    int delta = 0;

    // Top + Bottom rules
    if (topSubtype == "T-Shirt") {
        if (bottomSubtype == "Jeans") {
            delta += 20;
            reasons.push_back("Classic everyday pairing: T-Shirt and Jeans");
        } else if (bottomSubtype == "Cargo") {
            delta += 20;
            reasons.push_back("Relaxed utilitarian style: T-Shirt with Cargo bottoms");
        } else if (bottomSubtype == "Sweatpants") {
            delta += 18;
            reasons.push_back("Comfortable active/lounge pairing: T-Shirt and Sweatpants");
        } else if (bottomSubtype == "Trousers") {
            delta += 10;
            reasons.push_back("Modern smart-casual contrast: T-Shirt paired with Trousers");
        }
    } else if (topSubtype == "Shirt") {
        if (bottomSubtype == "Trousers") {
            delta += 22;
            reasons.push_back("Sharp and elegant combination: Shirt and Trousers");
        } else if (bottomSubtype == "Jeans") {
            delta += 20;
            reasons.push_back("Popular smart-casual look: Shirt with Jeans");
        } else if (bottomSubtype == "Cargo") {
            delta += 10;
            reasons.push_back("Casual shirt styling with Cargo bottoms");
        } else if (bottomSubtype == "Sweatpants") {
            delta -= 25;
            reasons.push_back("Style clash: formal Shirt with athletic Sweatpants");
        }
    } else if (topSubtype == "Hoodie") {
        if (bottomSubtype == "Jeans") {
            delta += 20;
            reasons.push_back("Streetwear classic: Hoodie paired with Jeans");
        } else if (bottomSubtype == "Cargo") {
            delta += 20;
            reasons.push_back("Practical casual style: Hoodie with Cargo bottoms");
        } else if (bottomSubtype == "Sweatpants") {
            delta += 20;
            reasons.push_back("Coordinated athletic/loungewear styling");
        } else if (bottomSubtype == "Trousers") {
            delta -= 20;
            reasons.push_back("Style clash: casual Hoodie with formal Trousers");
        }
    } else if (topSubtype == "Sweater") {
        if (bottomSubtype == "Jeans") {
            delta += 20;
            reasons.push_back("Cozy and stylish: Sweater paired with Jeans");
        } else if (bottomSubtype == "Trousers") {
            delta += 22;
            reasons.push_back("Refined smart-casual combination: Sweater and Trousers");
        } else if (bottomSubtype == "Cargo") {
            delta += 12;
            reasons.push_back("Casual autumn/winter style: Sweater with Cargo bottoms");
        } else if (bottomSubtype == "Sweatpants") {
            delta += 8;
            reasons.push_back("Relaxed indoor comfort combination");
        }
    }

    // Shoe + Bottom/Outfit rules
    if (shoeSubtype == "Formal Shoes") {
        if (bottomSubtype == "Trousers") {
            delta += 18;
            reasons.push_back("Formal Shoes complete the polished trouser silhouette");
        } else if (bottomSubtype == "Jeans") {
            delta += 8;
            reasons.push_back("Dressy smart-casual combination with Jeans");
        } else if (bottomSubtype == "Sweatpants") {
            delta -= 30;
            reasons.push_back("Severe style mismatch: Formal Shoes with Sweatpants");
        } else if (bottomSubtype == "Cargo") {
            delta -= 20;
            reasons.push_back("Style mismatch: Formal Shoes with rugged Cargo bottoms");
        }

        if (topSubtype == "Hoodie" || topSubtype == "T-Shirt") {
            delta -= 10;
            reasons.push_back("Formal footwear feels overly rigid with casual top");
        }
    } else if (shoeSubtype == "Sneakers") {
        if (bottomSubtype == "Sweatpants" || bottomSubtype == "Jeans" || bottomSubtype == "Cargo") {
            delta += 18;
            reasons.push_back("Sneakers naturally complement casual bottoms");
        } else if (bottomSubtype == "Trousers") {
            delta += 10;
            reasons.push_back("Contemporary smart-casual touch: Sneakers with Trousers");
        }
    } else if (shoeSubtype == "Boots") {
        if (bottomSubtype == "Jeans" || bottomSubtype == "Cargo") {
            delta += 18;
            reasons.push_back("Rugged, cohesive pairing of Boots with " + bottomSubtype);
        } else if (bottomSubtype == "Trousers") {
            delta += 12;
            reasons.push_back("Polished boot profile with Trousers");
        } else if (bottomSubtype == "Sweatpants") {
            delta -= 20;
            reasons.push_back("Awkward pairing: Boots with Sweatpants");
        }
    }

    return delta;
}

int RecommendationEngine::scoreOutfit(const Top* top, const Bottom* bottom, const Shoes* shoes,
                                      const RecommendationCriteria& criteria,
                                      std::vector<std::string>& reasons) {
    if (!top || !bottom || !shoes) {
        return 0;
    }

    // Calibrated base score (20 points) prevents score saturation while
    // giving an unconstrained, well-distributed range up to ~92 for peak outfits.
    int score = 20;

    // Context bonuses
    if (!criteria.occasion.empty() && matchesTag(top->getOccasions(), criteria.occasion, false) &&
        matchesTag(bottom->getOccasions(), criteria.occasion, false) &&
        matchesTag(shoes->getOccasions(), criteria.occasion, false)) {
        score += 5;
        reasons.push_back("Fully matches requested occasion (" + criteria.occasion + ")");
    }

    if (!criteria.season.empty() && matchesTag(top->getSeasons(), criteria.season, true) &&
        matchesTag(bottom->getSeasons(), criteria.season, true) &&
        matchesTag(shoes->getSeasons(), criteria.season, true)) {
        score += 5;
        reasons.push_back("Suitable for requested season (" + criteria.season + ")");
    }

    // Subtype compatibility (max +40, min -55)
    score += scoreSubtypeCompatibility(top->getSubtype(), bottom->getSubtype(), shoes->getSubtype(), reasons);

    // Color harmony (max +14, min -10)
    score += scoreColorHarmony(top->getColor(), bottom->getColor(), reasons);

    // Shoe color integration (max +8, min -8)
    score += scoreShoeColorIntegration(shoes->getColor(), bottom->getColor(), top->getColor(), reasons);

    return std::max(0, std::min(100, score));
}

std::vector<Outfit> RecommendationEngine::recommend(
    const WardrobeManager& manager,
    const RecommendationCriteria& criteria) {

    std::vector<const Top*> eligibleTops;
    std::vector<const Bottom*> eligibleBottoms;
    std::vector<const Shoes*> eligibleShoes;

    for (const auto& itemPtr : manager.getItems()) {
        if (!itemPtr) {
            continue;
        }
        const Clothing* item = itemPtr.get();

        if (!matchesTag(item->getOccasions(), criteria.occasion, false)) {
            continue;
        }
        if (!matchesTag(item->getSeasons(), criteria.season, true)) {
            continue;
        }

        if (const Top* top = dynamic_cast<const Top*>(item)) {
            eligibleTops.push_back(top);
        } else if (const Bottom* bottom = dynamic_cast<const Bottom*>(item)) {
            eligibleBottoms.push_back(bottom);
        } else if (const Shoes* shoes = dynamic_cast<const Shoes*>(item)) {
            eligibleShoes.push_back(shoes);
        }
    }

    if (eligibleTops.empty() || eligibleBottoms.empty() || eligibleShoes.empty()) {
        return {};
    }

    std::vector<Outfit> candidates;
    candidates.reserve(eligibleTops.size() * eligibleBottoms.size() * eligibleShoes.size());

    for (const Top* top : eligibleTops) {
        for (const Bottom* bottom : eligibleBottoms) {
            for (const Shoes* shoes : eligibleShoes) {
                std::vector<std::string> reasons;
                int score = scoreOutfit(top, bottom, shoes, criteria, reasons);
                candidates.emplace_back(top, bottom, shoes, score, std::move(reasons));
            }
        }
    }

    // Deterministic sorting: score DESC, then Top ID ASC, Bottom ID ASC, Shoes ID ASC
    std::sort(candidates.begin(), candidates.end(), [](const Outfit& a, const Outfit& b) {
        if (a.getScore() != b.getScore()) {
            return a.getScore() > b.getScore();
        }
        if (a.getTop()->getId() != b.getTop()->getId()) {
            return a.getTop()->getId() < b.getTop()->getId();
        }
        if (a.getBottom()->getId() != b.getBottom()->getId()) {
            return a.getBottom()->getId() < b.getBottom()->getId();
        }
        return a.getShoes()->getId() < b.getShoes()->getId();
    });

    if (criteria.maxResults > 0 && static_cast<int>(candidates.size()) > criteria.maxResults) {
        candidates.erase(candidates.begin() + criteria.maxResults, candidates.end());
    }

    return candidates;
}

} // namespace smartwardrobe

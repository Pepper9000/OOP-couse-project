#pragma once

#include "Top.h"
#include "Bottom.h"
#include "Shoes.h"

#include <string>
#include <vector>

namespace smartwardrobe {

/**
 * Represents a single recommended outfit combination composed of exactly:
 * - One Top
 * - One Bottom
 * - One pair of Shoes
 *
 * Includes an integer compatibility score (bounded 0 to 100) and
 * human-readable explanations of why the score was awarded.
 *
 * Lifetime & Ownership Semantics:
 * Outfit holds non-owning pointers (const Top*, const Bottom*, const Shoes*).
 * The referenced Clothing objects are owned by WardrobeManager and must remain
 * alive for the duration of the Outfit object's usage.
 * Outfit itself is value-copyable and movable without copying or slicing
 * any underlying Clothing instances.
 */
class Outfit {
public:
    Outfit() : top_(nullptr), bottom_(nullptr), shoes_(nullptr), score_(0) {}
    Outfit(const Top* top, const Bottom* bottom, const Shoes* shoes,
           int score, std::vector<std::string> reasons);

    const Top* getTop() const;
    const Bottom* getBottom() const;
    const Shoes* getShoes() const;

    int getScore() const;
    const std::vector<std::string>& getReasons() const;

    /**
     * UI / REST friendly descriptions formatted as "<Color> <Subtype>",
     * directly compatible with frontend color extraction and display.
     */
    std::string getTopDescription() const;
    std::string getBottomDescription() const;
    std::string getShoesDescription() const;

private:
    const Top* top_;
    const Bottom* bottom_;
    const Shoes* shoes_;
    int score_;
    std::vector<std::string> reasons_;
};

} // namespace smartwardrobe

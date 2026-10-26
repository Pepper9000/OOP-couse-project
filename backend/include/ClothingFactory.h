#pragma once

#include "Clothing.h"

#include <memory>
#include <string>
#include <vector>

namespace smartwardrobe {

/**
 * Factory class responsible for instantiating the correct concrete Clothing
 * subclasses based on subtype identifiers.
 *
 * Supported concrete subtypes:
 * - Tops: "T-Shirt", "Shirt", "Hoodie", "Sweater"
 * - Bottoms: "Jeans", "Cargo", "Trousers", "Sweatpants"
 * - Shoes: "Sneakers", "Boots", "Formal Shoes"
 */
class ClothingFactory {
public:
    /**
     * Determines whether the given subtype is recognized and supported.
     *
     * @param subtype Subtype name string.
     * @return true if supported, false otherwise.
     */
    static bool isSupportedSubtype(const std::string& subtype);

    /**
     * Returns the category type ("Top", "Bottom", "Shoes") corresponding to a subtype.
     *
     * @param subtype Subtype name string.
     * @return Category name ("Top", "Bottom", "Shoes"), or empty string if unsupported.
     */
    static std::string getTypeForSubtype(const std::string& subtype);

    /**
     * Returns a list of all currently supported subtype identifiers.
     */
    static std::vector<std::string> getSupportedSubtypes();

    /**
     * Constructs the appropriate concrete Clothing instance based on the subtype.
     *
     * @param subtype Concrete subtype name (e.g. "T-Shirt", "Jeans", "Sneakers").
     * @param id Unique integer identifier.
     * @param color Color name / description.
     * @param occasions Tag list of suitable occasions.
     * @param seasons Tag list of suitable seasons.
     * @return std::unique_ptr<Clothing> owning the newly created instance,
     *         or nullptr if the subtype is unsupported or invalid.
     */
    static std::unique_ptr<Clothing> create(
        const std::string& subtype,
        int id,
        std::string color,
        Clothing::TagList occasions,
        Clothing::TagList seasons);

    /**
     * Constructs a concrete Clothing instance, verifying that the provided category
     * type matches the subtype's expected category.
     *
     * @param type Expected category type ("Top", "Bottom", or "Shoes").
     * @param subtype Concrete subtype name.
     * @param id Unique integer identifier.
     * @param color Color name / description.
     * @param occasions Tag list of suitable occasions.
     * @param seasons Tag list of suitable seasons.
     * @return std::unique_ptr<Clothing> or nullptr if subtype is unsupported or type mismatches.
     */
    static std::unique_ptr<Clothing> create(
        const std::string& type,
        const std::string& subtype,
        int id,
        std::string color,
        Clothing::TagList occasions,
        Clothing::TagList seasons);
};

} // namespace smartwardrobe

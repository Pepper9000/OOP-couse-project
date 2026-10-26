#pragma once

#include "Clothing.h"
#include "Outfit.h"
#include "RecommendationEngine.h"

#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>

namespace smartwardrobe {

/**
 * Handles JSON serialization and deserialization between domain models
 * and frontend API contracts.
 */
class JsonSerialization {
public:
    /**
     * Serializes a single Clothing item into a JSON object.
     * Output fields: id, type, subtype, color, occasions, seasons.
     */
    static nlohmann::json clothingToJson(const Clothing& item);

    /**
     * Serializes a list of Clothing non-owning pointers into a JSON array.
     */
    static nlohmann::json clothingListToJson(const std::vector<const Clothing*>& items);
    static nlohmann::json clothingListToJson(const std::vector<Clothing*>& items);

    /**
     * Deserializes a Clothing JSON object into a concrete Clothing subclass instance.
     * Validates all required fields, types, and subtype/category consistency.
     *
     * @param j Input JSON object.
     * @param assignedId Unique integer ID to assign to the clothing object.
     * @param error Optional pointer to receive error description if parsing or validation fails.
     * @return std::unique_ptr<Clothing> on success, or nullptr on validation failure.
     */
    static std::unique_ptr<Clothing> jsonToClothing(
        const nlohmann::json& j,
        int assignedId,
        std::string* error = nullptr);

    /**
     * Serializes an Outfit object into a JSON representation matching frontend expectations:
     * { "score": 85, "top": "White T-Shirt", "bottom": "Blue Jeans", "shoes": "White Sneakers", "reason": [...] }
     */
    static nlohmann::json outfitToJson(const Outfit& outfit);

    /**
     * Serializes a vector of Outfit objects into a JSON array.
     */
    static nlohmann::json outfitsToJson(const std::vector<Outfit>& outfits);

    /**
     * Parses recommendation criteria from incoming POST JSON payload.
     */
    static RecommendationCriteria parseCriteria(const nlohmann::json& j);
};

} // namespace smartwardrobe

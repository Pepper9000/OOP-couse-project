#include "JsonSerialization.h"
#include "ClothingFactory.h"

#include <utility>

namespace smartwardrobe {

namespace {

Clothing::TagList extractTags(const nlohmann::json& j, const std::string& key) {
    Clothing::TagList result;
    if (!j.contains(key)) {
        return result;
    }
    const auto& val = j[key];
    if (val.is_array()) {
        for (const auto& elem : val) {
            if (elem.is_string() && !elem.get<std::string>().empty()) {
                result.push_back(elem.get<std::string>());
            }
        }
    } else if (val.is_string()) {
        std::string s = val.get<std::string>();
        std::string cur;
        for (char c : s) {
            if (c == '|') {
                if (!cur.empty()) {
                    result.push_back(cur);
                    cur.clear();
                }
            } else {
                cur += c;
            }
        }
        if (!cur.empty()) {
            result.push_back(cur);
        }
    }
    return result;
}

} // namespace

nlohmann::json JsonSerialization::clothingToJson(const Clothing& item) {
    nlohmann::json j;
    j["id"] = item.getId();
    j["type"] = item.getType();
    j["subtype"] = item.getSubtype();
    j["color"] = item.getColor();
    j["occasions"] = item.getOccasions();
    j["seasons"] = item.getSeasons();
    return j;
}

nlohmann::json JsonSerialization::clothingListToJson(const std::vector<const Clothing*>& items) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto* item : items) {
        if (item) {
            arr.push_back(clothingToJson(*item));
        }
    }
    return arr;
}

nlohmann::json JsonSerialization::clothingListToJson(const std::vector<Clothing*>& items) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto* item : items) {
        if (item) {
            arr.push_back(clothingToJson(*item));
        }
    }
    return arr;
}

std::unique_ptr<Clothing> JsonSerialization::jsonToClothing(
    const nlohmann::json& j,
    int assignedId,
    std::string* error) {

    if (!j.is_object()) {
        if (error) *error = "Expected JSON object for clothing item";
        return nullptr;
    }

    if (assignedId <= 0) {
        if (error) *error = "Assigned ID must be a positive integer";
        return nullptr;
    }

    if (!j.contains("subtype") || !j["subtype"].is_string()) {
        if (error) *error = "Missing or invalid 'subtype' field";
        return nullptr;
    }
    std::string subtype = j["subtype"].get<std::string>();
    if (!ClothingFactory::isSupportedSubtype(subtype)) {
        if (error) *error = "Unsupported clothing subtype '" + subtype + "'";
        return nullptr;
    }

    std::string expectedType = ClothingFactory::getTypeForSubtype(subtype);
    if (j.contains("type") && j["type"].is_string()) {
        std::string type = j["type"].get<std::string>();
        if (type != expectedType) {
            if (error) *error = "Subtype '" + subtype + "' belongs to category '" + expectedType + "', not '" + type + "'";
            return nullptr;
        }
    }

    if (!j.contains("color") || !j["color"].is_string() || j["color"].get<std::string>().empty()) {
        if (error) *error = "Missing or empty 'color' field";
        return nullptr;
    }
    std::string color = j["color"].get<std::string>();

    Clothing::TagList occasions = extractTags(j, "occasions");
    if (occasions.empty() && j.contains("occasion")) {
        occasions = extractTags(j, "occasion");
    }
    if (occasions.empty()) {
        if (error) *error = "At least one occasion must be specified";
        return nullptr;
    }

    Clothing::TagList seasons = extractTags(j, "seasons");
    if (seasons.empty() && j.contains("season")) {
        seasons = extractTags(j, "season");
    }
    if (seasons.empty()) {
        if (error) *error = "At least one season must be specified";
        return nullptr;
    }

    auto item = ClothingFactory::create(expectedType, subtype, assignedId, std::move(color), std::move(occasions), std::move(seasons));
    if (!item) {
        if (error) *error = "Failed to construct clothing item for subtype '" + subtype + "'";
        return nullptr;
    }

    return item;
}

nlohmann::json JsonSerialization::outfitToJson(const Outfit& outfit) {
    nlohmann::json j;
    j["score"] = outfit.getScore();
    j["top"] = outfit.getTopDescription();
    j["bottom"] = outfit.getBottomDescription();
    j["shoes"] = outfit.getShoesDescription();
    j["reason"] = outfit.getReasons();
    return j;
}

nlohmann::json JsonSerialization::outfitsToJson(const std::vector<Outfit>& outfits) {
    nlohmann::json arr = nlohmann::json::array();
    for (const auto& o : outfits) {
        arr.push_back(outfitToJson(o));
    }
    return arr;
}

RecommendationCriteria JsonSerialization::parseCriteria(const nlohmann::json& j) {
    RecommendationCriteria crit;
    if (!j.is_object()) {
        return crit;
    }
    if (j.contains("occasion") && j["occasion"].is_string()) {
        crit.occasion = j["occasion"].get<std::string>();
    }
    if (j.contains("season") && j["season"].is_string()) {
        crit.season = j["season"].get<std::string>();
    }
    if (j.contains("maxResults") && j["maxResults"].is_number_integer()) {
        crit.maxResults = j["maxResults"].get<int>();
    }
    return crit;
}

} // namespace smartwardrobe

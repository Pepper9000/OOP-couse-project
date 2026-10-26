#include "ClothingFactory.h"
#include "Top.h"
#include "Bottom.h"
#include "Shoes.h"

#include <utility>

namespace smartwardrobe {

bool ClothingFactory::isSupportedSubtype(const std::string& subtype) {
    return !getTypeForSubtype(subtype).empty();
}

std::string ClothingFactory::getTypeForSubtype(const std::string& subtype) {
    if (subtype == "T-Shirt" || subtype == "Shirt" ||
        subtype == "Hoodie" || subtype == "Sweater") {
        return "Top";
    }
    if (subtype == "Jeans" || subtype == "Cargo" ||
        subtype == "Trousers" || subtype == "Sweatpants") {
        return "Bottom";
    }
    if (subtype == "Sneakers" || subtype == "Boots" ||
        subtype == "Formal Shoes") {
        return "Shoes";
    }
    return "";
}

std::vector<std::string> ClothingFactory::getSupportedSubtypes() {
    return {
        "T-Shirt", "Shirt", "Hoodie", "Sweater",
        "Jeans", "Cargo", "Trousers", "Sweatpants",
        "Sneakers", "Boots", "Formal Shoes"
    };
}

std::unique_ptr<Clothing> ClothingFactory::create(
    const std::string& subtype,
    int id,
    std::string color,
    Clothing::TagList occasions,
    Clothing::TagList seasons) {

    if (subtype == "T-Shirt") {
        return std::make_unique<TShirt>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Shirt") {
        return std::make_unique<Shirt>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Hoodie") {
        return std::make_unique<Hoodie>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Sweater") {
        return std::make_unique<Sweater>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Jeans") {
        return std::make_unique<Jeans>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Cargo") {
        return std::make_unique<Cargo>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Trousers") {
        return std::make_unique<Trousers>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Sweatpants") {
        return std::make_unique<Sweatpants>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Sneakers") {
        return std::make_unique<Sneakers>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Boots") {
        return std::make_unique<Boots>(id, std::move(color), std::move(occasions), std::move(seasons));
    }
    if (subtype == "Formal Shoes") {
        return std::make_unique<FormalShoes>(id, std::move(color), std::move(occasions), std::move(seasons));
    }

    return nullptr;
}

std::unique_ptr<Clothing> ClothingFactory::create(
    const std::string& type,
    const std::string& subtype,
    int id,
    std::string color,
    Clothing::TagList occasions,
    Clothing::TagList seasons) {

    std::string expectedType = getTypeForSubtype(subtype);
    if (expectedType.empty() || (!type.empty() && type != expectedType)) {
        return nullptr;
    }

    return create(subtype, id, std::move(color), std::move(occasions), std::move(seasons));
}

} // namespace smartwardrobe

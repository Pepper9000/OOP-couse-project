#include "Bottom.h"

#include <utility>

namespace smartwardrobe {

Bottom::Bottom(int id, std::string subtype, std::string color,
               TagList occasions, TagList seasons)
    : Clothing(id, std::move(subtype), std::move(color),
               std::move(occasions), std::move(seasons)) {}

std::string Bottom::getType() const {
    return "Bottom";
}

Jeans::Jeans(int id, std::string color, TagList occasions, TagList seasons)
    : Bottom(id, "Jeans", std::move(color), std::move(occasions), std::move(seasons)) {}

Cargo::Cargo(int id, std::string color, TagList occasions, TagList seasons)
    : Bottom(id, "Cargo", std::move(color), std::move(occasions), std::move(seasons)) {}

Trousers::Trousers(int id, std::string color, TagList occasions, TagList seasons)
    : Bottom(id, "Trousers", std::move(color), std::move(occasions), std::move(seasons)) {}

Sweatpants::Sweatpants(int id, std::string color, TagList occasions, TagList seasons)
    : Bottom(id, "Sweatpants", std::move(color), std::move(occasions), std::move(seasons)) {}

} // namespace smartwardrobe
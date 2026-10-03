#include "Shoes.h"

#include <utility>

namespace smartwardrobe {

Shoes::Shoes(int id, std::string subtype, std::string color,
             TagList occasions, TagList seasons)
    : Clothing(id, std::move(subtype), std::move(color),
               std::move(occasions), std::move(seasons)) {}

std::string Shoes::getType() const {
    return "Shoes";
}

Sneakers::Sneakers(int id, std::string color, TagList occasions, TagList seasons)
    : Shoes(id, "Sneakers", std::move(color), std::move(occasions), std::move(seasons)) {}

Boots::Boots(int id, std::string color, TagList occasions, TagList seasons)
    : Shoes(id, "Boots", std::move(color), std::move(occasions), std::move(seasons)) {}

FormalShoes::FormalShoes(int id, std::string color, TagList occasions, TagList seasons)
    : Shoes(id, "Formal Shoes", std::move(color), std::move(occasions), std::move(seasons)) {}

} // namespace smartwardrobe
#include "Top.h"

#include <utility>

namespace smartwardrobe {

Top::Top(int id, std::string subtype, std::string color,
         TagList occasions, TagList seasons)
    : Clothing(id, std::move(subtype), std::move(color),
               std::move(occasions), std::move(seasons)) {}

std::string Top::getType() const {
    return "Top";
}

TShirt::TShirt(int id, std::string color, TagList occasions, TagList seasons)
    : Top(id, "T-Shirt", std::move(color), std::move(occasions), std::move(seasons)) {}

Shirt::Shirt(int id, std::string color, TagList occasions, TagList seasons)
    : Top(id, "Shirt", std::move(color), std::move(occasions), std::move(seasons)) {}

Hoodie::Hoodie(int id, std::string color, TagList occasions, TagList seasons)
    : Top(id, "Hoodie", std::move(color), std::move(occasions), std::move(seasons)) {}

Sweater::Sweater(int id, std::string color, TagList occasions, TagList seasons)
    : Top(id, "Sweater", std::move(color), std::move(occasions), std::move(seasons)) {}

} // namespace smartwardrobe
#include "Outfit.h"

#include <algorithm>
#include <utility>

namespace smartwardrobe {

Outfit::Outfit(const Top* top, const Bottom* bottom, const Shoes* shoes,
               int score, std::vector<std::string> reasons)
    : top_(top),
      bottom_(bottom),
      shoes_(shoes),
      score_(std::max(0, std::min(100, score))),
      reasons_(std::move(reasons)) {}

const Top* Outfit::getTop() const {
    return top_;
}

const Bottom* Outfit::getBottom() const {
    return bottom_;
}

const Shoes* Outfit::getShoes() const {
    return shoes_;
}

int Outfit::getScore() const {
    return score_;
}

const std::vector<std::string>& Outfit::getReasons() const {
    return reasons_;
}

std::string Outfit::getTopDescription() const {
    if (!top_) {
        return "";
    }
    return top_->getColor() + " " + top_->getSubtype();
}

std::string Outfit::getBottomDescription() const {
    if (!bottom_) {
        return "";
    }
    return bottom_->getColor() + " " + bottom_->getSubtype();
}

std::string Outfit::getShoesDescription() const {
    if (!shoes_) {
        return "";
    }
    return shoes_->getColor() + " " + shoes_->getSubtype();
}

} // namespace smartwardrobe

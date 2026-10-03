#include "Clothing.h"

#include <ostream>
#include <utility>

namespace smartwardrobe {

std::size_t Clothing::liveCount_ = 0;

Clothing::Clothing(int id, std::string subtype, std::string color,
                   TagList occasions, TagList seasons)
    : id_(id),
      subtype_(std::move(subtype)),
      color_(std::move(color)),
      occasions_(std::move(occasions)),
      seasons_(std::move(seasons)) {
    ++liveCount_;
}

Clothing::~Clothing() {
    --liveCount_;
}

int Clothing::getId() const {
    return id_;
}

const std::string& Clothing::getSubtype() const {
    return subtype_;
}

const std::string& Clothing::getColor() const {
    return color_;
}

const Clothing::TagList& Clothing::getOccasions() const {
    return occasions_;
}

const Clothing::TagList& Clothing::getSeasons() const {
    return seasons_;
}

std::size_t Clothing::liveCount() {
    return liveCount_;
}

std::ostream& operator<<(std::ostream& output, const Clothing& clothing) {
    return output << clothing.id_ << ", " << clothing.getType() << ", "
                  << clothing.subtype_ << ", " << clothing.color_;
}

} // namespace smartwardrobe
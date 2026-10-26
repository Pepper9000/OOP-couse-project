#include "WardrobeManager.h"

#include <algorithm>
#include <utility>

namespace smartwardrobe {

namespace {

bool containsTag(const Clothing::TagList& tags, const std::string& target) {
    return std::find(tags.begin(), tags.end(), target) != tags.end();
}

} // namespace

bool WardrobeManager::addItem(std::unique_ptr<Clothing> item) {
    if (!item) {
        return false;
    }
    if (contains(item->getId())) {
        return false;
    }
    items_.push_back(std::move(item));
    return true;
}

Clothing* WardrobeManager::findById(int id) {
    for (auto& item : items_) {
        if (item && item->getId() == id) {
            return item.get();
        }
    }
    return nullptr;
}

const Clothing* WardrobeManager::findById(int id) const {
    for (const auto& item : items_) {
        if (item && item->getId() == id) {
            return item.get();
        }
    }
    return nullptr;
}

bool WardrobeManager::contains(int id) const {
    return findById(id) != nullptr;
}

bool WardrobeManager::removeItem(int id) {
    auto it = std::find_if(items_.begin(), items_.end(), [id](const std::unique_ptr<Clothing>& item) {
        return item && item->getId() == id;
    });
    if (it != items_.end()) {
        items_.erase(it);
        return true;
    }
    return false;
}

bool WardrobeManager::updateItem(std::unique_ptr<Clothing> newItem) {
    if (!newItem) {
        return false;
    }
    int id = newItem->getId();
    auto it = std::find_if(items_.begin(), items_.end(), [id](const std::unique_ptr<Clothing>& item) {
        return item && item->getId() == id;
    });
    if (it != items_.end()) {
        *it = std::move(newItem);
        return true;
    }
    return false;
}

std::vector<const Clothing*> WardrobeManager::getAllItems() const {
    std::vector<const Clothing*> result;
    result.reserve(items_.size());
    for (const auto& item : items_) {
        if (item) {
            result.push_back(item.get());
        }
    }
    return result;
}

std::vector<Clothing*> WardrobeManager::getAllItems() {
    std::vector<Clothing*> result;
    result.reserve(items_.size());
    for (auto& item : items_) {
        if (item) {
            result.push_back(item.get());
        }
    }
    return result;
}

const std::vector<std::unique_ptr<Clothing>>& WardrobeManager::getItems() const {
    return items_;
}

std::size_t WardrobeManager::size() const {
    return items_.size();
}

bool WardrobeManager::empty() const {
    return items_.empty();
}

void WardrobeManager::clear() {
    items_.clear();
}

std::vector<const Clothing*> WardrobeManager::filterByType(const std::string& type) const {
    FilterCriteria criteria;
    criteria.type = type;
    return filter(criteria);
}

std::vector<const Clothing*> WardrobeManager::filterBySubtype(const std::string& subtype) const {
    FilterCriteria criteria;
    criteria.subtype = subtype;
    return filter(criteria);
}

std::vector<const Clothing*> WardrobeManager::filterByColor(const std::string& color) const {
    FilterCriteria criteria;
    criteria.color = color;
    return filter(criteria);
}

std::vector<const Clothing*> WardrobeManager::filterByOccasion(const std::string& occasion) const {
    FilterCriteria criteria;
    criteria.occasion = occasion;
    return filter(criteria);
}

std::vector<const Clothing*> WardrobeManager::filterBySeason(const std::string& season) const {
    FilterCriteria criteria;
    criteria.season = season;
    return filter(criteria);
}

std::vector<const Clothing*> WardrobeManager::filter(const FilterCriteria& criteria) const {
    std::vector<const Clothing*> result;
    for (const auto& item : items_) {
        if (!item) {
            continue;
        }
        if (!criteria.type.empty() && item->getType() != criteria.type) {
            continue;
        }
        if (!criteria.subtype.empty() && item->getSubtype() != criteria.subtype) {
            continue;
        }
        if (!criteria.color.empty() && item->getColor() != criteria.color) {
            continue;
        }
        if (!criteria.occasion.empty() && !containsTag(item->getOccasions(), criteria.occasion)) {
            continue;
        }
        if (!criteria.season.empty() && !containsTag(item->getSeasons(), criteria.season)) {
            continue;
        }
        result.push_back(item.get());
    }
    return result;
}

} // namespace smartwardrobe

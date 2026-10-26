#pragma once

#include "Clothing.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace smartwardrobe {

/**
 * Multi-criteria filter options for querying the wardrobe.
 * Empty strings are ignored, matching any value for that field.
 */
struct FilterCriteria {
    std::string type;       // "Top", "Bottom", "Shoes"
    std::string subtype;    // e.g. "T-Shirt", "Jeans"
    std::string color;      // e.g. "White", "Blue"
    std::string occasion;   // matches if present in item's occasions list
    std::string season;     // matches if present in item's seasons list
};

/**
 * In-memory manager that owns Clothing items.
 *
 * Items are held via std::unique_ptr<Clothing> to enforce distinct ownership
 * and accommodate Clothing's non-copyable and non-movable semantics.
 */
class WardrobeManager {
public:
    WardrobeManager() = default;
    ~WardrobeManager() = default;

    // Non-copyable to preserve unique ownership of polymorphic items
    WardrobeManager(const WardrobeManager&) = delete;
    WardrobeManager& operator=(const WardrobeManager&) = delete;

    // Movable
    WardrobeManager(WardrobeManager&&) = default;
    WardrobeManager& operator=(WardrobeManager&&) = default;

    /**
     * Adds a clothing item to the wardrobe, taking ownership.
     *
     * Rejection conditions:
     * - `item` is nullptr (returns false).
     * - An item with the same ID already exists (returns false).
     *
     * @param item std::unique_ptr<Clothing> to take ownership of.
     * @return true if added successfully, false if null or duplicate ID.
     */
    bool addItem(std::unique_ptr<Clothing> item);

    /**
     * Finds an item by its unique ID.
     *
     * @param id The integer ID of the item.
     * @return Non-owning pointer to the Clothing item, or nullptr if not found.
     */
    Clothing* findById(int id);
    const Clothing* findById(int id) const;

    /**
     * Checks if an item with the given ID exists.
     */
    bool contains(int id) const;

    /**
     * Removes an item by ID from the wardrobe.
     *
     * @param id The integer ID of the item to remove.
     * @return true if found and removed, false if not found.
     */
    bool removeItem(int id);

    /**
     * Updates an existing clothing item by replacing it in-place.
     * Preserves the immutability of Clothing without adding setters.
     *
     * Rejection conditions:
     * - `newItem` is nullptr (returns false).
     * - No item with `newItem->getId()` exists in the wardrobe (returns false).
     *
     * @param newItem std::unique_ptr<Clothing> with matching ID to replace the existing one.
     * @return true if updated, false if null or ID not found.
     */
    bool updateItem(std::unique_ptr<Clothing> newItem);

    /**
     * Returns non-owning pointers to all items in the wardrobe.
     */
    std::vector<const Clothing*> getAllItems() const;
    std::vector<Clothing*> getAllItems();

    /**
     * Returns a direct const reference to the internal storage vector.
     */
    const std::vector<std::unique_ptr<Clothing>>& getItems() const;

    /**
     * Returns the total number of items currently in the wardrobe.
     */
    std::size_t size() const;

    /**
     * Returns true if the wardrobe contains zero items.
     */
    bool empty() const;

    /**
     * Removes and destroys all items in the wardrobe.
     */
    void clear();

    /**
     * Filter methods returning non-owning pointers to matching items.
     */
    std::vector<const Clothing*> filterByType(const std::string& type) const;
    std::vector<const Clothing*> filterBySubtype(const std::string& subtype) const;
    std::vector<const Clothing*> filterByColor(const std::string& color) const;
    std::vector<const Clothing*> filterByOccasion(const std::string& occasion) const;
    std::vector<const Clothing*> filterBySeason(const std::string& season) const;
    std::vector<const Clothing*> filter(const FilterCriteria& criteria) const;

private:
    std::vector<std::unique_ptr<Clothing>> items_;
};

} // namespace smartwardrobe

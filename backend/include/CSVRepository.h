#pragma once

#include "WardrobeManager.h"

#include <string>

namespace smartwardrobe {

/**
 * Repository class handling persistence of wardrobe data to/from CSV files.
 *
 * CSV Schema:
 * id,subtype,color,occasions,seasons
 *
 * Supported features:
 * - RFC 4180 compliant CSV parsing and serialization (quoted fields, commas, escaped quotes).
 * - Pipe-separated lists for multiple occasions and seasons (e.g. "Casual|Smart Casual").
 * - Transactional append semantics on load: all records are validated before modifying the manager.
 * - Header-based column resolution (supports 5-column default and 6-column type-inclusive formats).
 */
class CSVRepository {
public:
    CSVRepository() = default;
    explicit CSVRepository(std::string filePath);

    const std::string& getFilePath() const;
    void setFilePath(std::string filePath);

    /**
     * Loads clothing records from the repository's configured filePath into manager.
     */
    bool load(WardrobeManager& manager, std::string* error = nullptr) const;
    bool load(WardrobeManager& manager, std::string& error) const;

    /**
     * Saves all clothing items from manager into the repository's configured filePath.
     */
    bool save(const WardrobeManager& manager, std::string* error = nullptr) const;
    bool save(const WardrobeManager& manager, std::string& error) const;

    /**
     * Loads clothing records from a CSV file into the manager using transactional append semantics:
     * 1. Reads and parses the entire file into temporary objects.
     * 2. Validates every record (column counts, ID format, subtype validity, color presence).
     * 3. Verifies that all IDs in the file are distinct and do not conflict with items already in manager.
     * 4. Only if ALL records pass validation are the new items appended to manager.
     * 5. If parsing or validation fails, manager is left completely unchanged.
     *
     * @param filePath Path to the CSV file to load.
     * @param manager Destination WardrobeManager to receive new items.
     * @param error Optional pointer to receive error description on failure.
     * @return true if all items were successfully parsed, validated, and appended; false otherwise.
     */
    static bool load(const std::string& filePath, WardrobeManager& manager, std::string* error = nullptr);
    static bool load(const std::string& filePath, WardrobeManager& manager, std::string& error);

    /**
     * Serializes all clothing items from manager into a CSV file at filePath.
     *
     * Header format: id,subtype,color,occasions,seasons
     *
     * @param manager Source WardrobeManager containing items to persist.
     * @param filePath Path to the destination CSV file.
     * @param error Optional pointer to receive error description on failure.
     * @return true on success, false if file cannot be opened or written.
     */
    static bool save(const WardrobeManager& manager, const std::string& filePath, std::string* error = nullptr);
    static bool save(const WardrobeManager& manager, const std::string& filePath, std::string& error);

    /**
     * Escapes a CSV field according to RFC 4180.
     * If the field contains comma, double-quote, or newline, it is enclosed
     * in quotes and internal quotes are doubled.
     */
    static std::string escapeCSVField(const std::string& field);

    /**
     * Joins a tag list into a pipe-separated string (e.g. "Casual|Smart Casual").
     */
    static std::string joinTags(const Clothing::TagList& tags);

    /**
     * Splits a pipe-separated string into a tag list, preserving spaces within tags.
     */
    static Clothing::TagList splitTags(const std::string& field);

private:
    std::string filePath_;
};

} // namespace smartwardrobe

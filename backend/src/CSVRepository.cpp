#include "CSVRepository.h"
#include "ClothingFactory.h"

#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

namespace smartwardrobe {

namespace {

std::string trim(const std::string& str) {
    auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::string toLower(const std::string& str) {
    std::string lower = str;
    for (char& c : lower) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}

struct CSVParseResult {
    bool success;
    std::string error;
    std::vector<std::vector<std::string>> records;
    std::vector<int> recordLines;
};

CSVParseResult parseCSVStream(std::istream& in) {
    CSVParseResult result;
    result.success = true;

    std::vector<std::string> currentRecord;
    std::string currentField;
    int currentLine = 1;
    int recordStartLine = 1;
    bool inQuotes = false;
    bool fieldIsEmpty = true;
    bool hasAnyData = false;

    char c;
    while (in.get(c)) {
        hasAnyData = true;

        if (inQuotes) {
            if (c == '"') {
                if (in.peek() == '"') {
                    // Escaped double-quote
                    char nextQuote;
                    in.get(nextQuote);
                    currentField += '"';
                    fieldIsEmpty = false;
                } else {
                    // End of quoted field
                    inQuotes = false;
                    // Check for invalid trailing characters before delimiter or line-break
                    int peekChar = in.peek();
                    if (peekChar != ',' && peekChar != '\r' && peekChar != '\n' && peekChar != EOF) {
                        result.success = false;
                        result.error = "Line " + std::to_string(currentLine) +
                                       ": Unexpected character '" + static_cast<char>(peekChar) +
                                       "' after closing quote";
                        return result;
                    }
                }
            } else {
                if (c == '\n') {
                    ++currentLine;
                }
                currentField += c;
                fieldIsEmpty = false;
            }
        } else {
            if (c == '"') {
                if (currentField.empty() && fieldIsEmpty) {
                    inQuotes = true;
                    fieldIsEmpty = false;
                } else {
                    result.success = false;
                    result.error = "Line " + std::to_string(currentLine) +
                                   ": Unexpected quote character inside unquoted field";
                    return result;
                }
            } else if (c == ',') {
                currentRecord.push_back(currentField);
                currentField.clear();
                fieldIsEmpty = true;
            } else if (c == '\r') {
                if (in.peek() == '\n') {
                    char nl;
                    in.get(nl);
                }
                currentRecord.push_back(currentField);
                currentField.clear();
                fieldIsEmpty = true;

                if (currentRecord.size() > 1 || !currentRecord[0].empty()) {
                    result.records.push_back(std::move(currentRecord));
                    result.recordLines.push_back(recordStartLine);
                }
                currentRecord.clear();
                ++currentLine;
                recordStartLine = currentLine;
            } else if (c == '\n') {
                currentRecord.push_back(currentField);
                currentField.clear();
                fieldIsEmpty = true;

                if (currentRecord.size() > 1 || !currentRecord[0].empty()) {
                    result.records.push_back(std::move(currentRecord));
                    result.recordLines.push_back(recordStartLine);
                }
                currentRecord.clear();
                ++currentLine;
                recordStartLine = currentLine;
            } else {
                currentField += c;
                fieldIsEmpty = false;
            }
        }
    }

    if (inQuotes) {
        result.success = false;
        result.error = "Line " + std::to_string(currentLine) + ": Unclosed quoted field at end of file";
        return result;
    }

    if (!fieldIsEmpty || !currentField.empty() || !currentRecord.empty()) {
        currentRecord.push_back(currentField);
        if (currentRecord.size() > 1 || !currentRecord[0].empty()) {
            result.records.push_back(std::move(currentRecord));
            result.recordLines.push_back(recordStartLine);
        }
    }

    if (!hasAnyData) {
        result.success = false;
        result.error = "CSV file is empty: header row missing";
        return result;
    }

    return result;
}

} // namespace

CSVRepository::CSVRepository(std::string filePath)
    : filePath_(std::move(filePath)) {}

const std::string& CSVRepository::getFilePath() const {
    return filePath_;
}

void CSVRepository::setFilePath(std::string filePath) {
    filePath_ = std::move(filePath);
}

bool CSVRepository::load(WardrobeManager& manager, std::string* error) const {
    return load(filePath_, manager, error);
}

bool CSVRepository::load(WardrobeManager& manager, std::string& error) const {
    return load(filePath_, manager, &error);
}

bool CSVRepository::save(const WardrobeManager& manager, std::string* error) const {
    return save(manager, filePath_, error);
}

bool CSVRepository::save(const WardrobeManager& manager, std::string& error) const {
    return save(manager, filePath_, &error);
}

bool CSVRepository::load(const std::string& filePath, WardrobeManager& manager, std::string& error) {
    return load(filePath, manager, &error);
}

bool CSVRepository::save(const WardrobeManager& manager, const std::string& filePath, std::string& error) {
    return save(manager, filePath, &error);
}

std::string CSVRepository::escapeCSVField(const std::string& field) {
    bool needsQuotes = false;
    for (char c : field) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            needsQuotes = true;
            break;
        }
    }
    if (!needsQuotes) {
        return field;
    }

    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    return escaped;
}

std::string CSVRepository::joinTags(const Clothing::TagList& tags) {
    std::string result;
    for (std::size_t i = 0; i < tags.size(); ++i) {
        if (i > 0) {
            result += "|";
        }
        result += tags[i];
    }
    return result;
}

Clothing::TagList CSVRepository::splitTags(const std::string& field) {
    Clothing::TagList tags;
    std::string current;
    for (char c : field) {
        if (c == '|') {
            if (!current.empty()) {
                tags.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        tags.push_back(current);
    }
    return tags;
}

bool CSVRepository::save(const WardrobeManager& manager, const std::string& filePath, std::string* error) {
    std::ofstream out(filePath.c_str());
    if (!out.is_open()) {
        if (error) {
            *error = "Cannot open file for writing: " + filePath;
        }
        return false;
    }

    out << "id,subtype,color,occasions,seasons\n";

    for (const auto& item : manager.getItems()) {
        if (!item) {
            continue;
        }
        out << item->getId() << ","
            << escapeCSVField(item->getSubtype()) << ","
            << escapeCSVField(item->getColor()) << ","
            << escapeCSVField(joinTags(item->getOccasions())) << ","
            << escapeCSVField(joinTags(item->getSeasons())) << "\n";
    }

    if (!out.good()) {
        if (error) {
            *error = "Failed writing CSV data to: " + filePath;
        }
        return false;
    }

    out.close();
    if (out.fail()) {
        if (error) {
            *error = "Failed closing file after writing: " + filePath;
        }
        return false;
    }

    return true;
}

bool CSVRepository::load(const std::string& filePath, WardrobeManager& manager, std::string* error) {
    std::ifstream in(filePath.c_str());
    if (!in.is_open()) {
        if (error) {
            *error = "Cannot open file for reading: " + filePath;
        }
        return false;
    }

    CSVParseResult parsed = parseCSVStream(in);
    if (!parsed.success) {
        if (error) {
            *error = parsed.error;
        }
        return false;
    }

    if (parsed.records.empty()) {
        if (error) {
            *error = "CSV file is empty: header row missing";
        }
        return false;
    }

    const auto& header = parsed.records[0];
    int idIdx = -1;
    int typeIdx = -1;
    int subtypeIdx = -1;
    int colorIdx = -1;
    int occasionsIdx = -1;
    int seasonsIdx = -1;

    for (std::size_t i = 0; i < header.size(); ++i) {
        std::string col = toLower(trim(header[i]));
        if (col == "id") {
            idIdx = static_cast<int>(i);
        } else if (col == "type") {
            typeIdx = static_cast<int>(i);
        } else if (col == "subtype") {
            subtypeIdx = static_cast<int>(i);
        } else if (col == "color" || col == "colour") {
            colorIdx = static_cast<int>(i);
        } else if (col == "occasions" || col == "occasion") {
            occasionsIdx = static_cast<int>(i);
        } else if (col == "seasons" || col == "season") {
            seasonsIdx = static_cast<int>(i);
        }
    }

    auto checkMissingHeader = [&](int idx, const std::string& colName) {
        if (idx == -1) {
            if (error) {
                *error = "Invalid CSV header: missing required column '" + colName + "'";
            }
            return false;
        }
        return true;
    };

    if (!checkMissingHeader(idIdx, "id") ||
        !checkMissingHeader(subtypeIdx, "subtype") ||
        !checkMissingHeader(colorIdx, "color") ||
        !checkMissingHeader(occasionsIdx, "occasions") ||
        !checkMissingHeader(seasonsIdx, "seasons")) {
        return false;
    }

    std::vector<std::unique_ptr<Clothing>> staging;
    staging.reserve(parsed.records.size() > 1 ? parsed.records.size() - 1 : 0);
    std::set<int> seenFileIds;

    for (std::size_t i = 1; i < parsed.records.size(); ++i) {
        const auto& row = parsed.records[i];
        int lineNum = parsed.recordLines[i];

        if (row.size() != header.size()) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": Expected " +
                         std::to_string(header.size()) + " columns but found " +
                         std::to_string(row.size());
            }
            return false;
        }

        std::string idStr = trim(row[idIdx]);
        if (idStr.empty()) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": ID field cannot be empty";
            }
            return false;
        }

        int id = 0;
        try {
            std::size_t pos = 0;
            id = std::stoi(idStr, &pos);
            if (pos != idStr.size()) {
                if (error) {
                    *error = "Line " + std::to_string(lineNum) + ": Invalid integer ID '" + idStr + "'";
                }
                return false;
            }
        } catch (...) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": Invalid integer ID '" + idStr + "'";
            }
            return false;
        }

        if (id <= 0) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": ID must be a positive integer, got " + std::to_string(id);
            }
            return false;
        }

        if (!seenFileIds.insert(id).second) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": Duplicate ID " + std::to_string(id) + " within CSV file";
            }
            return false;
        }

        if (manager.contains(id)) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": ID " + std::to_string(id) + " already exists in wardrobe manager";
            }
            return false;
        }

        std::string subtype = trim(row[subtypeIdx]);
        if (!ClothingFactory::isSupportedSubtype(subtype)) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": Unsupported clothing subtype '" + subtype + "'";
            }
            return false;
        }

        if (typeIdx != -1) {
            std::string type = trim(row[typeIdx]);
            if (!type.empty()) {
                std::string expectedType = ClothingFactory::getTypeForSubtype(subtype);
                if (type != expectedType) {
                    if (error) {
                        *error = "Line " + std::to_string(lineNum) + ": Subtype '" + subtype +
                                 "' belongs to category '" + expectedType + "', not '" + type + "'";
                    }
                    return false;
                }
            }
        }

        std::string color = trim(row[colorIdx]);
        if (color.empty()) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": Color field cannot be empty";
            }
            return false;
        }

        Clothing::TagList occasions = splitTags(row[occasionsIdx]);
        Clothing::TagList seasons = splitTags(row[seasonsIdx]);

        auto item = ClothingFactory::create(subtype, id, color, std::move(occasions), std::move(seasons));
        if (!item) {
            if (error) {
                *error = "Line " + std::to_string(lineNum) + ": Failed to instantiate item for subtype '" + subtype + "'";
            }
            return false;
        }

        staging.push_back(std::move(item));
    }

    // All records validated successfully: transactional append
    for (auto& item : staging) {
        manager.addItem(std::move(item));
    }

    return true;
}

} // namespace smartwardrobe

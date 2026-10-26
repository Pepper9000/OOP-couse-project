#include "CSVRepository.h"
#include "ClothingFactory.h"
#include "WardrobeManager.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace smartwardrobe;

static int testCount = 0;
static int passCount = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        ++testCount; \
        if (!(cond)) { \
            std::cerr << "FAIL [line " << __LINE__ << "]: " << msg << std::endl; \
            assert(false); \
        } else { \
            ++passCount; \
        } \
    } while (0)

static void writeRawFile(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str(), std::ios::binary);
    out << content;
}

static void removeFile(const std::string& path) {
    std::remove(path.c_str());
}

void testTagSerializationAndEscaping() {
    std::cout << "Running testTagSerializationAndEscaping..." << std::endl;

    // joinTags
    TEST_ASSERT(CSVRepository::joinTags({}) == "", "Empty tag list joins to empty string");
    TEST_ASSERT(CSVRepository::joinTags({"Casual"}) == "Casual", "Single tag joins correctly");
    TEST_ASSERT(CSVRepository::joinTags({"Casual", "Smart Casual", "Party"}) == "Casual|Smart Casual|Party",
                "Multiple tags join with pipe delimiter");

    // splitTags
    auto emptyTags = CSVRepository::splitTags("");
    TEST_ASSERT(emptyTags.empty(), "Empty string splits to empty vector");

    auto singleTag = CSVRepository::splitTags("Casual");
    TEST_ASSERT(singleTag.size() == 1 && singleTag[0] == "Casual", "Single tag splits correctly");

    auto multiTags = CSVRepository::splitTags("Casual|Smart Casual|All Season");
    TEST_ASSERT(multiTags.size() == 3, "Multi tags split to 3 items");
    TEST_ASSERT(multiTags[0] == "Casual", "First tag matches");
    TEST_ASSERT(multiTags[1] == "Smart Casual", "Second tag preserves internal space");
    TEST_ASSERT(multiTags[2] == "All Season", "Third tag preserves internal space");

    // Empty segments ignored
    auto withEmptySeg = CSVRepository::splitTags("Casual||Party");
    TEST_ASSERT(withEmptySeg.size() == 2 && withEmptySeg[0] == "Casual" && withEmptySeg[1] == "Party",
                "Empty segments in pipe string are skipped");

    // escapeCSVField
    TEST_ASSERT(CSVRepository::escapeCSVField("plain") == "plain", "Plain text not quoted");
    TEST_ASSERT(CSVRepository::escapeCSVField("hello, world") == "\"hello, world\"", "Commas cause quoting");
    TEST_ASSERT(CSVRepository::escapeCSVField("Dark \"Navy\"") == "\"Dark \"\"Navy\"\"\"", "Quotes are escaped as doubled quotes");
    TEST_ASSERT(CSVRepository::escapeCSVField("line1\nline2") == "\"line1\nline2\"", "Newlines cause quoting");
}

void testSaveAndLoadRoundtrip() {
    std::cout << "Running testSaveAndLoadRoundtrip..." << std::endl;

    const std::string testFile = "temp_roundtrip.csv";
    removeFile(testFile);

    WardrobeManager saveManager;
    saveManager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    saveManager.addItem(ClothingFactory::create("Jeans", 2, "Blue", {"Casual", "Smart Casual"}, {"All Season"}));
    saveManager.addItem(ClothingFactory::create("Sneakers", 3, "White", {"Active", "Casual"}, {"Summer", "Winter"}));

    std::string saveError;
    bool saved = CSVRepository::save(saveManager, testFile, saveError);
    TEST_ASSERT(saved, "Save should succeed: " + saveError);

    WardrobeManager loadManager;
    std::string loadError;
    bool loaded = CSVRepository::load(testFile, loadManager, loadError);
    TEST_ASSERT(loaded, "Load should succeed: " + loadError);
    TEST_ASSERT(loadManager.size() == 3, "Loaded manager should contain 3 items");

    // Verify item 1
    const Clothing* item1 = loadManager.findById(1);
    TEST_ASSERT(item1 != nullptr, "Item 1 must exist");
    if (item1) {
        TEST_ASSERT(item1->getType() == "Top", "Item 1 type must be Top");
        TEST_ASSERT(item1->getSubtype() == "T-Shirt", "Item 1 subtype must be T-Shirt");
        TEST_ASSERT(item1->getColor() == "White", "Item 1 color must be White");
        TEST_ASSERT(item1->getOccasions().size() == 1 && item1->getOccasions()[0] == "Casual", "Item 1 occasion matches");
        TEST_ASSERT(item1->getSeasons().size() == 1 && item1->getSeasons()[0] == "Summer", "Item 1 season matches");
    }

    // Verify item 2
    const Clothing* item2 = loadManager.findById(2);
    TEST_ASSERT(item2 != nullptr, "Item 2 must exist");
    if (item2) {
        TEST_ASSERT(item2->getType() == "Bottom", "Item 2 type must be Bottom");
        TEST_ASSERT(item2->getSubtype() == "Jeans", "Item 2 subtype must be Jeans");
        TEST_ASSERT(item2->getColor() == "Blue", "Item 2 color must be Blue");
        TEST_ASSERT(item2->getOccasions().size() == 2, "Item 2 has 2 occasions");
        TEST_ASSERT(item2->getOccasions()[0] == "Casual" && item2->getOccasions()[1] == "Smart Casual", "Item 2 occasions match");
    }

    // Verify item 3
    const Clothing* item3 = loadManager.findById(3);
    TEST_ASSERT(item3 != nullptr, "Item 3 must exist");
    if (item3) {
        TEST_ASSERT(item3->getType() == "Shoes", "Item 3 type must be Shoes");
        TEST_ASSERT(item3->getSubtype() == "Sneakers", "Item 3 subtype must be Sneakers");
        TEST_ASSERT(item3->getSeasons().size() == 2, "Item 3 has 2 seasons");
    }

    removeFile(testFile);
}

void testValuesContainingCommasAndQuotes() {
    std::cout << "Running testValuesContainingCommasAndQuotes..." << std::endl;

    const std::string testFile = "temp_special_chars.csv";
    removeFile(testFile);

    WardrobeManager saveManager;
    saveManager.addItem(ClothingFactory::create("Shirt", 1, "Navy, Dark", {"Formal"}, {"All Season"}));
    saveManager.addItem(ClothingFactory::create("Boots", 2, "Dark \"Vintage\" Brown", {"Casual"}, {"Winter"}));

    std::string err;
    TEST_ASSERT(CSVRepository::save(saveManager, testFile, err), "Save special chars should succeed");

    WardrobeManager loadManager;
    TEST_ASSERT(CSVRepository::load(testFile, loadManager, err), "Load special chars should succeed: " + err);

    const Clothing* item1 = loadManager.findById(1);
    TEST_ASSERT(item1 != nullptr && item1->getColor() == "Navy, Dark", "Color with comma preserved");

    const Clothing* item2 = loadManager.findById(2);
    TEST_ASSERT(item2 != nullptr && item2->getColor() == "Dark \"Vintage\" Brown", "Color with quotes preserved");

    removeFile(testFile);
}

void testEmptyWardrobeAndHeaderOnlyCSV() {
    std::cout << "Running testEmptyWardrobeAndHeaderOnlyCSV..." << std::endl;

    const std::string testFile = "temp_empty_wardrobe.csv";
    removeFile(testFile);

    // Save empty wardrobe
    WardrobeManager emptyManager;
    std::string err;
    TEST_ASSERT(CSVRepository::save(emptyManager, testFile, err), "Saving empty wardrobe should succeed");

    // Load header-only file
    WardrobeManager loadManager;
    TEST_ASSERT(CSVRepository::load(testFile, loadManager, err), "Loading header-only file should succeed");
    TEST_ASSERT(loadManager.empty(), "Manager should remain empty after loading header-only CSV");

    removeFile(testFile);

    // 0-byte file
    const std::string zeroByteFile = "temp_zero_byte.csv";
    writeRawFile(zeroByteFile, "");
    WardrobeManager zeroManager;
    bool loadedZero = CSVRepository::load(zeroByteFile, zeroManager, err);
    TEST_ASSERT(!loadedZero, "Loading 0-byte file should fail");
    TEST_ASSERT(!err.empty(), "Error message should be set for empty file");
    TEST_ASSERT(zeroManager.empty(), "Manager should remain empty");

    removeFile(zeroByteFile);
}

void testDuplicateIDsInCSV() {
    std::cout << "Running testDuplicateIDsInCSV..." << std::endl;

    const std::string testFile = "temp_dup_ids.csv";
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "1,T-Shirt,White,Casual,Summer\n"
                 "1,Jeans,Blue,Casual,All Season\n");

    WardrobeManager manager;
    std::string err;
    bool loaded = CSVRepository::load(testFile, manager, err);
    TEST_ASSERT(!loaded, "Duplicate ID within CSV should be rejected");
    TEST_ASSERT(err.find("Duplicate ID 1") != std::string::npos, "Error should report duplicate ID 1: " + err);
    TEST_ASSERT(manager.empty(), "Manager should remain empty after failure");

    removeFile(testFile);
}

void testDuplicateIDsAgainstExistingManager() {
    std::cout << "Running testDuplicateIDsAgainstExistingManager..." << std::endl;

    const std::string testFile = "temp_dup_existing.csv";
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "5,Sneakers,White,Casual,Summer\n");

    WardrobeManager manager;
    manager.addItem(ClothingFactory::create("T-Shirt", 5, "Black", {"Casual"}, {"Summer"}));
    TEST_ASSERT(manager.size() == 1, "Manager starts with 1 item");

    std::string err;
    bool loaded = CSVRepository::load(testFile, manager, err);
    TEST_ASSERT(!loaded, "Loading duplicate ID conflicting with manager should fail");
    TEST_ASSERT(err.find("already exists") != std::string::npos, "Error reports existing conflict: " + err);
    TEST_ASSERT(manager.size() == 1, "Manager size must remain 1");
    TEST_ASSERT(manager.findById(5)->getSubtype() == "T-Shirt", "Original item remains intact");

    removeFile(testFile);
}

void testUnknownClothingSubtype() {
    std::cout << "Running testUnknownClothingSubtype..." << std::endl;

    const std::string testFile = "temp_unknown_subtype.csv";
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "1,Jetpack,Chrome,Active,All Season\n");

    WardrobeManager manager;
    std::string err;
    bool loaded = CSVRepository::load(testFile, manager, err);
    TEST_ASSERT(!loaded, "Unknown subtype 'Jetpack' must be rejected");
    TEST_ASSERT(err.find("Unsupported clothing subtype 'Jetpack'") != std::string::npos,
                "Error reports unknown subtype: " + err);
    TEST_ASSERT(manager.empty(), "Manager remains empty");

    removeFile(testFile);
}

void testInvalidIDsAndColumnCounts() {
    std::cout << "Running testInvalidIDsAndColumnCounts..." << std::endl;

    const std::string testFile = "temp_invalid_fields.csv";

    // Non-integer ID
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "abc,T-Shirt,White,Casual,Summer\n");
    WardrobeManager m1;
    std::string err;
    TEST_ASSERT(!CSVRepository::load(testFile, m1, err), "Non-integer ID should fail");
    TEST_ASSERT(err.find("Invalid integer ID") != std::string::npos, "Error reports invalid ID");

    // Negative ID
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "-5,T-Shirt,White,Casual,Summer\n");
    WardrobeManager m2;
    TEST_ASSERT(!CSVRepository::load(testFile, m2, err), "Negative ID should fail");
    TEST_ASSERT(err.find("positive integer") != std::string::npos, "Error reports positive integer requirement");

    // Too few columns
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "1,T-Shirt,White\n");
    WardrobeManager m3;
    TEST_ASSERT(!CSVRepository::load(testFile, m3, err), "Incorrect column count should fail");
    TEST_ASSERT(err.find("columns but found") != std::string::npos, "Error reports column count mismatch");

    // Empty color
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "1,T-Shirt,,Casual,Summer\n");
    WardrobeManager m4;
    TEST_ASSERT(!CSVRepository::load(testFile, m4, err), "Empty color should fail");
    TEST_ASSERT(err.find("Color field cannot be empty") != std::string::npos, "Error reports empty color");

    removeFile(testFile);
}

void testMalformedQuotedFields() {
    std::cout << "Running testMalformedQuotedFields..." << std::endl;

    const std::string testFile = "temp_malformed_quotes.csv";

    // Unclosed quote
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "1,T-Shirt,\"White,Casual,Summer\n");
    WardrobeManager m1;
    std::string err;
    TEST_ASSERT(!CSVRepository::load(testFile, m1, err), "Unclosed quote should fail");
    TEST_ASSERT(err.find("Unclosed quoted field") != std::string::npos, "Error reports unclosed quote");

    // Stray characters after closing quote
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "1,T-Shirt,\"White\"bad,Casual,Summer\n");
    WardrobeManager m2;
    TEST_ASSERT(!CSVRepository::load(testFile, m2, err), "Stray chars after quote should fail");
    TEST_ASSERT(err.find("Unexpected character") != std::string::npos, "Error reports unexpected char: " + err);

    removeFile(testFile);
}

void testMissingOrUnreadableFiles() {
    std::cout << "Running testMissingOrUnreadableFiles..." << std::endl;

    WardrobeManager manager;
    std::string err;
    bool loaded = CSVRepository::load("this_file_definitely_does_not_exist_987654.csv", manager, err);
    TEST_ASSERT(!loaded, "Loading missing file should fail");
    TEST_ASSERT(err.find("Cannot open file for reading") != std::string::npos, "Error reports cannot open file");
}

void testFailedSaveOperations() {
    std::cout << "Running testFailedSaveOperations..." << std::endl;

    WardrobeManager manager;
    manager.addItem(ClothingFactory::create("T-Shirt", 1, "White", {"Casual"}, {"Summer"}));
    std::string err;
    // Attempt save to an invalid Windows path with illegal characters
    bool saved = CSVRepository::save(manager, "Z:/non_existent_folder_xyz_123/file.csv", err);
    TEST_ASSERT(!saved, "Saving to invalid drive/path should fail");
    TEST_ASSERT(err.find("Cannot open file for writing") != std::string::npos, "Error reports cannot open write");
}

void testTransactionalAppend() {
    std::cout << "Running testTransactionalAppend..." << std::endl;

    const std::string testFile = "temp_transactional.csv";
    // File has item 100 (valid), item 101 (valid), item 102 (INVALID subtype)
    writeRawFile(testFile,
                 "id,subtype,color,occasions,seasons\n"
                 "100,T-Shirt,White,Casual,Summer\n"
                 "101,Jeans,Blue,Casual,All Season\n"
                 "102,Spacesuit,Silver,Party,Winter\n");

    WardrobeManager manager;
    manager.addItem(ClothingFactory::create("Shirt", 1, "Red", {"Formal"}, {"All Season"}));
    manager.addItem(ClothingFactory::create("Sweater", 2, "Green", {"Casual"}, {"Winter"}));
    TEST_ASSERT(manager.size() == 2, "Initial size is 2");

    std::string err;
    bool loaded = CSVRepository::load(testFile, manager, err);
    TEST_ASSERT(!loaded, "Loading file with an invalid record should fail");

    // Manager must NOT have item 100 or 101 added!
    TEST_ASSERT(manager.size() == 2, "Manager size must remain exactly 2 after failed load");
    TEST_ASSERT(manager.contains(1) && manager.contains(2), "Initial items 1 and 2 must remain");
    TEST_ASSERT(!manager.contains(100), "Item 100 must NOT be present in manager");
    TEST_ASSERT(!manager.contains(101), "Item 101 must NOT be present in manager");

    removeFile(testFile);
}

void test6ColumnHeaderWithType() {
    std::cout << "Running test6ColumnHeaderWithType..." << std::endl;

    const std::string testFile = "temp_6col.csv";
    // Header includes 'type' column as documented in README
    writeRawFile(testFile,
                 "id,type,subtype,color,occasion,season\n"
                 "1,Top,T-Shirt,White,Casual,All Season\n"
                 "2,Bottom,Jeans,Blue,Casual|Smart Casual,All Season\n"
                 "3,Shoes,Sneakers,White,Casual|Smart Casual,All Season\n");

    WardrobeManager manager;
    std::string err;
    bool loaded = CSVRepository::load(testFile, manager, err);
    TEST_ASSERT(loaded, "Loading 6-column CSV should succeed: " + err);
    TEST_ASSERT(manager.size() == 3, "Manager should contain 3 items");

    // Mismatched type and subtype
    const std::string mismatchFile = "temp_mismatch.csv";
    writeRawFile(mismatchFile,
                 "id,type,subtype,color,occasion,season\n"
                 "10,Shoes,T-Shirt,White,Casual,All Season\n");

    WardrobeManager m2;
    bool mismatchLoaded = CSVRepository::load(mismatchFile, m2, err);
    TEST_ASSERT(!mismatchLoaded, "Type-subtype mismatch must fail");
    TEST_ASSERT(err.find("belongs to category 'Top', not 'Shoes'") != std::string::npos,
                "Error reports category mismatch: " + err);

    removeFile(testFile);
    removeFile(mismatchFile);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Smart Wardrobe - Phase 2 CSV Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    testTagSerializationAndEscaping();
    testSaveAndLoadRoundtrip();
    testValuesContainingCommasAndQuotes();
    testEmptyWardrobeAndHeaderOnlyCSV();
    testDuplicateIDsInCSV();
    testDuplicateIDsAgainstExistingManager();
    testUnknownClothingSubtype();
    testInvalidIDsAndColumnCounts();
    testMalformedQuotedFields();
    testMissingOrUnreadableFiles();
    testFailedSaveOperations();
    testTransactionalAppend();
    test6ColumnHeaderWithType();

    std::cout << "========================================" << std::endl;
    std::cout << "Summary: " << passCount << " / " << testCount << " assertions passed." << std::endl;
    std::cout << "ALL CSV TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

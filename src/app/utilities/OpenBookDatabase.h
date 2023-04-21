#ifndef OpenBookDatabase_h
#define OpenBookDatabase_h

#include <stdint.h>
#include <string>
#include <vector>
#include "OpenBookDevice.h"

static const std::string VERSION_FILE = "_VERSION";
static const std::string CHAPTERS_FILE = "/_CHAPTERS";
static const std::string PAGES_FILE = "/_PAGES";
static const std::string BOOK_FILE = "/_BOOK";
static const std::string CURRENT_PAGE_FILE = "/_CURRENT";
static const std::string BOOKS_DIR = "/BOOKS";
#define LIBRARY_DIR    ("/_LIBRARY/")
#define BACKUP_DIR     ("/_LIBBACK/")
#define WORKING_DIR    ("/_LIBTEMP/")

#define DATABASE_VERSION  (0x0005)

static const uint16_t LIBRARY_PAGE_SIZE = 12;

static const uint64_t VERSION_FILE_ID = 6825903261955698688;
static const uint32_t TXT_EXTENSION = 1954051118;

// Header
static const uint16_t HEADER_TAG_COUNT  = 5;
static const uint32_t HEADER_DELIMITER  = 170732845;
static const uint32_t TAG_TITLE         = 1280592212;
static const uint32_t TAG_AUTHOR        = 1213486401;
static const uint32_t TAG_GENRE         = 1163021895;
static const uint32_t TAG_DESCRIPTION   = 1129530692;
static const uint32_t TAG_LANGUAGE      = 1196310860;
static const uint16_t INDEX_TITLE       = 0;
static const uint16_t INDEX_AUTHOR      = 1;
static const uint16_t INDEX_GENRE       = 2;
static const uint16_t INDEX_DESCRIPTION = 3;
static const uint16_t INDEX_LANGUAGE    = 4;

// Special characters
static const char CHAPTER_MARK = 0x1e;
static const char NEW_LINE     = 0x0a;
static const char SPACE        = 0x20;

// Page consts
static const uint16_t PAGE_HEIGHT = 384;
static const uint16_t PAGE_WIDTH  = 288;

// Structs for the _LIBRARY database

typedef struct {
    uint32_t tag = 0;
    uint16_t loc = 0;
    uint16_t len = 0;
} BookField;

typedef struct {
    char filename[128];
    char fileHash[64];
    uint64_t fileSize = 0;
    uint64_t textStart = 0;
    uint32_t numChapters = 0; // Number of chapter descriptors
    uint32_t numPages = 0;    // Number of page descriptors
    BookField metadata[HEADER_TAG_COUNT];
} BookRecord;

typedef struct {
    uint64_t magic = VERSION_FILE_ID;
    uint32_t version = DATABASE_VERSION;
    char libraryHash[64] = {0};
} BookDatabaseVersion;

class OpenBookDatabase {
public:
    static OpenBookDatabase *sharedDatabase() {
        static OpenBookDatabase instance;
        return &instance;
    }
    OpenBookDatabase(OpenBookDatabase const&) = delete;
    void operator=(OpenBookDatabase const&) = delete;

    // Methods for working with the main _LIBRARY file
    bool connect();
    bool scanForNewBooks();
    std::vector<BookRecord> getLibrary();
    std::vector<BookRecord> getLibraryPage(uint16_t page);
    uint16_t getLibrarySize();
    BookRecord getLibraryBookRecord(uint16_t libraryIndex);
    void setLibraryBookRecord(uint16_t libraryIndex, BookRecord bookRecord);
    std::string getBookTitle(BookRecord record);
    std::string getBookAuthor(BookRecord record);
    std::string getBookDescription(BookRecord record);

    // These should work within _LIBRARY or the main database,
    // but use a sidecar file for now
    uint32_t getCurrentPage(BookRecord record);
    void setCurrentPage(BookRecord record, uint32_t page);

    // Methods for dealing with .pag sidecar files
    bool bookIsPaginated(BookRecord record);
    BookRecord paginateBook(BookRecord record);
    std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> _generatePages(BookRecord bookRecord);

    std::string getTextForPage(BookRecord record, uint32_t page);
protected:
    File _findOrCreateLibraryFile();
    void _createLibraryVersionFile(std::string versionFilename);
    void _setLibraryHash(char* libraryHash);
    bool _copyTxtFilesToBookDirectory();
    void _processNewTxtFiles();
    void _setLibrary();
    void _writeNewBookRecordFiles();
    bool _fileIsTxt(File entry);
    void _setFileHash(char* fileHash, File entry);
    bool _hasHeader(File entry);
    BookRecord _getBookRecord(char* fileHash);
    BookRecord _processBookFile(File entry, char* fileHash);
    std::string _getMetadataAtIndex(BookRecord record, uint16_t i);
    void _getPaginationFilename(BookRecord record, char* paginationFilename);
    void _getChaptersFilename(BookRecord record, char* chaptersFilename);
    void _getCurrentPageFilename(BookRecord record, char* currentPageFilename);

    BookDatabaseVersion version;
    std::vector <BookRecord>Records;
private:
    OpenBookDatabase();
};

#endif // OpenBookDatabase_h

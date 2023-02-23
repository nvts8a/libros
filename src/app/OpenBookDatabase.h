#ifndef OpenBookDatabase_h
#define OpenBookDatabase_h

#include <stdint.h>
#include <string>
#include "OpenBookDevice.h"

#define LIBRARY_FILENAME  ("_LIBRARY")
#define BACKUP_FILENAME   ("_LIBBACK")
#define WORKING_FILENAME  ("_LIBTEMP")

#define DATABASE_VERSION  (0x0000)

static const uint64_t DATABASE_FILE_IDENTIFIER = 6825903261955698688;

// Header
static const int HEADER_TAG_COUNT  = 5;
static const int TAG_TITLE         = 1280592212;
static const int TAG_AUTHOR        = 1213486401;
static const int TAG_GENRE         = 1163021895;
static const int TAG_DESCRIPTION   = 1129530692;
static const int TAG_LANGUAGE      = 1196310860;
static const int INDEX_TITLE       = 0;
static const int INDEX_AUTHOR      = 1;
static const int INDEX_GENRE       = 2;
static const int INDEX_DESCRIPTION = 3;
static const int INDEX_LANGUAGE    = 4;

// Special characters
static const char  CHAPTER_MARK = 0x1e;
static const char  SPACE        = 0x20;

// Structs for the _LIBRARY database

typedef struct {
    uint32_t tag = 0;
    uint16_t loc = 0;
    uint16_t len = 0;
} BookField;

typedef struct {
    char filename[128];
    uint64_t fileHash = 0;
    uint64_t fileSize = 0;
    uint64_t textStart = 0;
    uint64_t currentPosition = 0;
    uint64_t flags = 0;
    BookField metadata[HEADER_TAG_COUNT];
} BookRecord;

typedef struct {
    uint64_t flags = 0;
    uint32_t version = DATABASE_VERSION;
    uint16_t reserved1 = 0;
    uint32_t numBooks = 0;
    uint32_t reserved2 = 0;
    uint64_t reserved3 = 0;
} BookDatabaseHeader;

// structs for the .pag pagination files

typedef struct {
    uint64_t magic = 4992030523817504768;   // for identifying the file
    uint32_t numChapters = 0;               // Number of chapter descriptors
    uint32_t numPages = 0;                  // Number of page descriptors
    uint32_t tocStart = 0;                  // Start of chapter descriptors
    uint32_t pageStart = 0;                 // Start of page descriptors
} BookPaginationHeader;

typedef struct {
    uint32_t loc = 0;       // Location in the text file of the RS indicating chapter separation
    uint16_t len = 0;       // Length of the chapter header, including RS character
    uint16_t reserved = 0;  // Reserved for future use
} BookChapter;

typedef struct {
    uint32_t loc = 0;                       // Location in the text file of the page
    uint16_t len = 0;                       // Length of the page in characters
    struct {
        uint16_t isChapterSeparator : 1;    // 1 if this is a chapter separator page
        uint16_t activeShifts : 2;          // 0-3 for number of format shifts
        uint16_t reserved : 13;             // Reserved for future use
    } flags = {0};
} BookPage;

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
    uint32_t getNumberOfBooks();
    BookRecord getBookRecord(uint32_t i);
    std::string getBookTitle(BookRecord record);
    std::string getBookAuthor(BookRecord record);
    std::string getBookDescription(BookRecord record);

    // These should work within _LIBRARY or the main database,
    // but use a sidecar file for now
    uint32_t getCurrentPage(BookRecord record);
    void setCurrentPage(BookRecord record, uint32_t page);

    // Methods for dealing with .pag sidecar files
    bool bookIsPaginated(BookRecord record);
    void paginateBook(BookRecord record);
    uint32_t numPages(BookRecord record);

    std::string getTextForPage(BookRecord record, uint32_t page);
protected:
    File _findOrCreateLibraryFile(OpenBookDevice* device);
    bool _fileIsTxt(File entry);
    bool _fileLooksLikeBook(File entry);
    std::string _getMetadataAtIndex(BookRecord record, uint16_t i);
    bool _getPaginationFile(BookRecord record, char *outFilename);
    uint32_t numBooks = 0;
private:
    OpenBookDatabase();
};

#endif // OpenBookDatabase_h

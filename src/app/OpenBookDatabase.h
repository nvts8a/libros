#ifndef OpenBookDatabase_h
#define OpenBookDatabase_h

#include <stdint.h>
#include <string>
#include <vector>
#include "OpenBookDevice.h"

static const std::string HEADER_FILE = "/_LIBRARY/_HEADER";
static const std::string PAGES_FILE = "/_PAGES";
static const std::string BOOK_FILE = "/_BOOK";
static const std::string OBP_FILE = "/_OBP";
static const std::string BOOKS_DIR = "/BOOKS";
#define LIBRARY_DIR    ("/_LIBRARY/")
#define BACKUP_DIR     ("/_LIBBACK/")
#define WORKING_DIR    ("/_LIBTEMP/")

#define DATABASE_VERSION  (0x0003)

static const uint16_t LIBRARY_PAGE_SIZE = 12;

static const uint64_t DATABASE_FILE_IDENTIFIER = 6825903261955698688;
static const uint64_t PAGINATION_FILE_IDENTIFIER = 4992030523817504768; 
static const int TXT_EXTENSION = 1954051118;

// Header
static const int HEADER_TAG_COUNT  = 5;
static const int HEADER_DELIMITER  = 170732845;
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
    char fileHash[64];
    uint64_t fileSize = 0;
    uint64_t textStart = 0;
    uint64_t flags = 0;
    BookField metadata[HEADER_TAG_COUNT];
} BookRecord;

typedef struct {
    uint64_t magic = DATABASE_FILE_IDENTIFIER;
    uint32_t version = DATABASE_VERSION;
    char bookDirectoryHash[64];
} BookDatabaseHeader;

// structs for the .pag pagination files

typedef struct {
    uint64_t magic = PAGINATION_FILE_IDENTIFIER;
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
    std::vector<BookRecord, std::allocator<BookRecord>> getBookRecords();
    std::vector<std::string> getLibraryPage(uint16_t page);
    BookRecord getBookRecord(char* fileHash);
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
    File _findOrCreateLibraryFile();
    void _updateHeaderFile();
    void _setBookDirectoryHash(char* bookDirectoryHash);
    bool _booksDirectoryChanged();
    bool _copyTxtFilesToBookDirectory();
    void _processNewAndLoadCurrentLibrary();
    void _loadCurrentLibrary();
    void _writeNewBookRecordFiles();
    bool _fileIsTxt(File entry);
    void _setFileHash(char* fileHash, File entry);
    bool _hasHeader(File entry);
    BookRecord _processBookFile(File entry, char* fileHash);
    std::string _getMetadataAtIndex(BookRecord record, uint16_t i);
    const char* _getPaginationFilename(BookRecord record);
    const char* _getCurrentPageFilename(BookRecord record);

    char bookDirectoryHash[64] = {0};
    std::vector <BookRecord>Records;
private:
    OpenBookDatabase();
};

#endif // OpenBookDatabase_h

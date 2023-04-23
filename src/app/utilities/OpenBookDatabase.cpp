#include "OpenBookDatabase.h"
#include "Logger.h"
#include "sha256.h"
#include <map>

OpenBookDatabase::OpenBookDatabase() {}

/**
 * Run on an instance of OpenBookDatabase, this loads the library file from disk
 *  then it ensures the validity of the database and sets class variables based on the current header.
 * @return A boolean if the OpenBookDatabase of if the database is present and valid
*/
bool OpenBookDatabase::connect() {
    File database = this->_findOrCreateLibraryFile();

    BookDatabaseVersion version;
    database.read((byte *)&version, sizeof(BookDatabaseVersion));
    database.close();

    if (version.magic != VERSION_FILE_ID) {
        Logger::ERROR("OpenBookDatabase failed to initialize- magic: " + std::to_string(version.magic) + " should equal " + std::to_string(VERSION_FILE_ID));
        return false;
    } else if (version.version != DATABASE_VERSION) {
        Logger::ERROR("OpenBookDatabase failed to initialize- header.version: " + std::to_string(version.version) + " should equal " + std::to_string(DATABASE_VERSION));
        return false;
    }

    this->version = version;
    Logger::DEBUG("OpenBookDatabase initialized with-     version: " + std::to_string(this->version.version));
    Logger::DEBUG("OpenBookDatabase initialized with- libraryHash: " + std::string(this->version.libraryHash));
    return true;
}

/**
 * Ensures the appropriate library directories are present, if not, creates them.
 *  Recovers from a backup if a backup is present, creates the library header binary if one doesn't exist,
 *  then cleans up working and backup files, if present.
 * Then loads the library header file and returns it.
 * @return The library header file
*/
File OpenBookDatabase::_findOrCreateLibraryFile() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    if (!device->fileExists(BOOKS_DIR.c_str())) device->makeDirectory(BOOKS_DIR.c_str()); // TODO: Bulletproof this by ignoring case

    if (!device->fileExists(LIBRARY_DIR)) {
        if (device->fileExists(BACKUP_DIR)) {
            device->renameFile(BACKUP_DIR, LIBRARY_DIR);
        } else device->makeDirectory(LIBRARY_DIR);
    }

    std::string versionFilename = LIBRARY_DIR + VERSION_FILE;
    if (!device->fileExists(versionFilename.c_str())) {
        Logger::WARN("Library version file missing. Creating a new one at: " + versionFilename);

        File versionFile = OpenBookDevice::sharedDevice()->openFile(versionFilename.c_str(), O_CREAT | O_RDWR);
        BookDatabaseVersion version;
        versionFile.write((byte *)&version, sizeof(BookDatabaseVersion));
        versionFile.flush(); versionFile.close();
    }

    if (device->fileExists(WORKING_DIR)) device->removeDirectoryRecursive(WORKING_DIR);
    if (device->fileExists(BACKUP_DIR))  device->removeDirectoryRecursive(BACKUP_DIR);

    return device->openFile(versionFilename.c_str());
}

/**
 * Creates a hash of the contents of the Books directory.
 * @param libraryHash The pointer to the destination to store the hash.
*/
void OpenBookDatabase::_setLibraryHash(char* libraryHash) {
    std::string booksDirectory = "";
    File booksDirectoryFile = OpenBookDevice::sharedDevice()->openFile(BOOKS_DIR.c_str());
    File child;

    while (child = booksDirectoryFile.openNextFile()) {
        char childName[128]; child.getName(childName, 128);
        booksDirectory = booksDirectory + std::string(childName) + std::to_string(child.fileSize());
        child.close();
    } booksDirectoryFile.close();

    SHA256 sha256; std::string booksSha = sha256(booksDirectory.c_str(), booksDirectory.length()).substr(0, 63);;
    strcpy(libraryHash, booksSha.c_str());
    Logger::DEBUG("Books Directory unique string: " + booksDirectory);
    Logger::DEBUG("Library Hash generated: " + std::string(libraryHash));
}

/**
 * Runs the method to indentify new text files in the BOOKS dir and processes them
 * @return TODO: is this needed?
*/
bool OpenBookDatabase::scanForNewBooks() {
    char newLibraryHash[64]; this->_setLibraryHash(newLibraryHash);
    bool libraryChanged = memcmp(this->version.libraryHash, newLibraryHash, 64);
    bool movedFiles     = this->_copyTxtFilesToBookDirectory();

    if (movedFiles || libraryChanged) {
        this->_processNewTxtFiles();
        this->_writeNewBookRecordFiles();
    } else this->_setLibrary();

    return true;
}


/**
 * Moves any txt files from the root of the SD card to the correct location in the Books directory
 * @return True if the the method moved any files
*/
bool OpenBookDatabase::_copyTxtFilesToBookDirectory() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    std::string rootFilepath = "/";

    Logger::INFO("Looking for files in " + rootFilepath + " to copy to " + BOOKS_DIR + "..."); Logger::LOAD_TEST();

    bool movedFile = false;
    File root = device->openFile(rootFilepath.c_str());
    File entry;

    while (entry = root.openNextFile()) {
        char filename[128];
        entry.getName(filename, 128);
        Logger::DEBUG("File in root " + std::string(filename));
        if (this->_fileIsTxt(entry)) {
            char bookFilename[128]; entry.getName(bookFilename, 128);
            std::string newBookPath = BOOKS_DIR + '/' + std::string(bookFilename);
            Logger::WARN(std::string(bookFilename) + " found in the root of the SD card. Moving it to: " + newBookPath);

            if(device->renameFile(bookFilename, newBookPath.c_str())) movedFile = true;
        } entry.close();
    } root.close();

    Logger::INFO("Completed looking for and moving files that belong in " + BOOKS_DIR); Logger::LOAD_TEST();
    return movedFile;
}


/**
 * Using the BOOKS dir, looks at all files in it. Then, using the files hash, if the BookRecord files
 *  already exist in the LIBRARY dir, will read in that BookRecord into the active Database vector.
 *  If it doesn't yet exist, will process that new text file into an active BookRecord.
*/
void OpenBookDatabase::_processNewTxtFiles() {
    Logger::INFO("Processing new text files..."); Logger::LOAD_TEST();

    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    File booksDirectory = device->openFile(BOOKS_DIR.c_str());
    File entry;

    while (entry = booksDirectory.openNextFile()) {
        if (this->_fileIsTxt(entry)) {
            char fileHash[64]; _setFileHash(fileHash, entry);
            std::string bookRecordDirectory = LIBRARY_DIR + std::string(fileHash);

            if (device->fileExists(bookRecordDirectory.c_str())) {
                this->Records.push_back(this->_getBookRecord(fileHash));
            } else this->Records.push_back(this->_processBookFile(entry, fileHash));
        } entry.close();
    } booksDirectory.close();

    Logger::INFO("Completed processing new text files."); Logger::LOAD_TEST();
}

/**
 * Using the Database's vector of BookRecords, will either move those BookRecords to the WORKING dir,
 *  or create new BookRecords for them. Then renames the WORKING dir to the LIBRARY dir. It does this
 *  such that any old BookRecords will be cleaned up. TODO: Find a more performant way to do this.
*/
void OpenBookDatabase::_writeNewBookRecordFiles() {
    Logger::INFO("Writing new BookRecords to disk..."); Logger::LOAD_TEST();

    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    device->makeDirectory(WORKING_DIR);

    for (uint16_t i = 0; i < this->getLibrarySize(); i++) {
        std::string tempBookDirectory = WORKING_DIR + std::string(this->Records[i].fileHash);
        std::string libraryBookDirectory = LIBRARY_DIR + std::string(this->Records[i].fileHash);
        if (device->fileExists(libraryBookDirectory.c_str())) {
            Logger::DEBUG("Current BookRecord found, moving: " + libraryBookDirectory + " to " + tempBookDirectory);
            device->renameFile(libraryBookDirectory.c_str(), tempBookDirectory.c_str());
        } else {
            std::string tempBookFilename = tempBookDirectory + BOOK_FILE;
            Logger::DEBUG("Writing new BookRecord to: " + tempBookFilename);
            device->makeDirectory(tempBookDirectory.c_str());
            File tempBook = device->openFile(tempBookFilename.c_str(), O_RDWR | O_CREAT);
            tempBook.write((byte *)&this->Records[i], sizeof(BookRecord));
            tempBook.flush(); tempBook.close();
            Logger::DEBUG("Completed writing new BookRecord to: " + tempBookFilename);
        }
    }
    // Create new Library verion file for the updates
    this->_setLibraryHash(version.libraryHash);
    std::string versionFilename = WORKING_DIR + VERSION_FILE;
    File versionFile = OpenBookDevice::sharedDevice()->openFile(versionFilename.c_str(), O_TRUNC | O_RDWR);
    versionFile.write((byte *)&this->version, sizeof(BookDatabaseVersion));
    versionFile.flush(); versionFile.close();

    // Persist and clean up files
    device->renameFile(LIBRARY_DIR, BACKUP_DIR);
    device->renameFile(WORKING_DIR, LIBRARY_DIR);
    device->removeDirectoryRecursive(BACKUP_DIR);

    Logger::INFO("Completed writing new BookRecord files to disk."); Logger::LOAD_TEST();
}

/**
 * Using the LIBRARY dir, looks for BookRecord directories in there. Using the BookRecord
 *  directory name, constructs the BookRecord and pushes it to the running database vector
 *  of BookRecords
*/
void OpenBookDatabase::_setLibrary() {
    Logger::DEBUG("No changes to the Library Database detected, loading from disk..."); Logger::LOAD_TEST();

    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    File libraryDirectory = device->openFile(LIBRARY_DIR);
    File bookRecord;

    while (bookRecord = libraryDirectory.openNextFile()) {
        if (bookRecord.isDirectory()) {
            char bookRecordHash[64]; bookRecord.getName(bookRecordHash, 64);
            this->Records.push_back(this->_getBookRecord(bookRecordHash));
        }
        bookRecord.close();
    } libraryDirectory.close();

    Logger::DEBUG("Completed loading the Library Database from disk."); Logger::LOAD_TEST();
}

/**
 * Returns a boolean of if the provided file ends in ".txt" and isn't hidden, starting with '.'
 * @param  entry The File object to be checked
 * @return True if the file is determined to be a text file
*/
bool OpenBookDatabase::_fileIsTxt(File entry) {
    if (entry.isDirectory()) return false;

    uint32_t extension = 0;
    char filename[128];

    entry.getName(filename, 128);
    memcpy((byte *)&extension, filename + (strlen(filename) - 4), 4);

    return (extension == TXT_EXTENSION && filename[0] != '.');
}

/**
 * This method takes a file, already determined to be a text file, and will
 *  appropriately process it into a BookRecord.
 * @param entry A text file to be created into a BookRecord
 * @param fileHash A unique representation of the file to be used as the directory
 * @return A successfully processed BookRecord
*/
BookRecord OpenBookDatabase::_processBookFile(File entry, char* fileHash) {

    BookRecord record = {0};

    // Copy file data to BookRecord
    char bookFilename[128]; entry.getName(bookFilename, 128);
    std::string bookPath = BOOKS_DIR + '/' + std::string(bookFilename);

    Logger::INFO("Processing new BookRecord " + bookPath + ": " + std::string(fileHash)); Logger::LOAD_TEST();
    strcpy(record.filename, bookPath.c_str());
    strcpy(record.fileHash, fileHash);
    record.fileSize = entry.size();

    if (this->_hasHeader(entry)) {  // if file is a text file AND it has front matter, parse the front matter.
        entry.seekSet(4);
        bool done = false;
        while (!done) {
            uint32_t tag;

            entry.read((byte *)&tag, sizeof(tag));
            if (tag == HEADER_DELIMITER) {
                done = true;
                record.textStart = entry.position();
                break;
            }
            
            char c;
            do { c = entry.read(); } while (c != ':');  // skip delimiter
            do { c = entry.read(); } while (c == ' ');  // skip whitespace

            uint64_t loc = entry.position() - 1;
            uint64_t len = 0;
            do { c = entry.read(); len++; } while (c != '\n'); // len is now the length of the metadata

            BookField field;
            field.tag = tag;
            field.loc = loc;
            field.len = len;

            switch (tag) {
                case (TAG_TITLE):
                    record.metadata[INDEX_TITLE] = field;
                    break;
                case (TAG_AUTHOR):
                    record.metadata[INDEX_AUTHOR] = field;
                    break;
                case (TAG_GENRE):
                    record.metadata[INDEX_GENRE] = field;
                    break;
                case (TAG_DESCRIPTION):
                    record.metadata[INDEX_DESCRIPTION] = field;
                    break;
                case (TAG_LANGUAGE):
                    record.metadata[INDEX_LANGUAGE] = field;
                    break;
                default:
                    break;
            }            
        }
    } else { // if it's just a text file, use the first line as the title.
        BookField field;
        field.tag = TAG_TITLE;
        field.loc = 0;
        field.len = min(record.fileSize, (uint64_t)32); // up to 32 characters
        for(int i = 0; i < field.len; i++) { // but truncate it at the first newline
            char c = entry.read();
            if (c == '\r' || c == '\n') field.len = i;
        }
        record.metadata[INDEX_TITLE] = field;
    }

    Logger::INFO("Completed processing new BookRecord " + bookPath + ": " + std::string(fileHash)); Logger::LOAD_TEST();
    return record;
}

/**
 * Creates a 64 byte SHA256 of the first 512 bytes of a book file.
 *  This is used as the idenitifier for the book.
 * TODO: This could be better, can you take in an entire file?
 * @param fileHash The 64 byte char array of the file hash value
 * @param file The file to be hashed
*/
void OpenBookDatabase::_setFileHash(char* fileHash, File file) {
    SHA256 sha256;
    char fileBytes[512];  file.readBytes(fileBytes, 512); file.seekSet(0);
    char filename[64];    file.getName(filename, 64);
    std::string fileSha = sha256(fileBytes, 64).substr(0, 63);

    Logger::DEBUG("File hash for " + std::string(filename) + " determined to be: " + fileSha);
    strcpy(fileHash, fileSha.c_str());
}

/**
 * Determines if a file has a BookRecord Header Block by
 *  seeing if the first four characters are "---\n"
 * @param  entry The file to be checked
 * @return True if the file has a start to the Header Block
*/
bool OpenBookDatabase::_hasHeader(File entry) {
    uint32_t magic;

    entry.seekSet(0);
    entry.read((void *)&magic, sizeof(magic));
    entry.seekSet(0);

    return (magic == HEADER_DELIMITER);
}

/**
 * @return A vector of all the current BookRecords in memory
*/
std::vector<BookRecord> OpenBookDatabase::getLibrary() {
    return this->Records;
}

/**
 * @return The number of the current BookRecords in memory
*/
uint16_t OpenBookDatabase::getLibrarySize() {
    return this->Records.size();
}

/**
 * Constructs the list of titles for the provided library page
 * @param page The page you want titles for
 * @return A vector of strings of titles for the provided page
*/
std::vector<BookRecord> OpenBookDatabase::getLibraryPage(uint16_t page) {
    std::vector<BookRecord> bookRecords;
    uint16_t pageStart = LIBRARY_PAGE_SIZE * page;
    uint16_t pageEnd = min(pageStart + LIBRARY_PAGE_SIZE, (uint16_t)this->getLibrarySize());

    for (uint16_t i = pageStart; i < pageEnd; i++) bookRecords.push_back(this->getLibraryBookRecord(i));

    return bookRecords;
}

/**
 * @param libraryIndex The index position of the book in the library
 * @return The in memory BookRecord at the provided library index
*/
BookRecord OpenBookDatabase::getLibraryBookRecord(uint16_t libraryIndex) {
    return this->Records.at(libraryIndex);
}

/**
 * @param libraryIndex The index position of the book in the library
 * @param bookRecord The BookRecord object that is replacing the value at the provided library index
*/
void OpenBookDatabase::setLibraryBookRecord(uint16_t libraryIndex, BookRecord bookRecord) {
    this->Records.at(libraryIndex) = bookRecord;
}

/**
 * Using the fileHash of a book file, gets a stored BookRecord object for that book file.
 * TODO: should handle missing BookRecords more gracefully, I think it'll currently just read garbage
 * @param fileHash the char array of the hash of a book file, to be used in the directory of the BookRecord
 * @return the BookRecord object for that book's file hash
*/
BookRecord OpenBookDatabase::_getBookRecord(char* fileHash) {
    BookRecord bookRecord;
    std::string bookRecordFilename = LIBRARY_DIR + std::string(fileHash) + BOOK_FILE;
    File bookFile = OpenBookDevice::sharedDevice()->openFile(bookRecordFilename.c_str());
    bookFile.read((byte *)&bookRecord, sizeof(BookRecord));
    bookFile.close();

    return bookRecord;
}

std::string OpenBookDatabase::getBookTitle(BookRecord record) {
    return this->_getMetadataAtIndex(record, INDEX_TITLE);
}

std::string OpenBookDatabase::getBookAuthor(BookRecord record) {
    return this->_getMetadataAtIndex(record, INDEX_AUTHOR);
}

std::string OpenBookDatabase::getBookDescription(BookRecord record) {
    return this->_getMetadataAtIndex(record, INDEX_DESCRIPTION);
}

/**
 * Gets the last page number the user was on for a BookRecord, or 0 if the book has not been read
 * @param  record The BookRecord of the book the user is getting the last page of
 * @return The integer page number of the book
*/
uint32_t OpenBookDatabase::getCurrentPage(BookRecord record) {
    uint32_t retval = 0;
    char currentPageFilename[128]; this->_getCurrentPageFilename(record, currentPageFilename);

    if (OpenBookDevice::sharedDevice()->fileExists(currentPageFilename)) {
        File pageFile = OpenBookDevice::sharedDevice()->openFile(currentPageFilename);
        pageFile.read((void *)&retval, 4);
        pageFile.close();
    }

    return retval;
}

/**
 * Sets the current page number the user is on for a BookRecord to a page file
 * @param record The BookRecord of the book the user is setting the current page of
 * @param pageNumber The integer page number to set the current page to
*/
void OpenBookDatabase::setCurrentPage(BookRecord record, uint32_t pageNumber) {
    char currentPageFilename[128]; this->_getCurrentPageFilename(record, currentPageFilename);

    File pageFile = OpenBookDevice::sharedDevice()->openFile(currentPageFilename, O_CREAT | O_WRITE | O_TRUNC);
    pageFile.write(&pageNumber, 4);
    pageFile.close();
}

std::string OpenBookDatabase::_getMetadataAtIndex(BookRecord record, uint16_t i) {
    BookField field = record.metadata[i];
    char *value = (char *)malloc(field.len + 1);
    File f = OpenBookDevice::sharedDevice()->openFile(record.filename);

    f.seekSet(field.loc);
    f.read((void *)value, field.len);
    f.close();
    value[field.len] = 0;
    std::string retval = std::string(value);
    free(value);

    return retval;
}

bool OpenBookDatabase::bookIsPaginated(BookRecord bookRecord) {
    char paginationFilename[128]; this->_getPaginationFilename(bookRecord, paginationFilename);
    return OpenBookDevice::sharedDevice()->fileExists(paginationFilename);
}

/**
 * Paginates a provided BookRecord by parsing its filename for chapters and formatted pages,
 *      then writes those chapters and pages to disk to be used when reading the book, and writes
 *      an updated BookRecord to disk with new metadata.
 * @param record The BookRecord to be paginated and updated
 * @return a BookRecord object that was saved to disk with updated numChapters and numPages values
*/
BookRecord OpenBookDatabase::paginateBook(BookRecord bookRecord) {
    Logger::INFO("Starting pagination of " + std::string(bookRecord.filename) + "..."); Logger::LOAD_TEST();
    OpenBookDevice *device = OpenBookDevice::sharedDevice();

    // Get the Pages
    std::vector<uint32_t> pages;
    std::vector<uint32_t> chapters;
    std::tie(pages, chapters) = this->_generatePages(bookRecord);

    // Write the Pages
    char paginationFilename[128]; this->_getPaginationFilename(bookRecord, paginationFilename);
    Logger::DEBUG("paginationFilename: " + std::string(paginationFilename));
    File paginationFile = device->openFile(paginationFilename, O_CREAT | O_RDWR);
    for (auto page : pages) paginationFile.write((byte *)&page, sizeof(uint32_t));
    paginationFile.flush(); paginationFile.close();

    // Write the Chapters
    char chaptersFilename[128]; this->_getChaptersFilename(bookRecord, chaptersFilename);
    File chaptersFile = device->openFile(chaptersFilename, O_CREAT | O_RDWR);
    for (auto chapter : chapters) chaptersFile.write((byte *)&chapter, sizeof(uint32_t));
    chaptersFile.flush(); chaptersFile.close();
    
    // Update the BookRecord
    bookRecord.numChapters = chapters.size();
    bookRecord.numPages = pages.size()-1;
    std::string bookRecordFilename = LIBRARY_DIR + std::string(bookRecord.fileHash) + BOOK_FILE;
    File bookRecordFile = device->openFile(bookRecordFilename.c_str(), O_TRUNC | O_RDWR);
    bookRecordFile.write((byte *)&bookRecord, sizeof(BookRecord));
    bookRecordFile.flush(); bookRecordFile.close();

    Logger::INFO("Pagination of " + std::string(bookRecord.filename) + " completed."); Logger::LOAD_TEST();
    return bookRecord;
}

/**
 * Parses over a book text file, using the page view size and babel to determine the most
 *      amount of bytes, formated appropriatly, that can fit on a page before a new page
 *      is needed, storing that location, and restarting, until all formated page lengths
 *      are found and returned.
 * @param bookRecord the BookRecord object with the book filename to be parsed
 * @return a tuple, a vector of page byte start locations and vector locations of the chapters
*/
std::tuple<std::vector<uint32_t>, std::vector<uint32_t>> OpenBookDatabase::_generatePages(BookRecord bookRecord) {
    std::vector<uint32_t> pages = {};
    std::vector<uint32_t> chapters = {};
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    BabelDevice *babel = device->getTypesetter()->getBabel();

    File bookFile = device->openFile(bookRecord.filename);
    bookFile.seekSet(bookRecord.textStart);
    pages.push_back(bookRecord.textStart);
    uint16_t yPos = 0; // Used to calculate when a page grows past the page height

    do {
        // Read in bytes from the book file and convert them with babel
        char rawBookFileBytes[READ_AMOUNT + 1] = {0};
        int byteCountRead = bookFile.read(rawBookFileBytes, READ_AMOUNT);
        BABEL_CODEPOINT babelGlyphBytes[READ_AMOUNT];
        babel->utf8_parse(rawBookFileBytes, babelGlyphBytes);

        // If the first character of the babel bytes is a chapter marker, process the chapter details
        if (babelGlyphBytes[0] == CHAPTER_MARK) {
            bookFile.seekSet(bookFile.position() - byteCountRead); // Reset the file location, we look for a newline, not a break point
            // If the previous page has data, close it out and push it onto the page vector
            if (bookFile.position() > pages.back()) pages.push_back(bookFile.position());

            chapters.push_back(pages.size());     // Push the size of the pages vector as the chapter marker, this could happen after but we'd have to subtract 1
            bookFile.find(NEW_LINE);              // Find the end of the chapter title data
            pages.push_back(bookFile.position()); // Push the position read to as the start of the next page
            yPos = 0;                             // Reset page position

        // If there's a Form Feed character at the start of the line, break the page after it by subtracting 1 from what was read in
        } else if (babelGlyphBytes[0] == PAGE_BREAK) {
            bookFile.seekSet(bookFile.position()-(byteCountRead - 1));
            pages.push_back(bookFile.position());
            yPos = 0;

        // ...otherwise process it onto a page
        } else {
            // First, we find the last appropriate break point in the processed line of data for the width of the eReader page
            bool lineWrapped = false;
            size_t bytePosition;
            int16_t breakPosition = babel->word_wrap_position(babelGlyphBytes, byteCountRead, &lineWrapped, &bytePosition, PAGE_WIDTH, 1);

            // If the bytePosition is present, reset the file position to it's length prior to reading...
            //  (bytePosition is prefered but can be empty for long words, cut the last line short, or present for EOF characters)
            if (bytePosition > 0) bookFile.seekSet(bookFile.position() + (bytePosition - byteCountRead));
            // ...or if the breakPosition is present, reset the file position to it's length prior to reading...
            //   (breakPosition will be the entire line for long unbroken words, however unusual)
            else if (breakPosition > 0) bookFile.seekSet(bookFile.position() + (breakPosition - byteCountRead));
            // ...or it will be 0 or -1 if the file is empty or only contains control characters, we can stop processing
            else break;

            // If the line wraps, as in a paragraph continuing, we provide less white space than if it doesn't and a new paragraph is starting
            yPos += 16 + 2 + (lineWrapped ? 0 : 8);
            // If another line would put the page beyond the height of the page, store the position, and reset the page data for a fresh one
            if (yPos + 16 + 2 > PAGE_HEIGHT) {
                pages.push_back(bookFile.position());
                yPos = 0;
            }
        }
    } while (bookFile.available());

    pages.push_back(bookFile.size()); // Close out the last page
    bookFile.close(); // Make sure our file is closed

    return std::make_tuple(pages, chapters);
}

/**
 * Gets the text for a book's page! Perscribed by our pagination and babel.
 * @param bookRecord the BookRecord object used to get the book file and text and pagination file and page data
 * @param page the interger page number to retrieve
 * @return a string of text from the book's text file, starting and as long as persribed by the pagination file for the page number
*/
std::string OpenBookDatabase::getTextForPage(BookRecord bookRecord, uint32_t page) {
    std::string retval = "";
    char paginationFilename[128]; this->_getPaginationFilename(bookRecord, paginationFilename);

    if (page < bookRecord.numPages && OpenBookDevice::sharedDevice()->fileExists(paginationFilename)) {
        File paginationFile = OpenBookDevice::sharedDevice()->openFile(paginationFilename);
        paginationFile.seekSet(page * sizeof(uint32_t));

        uint32_t startPosition; paginationFile.read(&startPosition, sizeof(uint32_t));
        uint32_t endPosition;   paginationFile.read(&endPosition, sizeof(uint32_t));
        paginationFile.close();

        uint32_t pageByteSize = endPosition - startPosition;

        File bookFile = OpenBookDevice::sharedDevice()->openFile(bookRecord.filename);
        bookFile.seekSet(startPosition);

        char *buf = (char *)malloc(pageByteSize + 1);
        bookFile.read(buf, pageByteSize);
        buf[pageByteSize] = 0; // TODO: What's this do?
        bookFile.close();

        retval = std::string(buf); free(buf);
    }

    return retval;
}

/**
 * This method takes in a BookRecord and using it's file hash, constructs a string of its pages file location
 * @param  record The BookRecord object you're attempting to get the pagination file for
 * @return A char array of BookRecords pagination file location
 */
void OpenBookDatabase::_getPaginationFilename(BookRecord record, char* paginationFilename) {
    strcpy(paginationFilename, (LIBRARY_DIR + std::string(record.fileHash) + PAGES_FILE).c_str());
}

/**
 * This method takes in a BookRecord and using it's file hash, constructs a string of its chapters file location
 * @param  record The BookRecord object you're attempting to get the chapters file for
 * @return A char array of BookRecords chapters file location
 */
void OpenBookDatabase::_getChaptersFilename(BookRecord record, char* chaptersFilename) {
    strcpy(chaptersFilename, (LIBRARY_DIR + std::string(record.fileHash) + CHAPTERS_FILE).c_str());
}

/**
 * This method takes in a BookRecord and using it's file hash, constructs a string of its OBP file location
 * @param  record The BookRecord object you're attempting to get the current file for
 * @return A char array of BookRecords current page file location
 */
void OpenBookDatabase::_getCurrentPageFilename(BookRecord record, char* currentPageFilename) {
    strcpy(currentPageFilename, (LIBRARY_DIR + std::string(record.fileHash) + CURRENT_PAGE_FILE).c_str());
}
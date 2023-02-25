#include "OpenBookDatabase.h"
#include "sha256.h"
#include <map>
#include <vector>

OpenBookDatabase::OpenBookDatabase() {}

/**
 * Run on an instance of OpenBookDatabase, this loads the library file from disk
 *  then it ensures the validity of the database and sets class variables based on the current header.
 * @return A boolean if the OpenBookDatabase of if the database is present and valid
*/
bool OpenBookDatabase::connect() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    File database = this->_findOrCreateLibraryFile(device);

    uint64_t magic;
    BookDatabaseHeader header;
    database.read((byte *)&magic, sizeof(magic));
    database.read((byte *)&header, sizeof(BookDatabaseHeader));
    database.close();

    if (magic != DATABASE_FILE_IDENTIFIER)  return false;
    if (header.version != DATABASE_VERSION) return false;

    this->numBooks = header.numBooks;
    return true;
}

/**
 * Ensures the appropriate library directories are present, if not, creates them.
 *  Recovers from a backup if a backup is present, creates the library header binary if one doesn't exist,
 *  then cleans up working and backup files, if present.
 * Then loads the library header file and returns it.
 * @param  device The OpenBook hardware device
 * @return The library header file
*/
File OpenBookDatabase::_findOrCreateLibraryFile(OpenBookDevice* device) {
    if (!device->fileExists(DATABASE_DIR))   device->makeDirectory(DATABASE_DIR);
    if (!device->fileExists(PAGINATION_DIR)) device->makeDirectory(PAGINATION_DIR);
    if (!device->fileExists(BOOKS_DIR))      device->makeDirectory(BOOKS_DIR); // TODO: Bulletproof this by ignoring case

    if (!device->fileExists(LIBRARY_DIR)) {
        if (device->fileExists(BACKUP_DIR)) {
            device->renameFile(BACKUP_DIR, LIBRARY_DIR);
        } else device->makeDirectory(LIBRARY_DIR);
    }

    std::string headerFilename = LIBRARY_DIR + HEADER_FILE;
    if (!device->fileExists(headerFilename.c_str())) {
        File header = device->openFile(headerFilename.c_str(), O_CREAT | O_RDWR);
        BookDatabaseHeader blankHeader;
        header.write((byte *)&DATABASE_FILE_IDENTIFIER, sizeof(DATABASE_FILE_IDENTIFIER));
        header.write((byte *)&blankHeader, sizeof(BookDatabaseHeader));
        header.flush();
        header.close();
    }

    if (device->fileExists(WORKING_DIR)) device->removeDirectoryRecursive(WORKING_DIR);
    if (device->fileExists(BACKUP_DIR))  device->removeDirectoryRecursive(BACKUP_DIR);

    return device->openFile(headerFilename.c_str());
}

/**
 * Takes in the configured directory for books and iterates over every file,
 *  if the file is ends in .txt, it'll process that file and add it to the library binary
 *  then writing a header and the book records in the binary to disk.
 * @return TODO: is this needed?
*/
bool OpenBookDatabase::scanForNewBooks() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    // TODO: scan the existing database for book progress and store it in a map

    // Find books, increment the count, and process them
    File root = device->openFile(BOOKS_DIR);
    File entry;
    std::vector <BookRecord>Records;
    uint16_t numBooks = 0;
    while (entry = root.openNextFile()) {
        if (this->_fileIsTxt(entry)) {
            numBooks++;
            Records.push_back(this->_processBookFile(entry));
        }
        entry.close();
    }

    // Now that we have all the data processed we can write the Library files
    device->makeDirectory(WORKING_DIR);
    BookDatabaseHeader header;
    header.numBooks = this->numBooks = numBooks;

    for(int i = 0; i < min(MAX_BOOKS, (uint16_t)Records.size()); i++) {
        std::string tempBookFilename = WORKING_DIR + std::to_string(i);
        File tempBook = device->openFile(tempBookFilename.c_str(), O_RDWR | O_CREAT);
        tempBook.write((byte *)&Records[i], sizeof(BookRecord));
        tempBook.flush(); tempBook.close();
    }

    std::string tempHeaderFilename = WORKING_DIR + HEADER_FILE;
    File tempHeader = device->openFile(tempHeaderFilename.c_str(), O_RDWR | O_CREAT);
    tempHeader.write((byte *)&DATABASE_FILE_IDENTIFIER, sizeof(DATABASE_FILE_IDENTIFIER));
    tempHeader.write((byte *)&header, sizeof(BookDatabaseHeader));
    tempHeader.flush(); tempHeader.close();
    
    // Persist and clean up files
    device->renameFile(LIBRARY_DIR, BACKUP_DIR);
    device->renameFile(WORKING_DIR, LIBRARY_DIR);
    device->removeDirectoryRecursive(BACKUP_DIR);

    return true;
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
 * @return A successfully processed BookRecord
*/
BookRecord OpenBookDatabase::_processBookFile(File entry) {
    SHA256 sha256;
    BookRecord record = {0};

    // Copy file data to BookRecord
    entry.getName(record.filename, 128);
    memcpy((void *)&record.fileHash, sha256(std::string(record.filename)).c_str(), sizeof(record.fileHash));
    record.fileSize = entry.size();
    record.currentPosition = 0; // TODO: copy from map

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
        field.len = min(record.fileSize, 32); // up to 32 characters
        for(int i = 0; i < field.len; i++) { // but truncate it at the first newline
            char c = entry.read();
            if (c == '\r' || c == '\n') field.len = i;
        }
        record.metadata[INDEX_TITLE] = field;
    }

    return record;
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

uint32_t OpenBookDatabase::getNumberOfBooks() {
    return this->numBooks;
}

BookRecord OpenBookDatabase::getBookRecord(uint32_t i) {
    BookRecord bookRecord;
    //std::string bookRecordFilename = LIBRARY_DIR + std::to_string(this->bookShas[i]);
    std::string bookRecordFilename = LIBRARY_DIR + std::to_string(i);
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
    const char *pageFilename = this->_getCurrentPageFilename(record);

    if (OpenBookDevice::sharedDevice()->fileExists(pageFilename)) {
        File pageFile = OpenBookDevice::sharedDevice()->openFile(pageFilename);
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
    const char *pageFilename = this->_getCurrentPageFilename(record);

    File pageFile = OpenBookDevice::sharedDevice()->openFile(pageFilename, O_CREAT | O_WRITE | O_TRUNC);
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

bool OpenBookDatabase::bookIsPaginated(BookRecord record) {
    return OpenBookDevice::sharedDevice()->fileExists(this->_getPaginationFilename(record));
}

/**
 * Paginates a provided BookRecord by ... 
 *  and outputing a pagination file in the same directory the BookRecord is found in.
 *  The paginations file is a .pag binary file containing a header of metadata,
 * @param record The BookRecord to be paginated
*/
void OpenBookDatabase::paginateBook(BookRecord record) {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    File paginationFile;
    const char *paginationFilename = this->_getPaginationFilename(record);

    if (device->fileExists(paginationFilename)) { // If the pagination file for the BookRecord exists
        paginationFile = device->openFile(paginationFilename, O_RDWR | O_TRUNC); // ...open it with read/write and truncate
    } else {
        paginationFile = device->openFile(paginationFilename, O_RDWR | O_CREAT); // ...or else, create and open it read/write
    }

    // Write an empty placeholder header
    BookPaginationHeader header = {0};
    paginationFile.write((byte *)&header, sizeof(BookPaginationHeader));
    paginationFile.seekSet(0);  // TODO: Is this needed?
    paginationFile.flush();
    paginationFile.close();

    // now process the whole file and seek out chapter headings.
    BookChapter chapter = {0};
    File bookFile = device->openFile(record.filename);
    bookFile.seekSet(record.textStart);
    do {
        if (bookFile.read() == CHAPTER_MARK) {
            chapter.loc = bookFile.position() - 1;
            chapter.len++;
            header.numChapters++;
            char c;
            do {
                c = bookFile.read();
                chapter.len++;
            } while(c != '\n');
            bookFile.close();
            paginationFile = device->openFile(paginationFilename, O_RDWR | O_AT_END);
            paginationFile.write((byte *)&chapter, sizeof(BookChapter));
            paginationFile.flush();
            paginationFile.close();
            bookFile = device->openFile(record.filename);
            bookFile.seekSet(chapter.loc + chapter.len);
            chapter = {0};
        }
    } while (bookFile.available());
    bookFile.close();

    if (header.numChapters) {
        // if we found chapters, mark the table of contents as starting right after the header...
        header.tocStart = sizeof(BookPaginationHeader);
        // ...and the page index as starting right after that.
        header.pageStart = header.tocStart + header.numChapters * sizeof(BookChapter);
    } else {
        // Otherwise we have no table of contents.
        header.tocStart = 0;
        // Mark page index as starting right after the header.
        header.pageStart = sizeof(BookPaginationHeader);
    }

    // OKAY! Time to do pages. For this we have to traverse the whole file again,
    // but this time we need to simulate actually laying it out.
    BabelDevice *babel = device->getTypesetter()->getBabel();
    BookPage page = {0};
    uint16_t yPos = 0;
    char utf8bytes[128];
    BABEL_CODEPOINT codepoints[127];

    bookFile = device->openFile(record.filename);
    bookFile.seekSet(record.textStart);
    const int16_t pageWidth = 288;
    const int16_t pageHeight = 384;
    uint32_t nextPosition = 0;
    bool firstLoop = true;

    page.loc = bookFile.position();
    page.len = 0;
    do {
        uint32_t startPosition = bookFile.position();
        int bytesRead = bookFile.read(utf8bytes, 127);
        utf8bytes[127] = 0;
        bool wrapped = false;
        babel->utf8_parse(utf8bytes, codepoints);

        if (codepoints[0] == CHAPTER_MARK) {
            if (!firstLoop) {
                // close out the last chapter
                nextPosition = bookFile.position();
                bookFile.close();
                paginationFile = device->openFile(paginationFilename, O_RDWR | O_AT_END);
                paginationFile.write((byte *)&page, sizeof(BookPage));
                paginationFile.flush();
                paginationFile.close();
                bookFile = device->openFile(record.filename);
                header.numPages++;
                page.loc = page.loc + page.len;
                page.len = 0;
                bookFile.seekSet(nextPosition);
            }

            int32_t line_end = 0;
            // FIXME: handle case where no newline in 127 code points
            while(codepoints[line_end++] != '\n');
            nextPosition = startPosition + line_end;
            page.len = line_end;
            goto BREAK_PAGE;
        } else {
            size_t bytePosition;
            babel->word_wrap_position(codepoints, bytesRead, &wrapped, &bytePosition, pageWidth, 1);
            for(int i = bytePosition; i < 127; i++) {
                if (utf8bytes[i] == SPACE) bytePosition++;
                else break;
            }
            if (bytePosition > 0) {
                page.len += bytePosition;
                nextPosition = startPosition + bytePosition;
            } else {
                page.len += bytesRead;
                nextPosition = startPosition + bytesRead;
            }
        }

        if (wrapped) {
            yPos += 16 + 2;
        } else {
            yPos += 16 + 2 + 8;
        }
        
        if (yPos + 16 > pageHeight) {
BREAK_PAGE:
            bookFile.close();
            paginationFile = device->openFile(paginationFilename, O_RDWR | O_AT_END);
            paginationFile.write((byte *)&page, sizeof(BookPage));
            paginationFile.flush();
            paginationFile.close();
            bookFile = device->openFile(record.filename);
            header.numPages++;
            yPos = 0;
            page.loc = page.loc + page.len;
            page.len = 0;
        }
        bookFile.seekSet(nextPosition);
        firstLoop = false;
    } while (bookFile.available());

    bookFile.close();
    paginationFile = device->openFile(paginationFilename, O_RDWR | O_AT_END);
    paginationFile.write((byte *)&page, sizeof(BookPage));
    paginationFile.flush();
    paginationFile.close();
    header.numPages++;

    paginationFile = device->openFile(paginationFilename, O_RDWR);
    paginationFile.seekSet(0);
    paginationFile.write((byte *)&header, sizeof(BookPaginationHeader));
    paginationFile.flush();
    paginationFile.close();
}

uint32_t OpenBookDatabase::numPages(BookRecord record) {
    const char *paginationFilename = this->_getPaginationFilename(record);

    if (OpenBookDevice::sharedDevice()->fileExists(paginationFilename)) {
        BookPaginationHeader header;
        File f = OpenBookDevice::sharedDevice()->openFile(paginationFilename);
        f.read(&header, sizeof(BookPaginationHeader));
        return header.numPages;
    }

    return 0;
}

std::string OpenBookDatabase::getTextForPage(BookRecord record, uint32_t page) {
    const char *paginationFilename = this->_getPaginationFilename(record);

    if (OpenBookDevice::sharedDevice()->fileExists(paginationFilename)) {
        BookPaginationHeader header;
        BookPage pageInfo;

        File f = OpenBookDevice::sharedDevice()->openFile(paginationFilename);
        f.read(&header, sizeof(BookPaginationHeader));
        if (page >= header.numPages) return "";

        f.seekSet(header.pageStart + page * sizeof(BookPage));
        f.read(&pageInfo, sizeof(BookPage));
        f.close();

        f = OpenBookDevice::sharedDevice()->openFile(record.filename);
        f.seekSet(pageInfo.loc);
        char *buf = (char *)malloc(pageInfo.len + 1);
        f.read(buf, pageInfo.len);
        f.close();
        buf[pageInfo.len] = 0;
        std::string retval = std::string(buf);
        free(buf);

        return retval;
    }

    return "";
}

/**
 * This method takes in a BookRecord and using it's filename, removes the extension,
 *  and constructs a string using it, the pagination database directory, and the pag extension.
 * @param  record The BookRecord object you're attempting to get the pagination file for
 * @return A char array of BookRecords pagination file location
 */
const char* OpenBookDatabase::_getPaginationFilename(BookRecord record) {
    std::string bookFilename = record.filename;
    bookFilename = bookFilename.substr(0, strlen(record.filename) - 4);

    return (PAGINATION_DIR + bookFilename + PAG_EXTENSION).c_str();
}

/**
 * This method takes in a BookRecord and using it's filename, removes the extension,
 *  and constructs a string using it, the pagination database directory, and the obp extension.
 * @param  record The BookRecord object you're attempting to get the current file for
 * @return A char array of BookRecords current page file location
 */
const char* OpenBookDatabase::_getCurrentPageFilename(BookRecord record) {
    std::string bookFilename = record.filename;
    bookFilename = bookFilename.substr(0, strlen(record.filename) - 4);

    return (PAGINATION_DIR + bookFilename + OBP_EXTENSION).c_str();
}
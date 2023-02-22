#include "OpenBookDatabase.h"
#include "sha256.h"
#include <map>

static const uint64_t DATABASE_FILE_IDENTIFIER = 6825903261955698688;

OpenBookDatabase::OpenBookDatabase() {

}

bool OpenBookDatabase::connect() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    BookDatabaseHeader header;

    if (!device->fileExists(OPEN_BOOK_LIBRARY_FILENAME)) {
        if (device->fileExists(OPEN_BOOK_BACKUP_FILENAME)) {
            device->renameFile(OPEN_BOOK_BACKUP_FILENAME, OPEN_BOOK_LIBRARY_FILENAME);
        } else {
            File database = device->openFile(OPEN_BOOK_LIBRARY_FILENAME, O_CREAT | O_RDWR);
            database.write((byte *)&DATABASE_FILE_IDENTIFIER, sizeof(DATABASE_FILE_IDENTIFIER));
            database.write((byte *)&header, sizeof(header));
            database.flush();
            database.close();
        }
    }
    if (device->fileExists(OPEN_BOOK_WORKING_FILENAME)) {
        device->removeFile(OPEN_BOOK_WORKING_FILENAME);
    }
    if (device->fileExists(OPEN_BOOK_BACKUP_FILENAME)) {
        device->removeFile(OPEN_BOOK_BACKUP_FILENAME);
    }
    File database = device->openFile(OPEN_BOOK_LIBRARY_FILENAME);
    uint64_t magic;
    database.read((byte *)&magic, sizeof(magic));
    database.read((byte *)&header, sizeof(BookDatabaseHeader));
    database.close();

    if (magic != DATABASE_FILE_IDENTIFIER) {
        return false;
    }

    if (header.version != OPEN_BOOK_DATABASE_VERSION) {
        return false;
    }

    this->numBooks = header.numBooks;
    this->numFields = header.numFields;

    return true;
}

bool OpenBookDatabase::_fileIsTxt(File entry) {
    if (entry.isDirectory()) return false;

    uint32_t extension = 0;
    char filename[128];

    entry.getName(filename, 128);
    memcpy((byte *)&extension, filename + (strlen(filename) - 4), 4);
    // return true if file extension is .txt
    // and first character is not '.'
    return (extension == 1954051118 && filename[0] != '.');
}

bool OpenBookDatabase::_fileLooksLikeBook(File entry) {
    uint32_t magic = 0;
    entry.seekSet(0);
    entry.read((void *)&magic, sizeof(magic));
    entry.seekSet(0);
    // return true if file begins with three hyphens followed by a newline
    return (magic == 170732845);
}

bool OpenBookDatabase::scanForNewBooks() {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    uint32_t numBooks = 0;
    File root, entry;
    SHA256 sha256;
    std::string hash;

    // TODO: scan the existing database for book progress and store it in a map

    // Next, scan the root folder for things that look like books.
    root = device->openFile("/");
    entry = root.openNextFile();
    while (entry) {
        if (this->_fileIsTxt(entry)) {
            numBooks++;
        }
        entry.close();
        entry = root.openNextFile();
    }
    entry.close();

    // now that we have a number of books we can write the header to the temp database
    BookDatabaseHeader header;
    header.numBooks = numBooks;
    File temp = device->openFile(OPEN_BOOK_WORKING_FILENAME, O_RDWR | O_CREAT);
    temp.write((byte *)&DATABASE_FILE_IDENTIFIER, sizeof(DATABASE_FILE_IDENTIFIER));
    temp.write((byte *)&header, sizeof(BookDatabaseHeader));
    temp.flush();
    temp.close();

    root = device->openFile("/");
    entry = root.openNextFile();
    while (entry) {
        BookRecord record = {0};
        entry.getName(record.filename, 128);
        if (this->_fileIsTxt(entry)) {
            hash = sha256(std::string(record.filename));
            memcpy((void *)&record.fileHash, hash.c_str(), sizeof(record.fileHash));
            record.fileSize = entry.size();
            record.currentPosition = 0; // TODO: copy from map
            if (this->_fileLooksLikeBook(entry)) {
                // if file is a text file AND it has front matter, parse the front matter.
                uint32_t tag;
                char c;
                bool done = false;
                entry.seekSet(4);
                while(!done) {
                    entry.read((byte *)&tag, sizeof(tag));
                    if (tag == 170732845) { // ---\n, end of front matter
                        done = true;
                        record.textStart = entry.position();
                        break;
                    }
                    do {
                        c = entry.read();
                    } while (c != ':');
                    do {
                        c = entry.read();
                    } while (c == ' ');
                    uint64_t loc = entry.position() - 1;
                    uint64_t len = 0;
                    do {
                        len++;
                        c = entry.read();
                    } while (c != '\n');
                    // len is now the length of the metadata
                    BookField field;
                    field.tag = tag;
                    field.loc = loc;
                    field.len = len;
                    switch (tag) {
                        case 1280592212: // TITL
                            record.metadata[OPEN_BOOK_TITLE_INDEX] = field;
                            break;
                        case 1213486401: // AUTH
                            record.metadata[OPEN_BOOK_AUTHOR_INDEX] = field;
                            break;
                        case 1163021895: // GNRE
                            record.metadata[OPEN_BOOK_GENRE_INDEX] = field;
                            break;
                        case 1129530692: // DESC
                            record.metadata[OPEN_BOOK_DESCRIPTION_INDEX] = field;
                            break;
                        case 1196310860: // LANG
                            record.metadata[OPEN_BOOK_LANGUAGE_INDEX] = field;
                            break;
                        default:
                            break;
                    }            
                }
            } else if (this->_fileIsTxt(entry)) {
                // if it's just a text file, use the first line as the title.
                record.fileSize = entry.size();
                record.currentPosition = 0; // TODO: copy from map

                BookField field;
                field.tag = 1280592212; // TITL
                field.loc = 0;
                // up to 32 characters
                field.len = min(record.fileSize, 32);
                entry.seekSet(0);
                for(int i = 0; i < field.len; i++) {
                    // but truncate it at the first newline
                    char c = entry.read();
                    if ((c == '\r') || c == '\n') field.len = i;
                }
                record.metadata[OPEN_BOOK_TITLE_INDEX] = field;
            }
            entry.close();
            temp = device->openFile(OPEN_BOOK_WORKING_FILENAME, O_RDWR | O_AT_END);
            temp.write((byte *)&record, sizeof(BookRecord));
            temp.flush();
            temp.close();
        }
        entry = root.openNextFile();
    }

    device->renameFile(OPEN_BOOK_LIBRARY_FILENAME, OPEN_BOOK_BACKUP_FILENAME);
    device->renameFile(OPEN_BOOK_WORKING_FILENAME, OPEN_BOOK_LIBRARY_FILENAME);
    device->removeFile(OPEN_BOOK_BACKUP_FILENAME);
    
    return true;
}

uint32_t OpenBookDatabase::getNumberOfBooks() {
    return this->numBooks;
}

BookRecord OpenBookDatabase::getBookRecord(uint32_t i) {
    BookRecord retval;
    File database = OpenBookDevice::sharedDevice()->openFile(OPEN_BOOK_LIBRARY_FILENAME);

    database.seekSet(sizeof(DATABASE_FILE_IDENTIFIER) + sizeof(BookDatabaseHeader) + i * sizeof(BookRecord));
    database.read((byte *)&retval, sizeof(BookRecord));
    database.close();

    return retval;
}

std::string OpenBookDatabase::getBookTitle(BookRecord record) {
    return this->_getMetadataAtIndex(record, OPEN_BOOK_TITLE_INDEX);
}

std::string OpenBookDatabase::getBookAuthor(BookRecord record) {
    return this->_getMetadataAtIndex(record, OPEN_BOOK_AUTHOR_INDEX);
}

std::string OpenBookDatabase::getBookDescription(BookRecord record) {
    return this->_getMetadataAtIndex(record, OPEN_BOOK_DESCRIPTION_INDEX);
}

uint32_t OpenBookDatabase::getCurrentPage(BookRecord record) {
    uint32_t retval = 0;
    std::string filename = std::string(record.filename);

    filename.replace(strlen(record.filename) - 3, 3, "obp");
    if (OpenBookDevice::sharedDevice()->fileExists(filename.c_str())) {
        File f = OpenBookDevice::sharedDevice()->openFile(filename.c_str());
        f.read((void *)&retval, 4);
        f.close();
    }

    return retval;
}

void OpenBookDatabase::setCurrentPage(BookRecord record, uint32_t page) {
    std::string filename = std::string(record.filename);
    filename.replace(strlen(record.filename) - 3, 3, "obp");
    File f = OpenBookDevice::sharedDevice()->openFile(filename.c_str(), O_CREAT | O_WRITE | O_TRUNC);
    f.write(&page, 4);
    f.close();
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
    char paginationFilename[128];
    return this->_getPaginationFile(record, paginationFilename);
}

/**
 * Paginates a provided BookRecord by ... 
 *  and outputing a pagination file in the same directory the BookRecord is found in.
 *  The paginations file is a .pag binary file containing a header of metadata,
 *   
 * 
 * @param record The BookRecord to be paginated
*/
void OpenBookDatabase::paginateBook(BookRecord record) {
    OpenBookDevice *device = OpenBookDevice::sharedDevice();
    File paginationFile;
    char paginationFilename[128];

    if (this->_getPaginationFile(record, paginationFilename)) { // If the pagination file for the BookRecord exists
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
        if (bookFile.read() == 0x1e) {
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
    const int16_t pageHeight = 320;
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

        if (codepoints[0] == 0x1e) {
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
                if (utf8bytes[i] == 0x20) bytePosition++;
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
    char paginationFilename[128];
    if (this->_getPaginationFile(record, paginationFilename)) {
        BookPaginationHeader header;
        File f = OpenBookDevice::sharedDevice()->openFile(paginationFilename);
        f.read(&header, sizeof(BookPaginationHeader));
        return header.numPages;
    }

    return 0;
}

std::string OpenBookDatabase::getTextForPage(BookRecord record, uint32_t page) {
    char paginationFilename[128];

    if (this->_getPaginationFile(record, paginationFilename)) {
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
 * This method takes in a BookRecord and using it's filename, copies it to the output filename
 *  then finds the byte location of four less than the record's filename's string length
 *  and appends the extension .pag to it.
 * Then returning whether or not that filename exists on the device or not.
 * 
 * @param  record The BookRecord object you're attempting to get the pagination file for
 * @param  outFilename A pointer to the char array for the pagination filename
 * @return A boolean of if the file already exists or not
*/
bool OpenBookDatabase::_getPaginationFile(BookRecord record, char *outFilename) {
    const uint32_t extension = 1734438958; // Four ASCII characters, .pag

    memcpy(outFilename, record.filename, 128); // Copy record's filename to the output filename variable
    memcpy((byte *)outFilename + (strlen(outFilename) - 4), (byte *)&extension, sizeof(extension)); // 

    return OpenBookDevice::sharedDevice()->fileExists(outFilename);
}

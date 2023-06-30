#ifndef I18n_h
#define I18n_h

#include <map>
#include <string>
#include <vector>
#include "Config.h"

// The current languages supported and configured
const std::vector<arduino::String> LANGUAGES { "en", "ia" };

typedef struct {
    std::string yes     = "Yes";
    std::string no      = "No";
    std::string open    = "Open";
    std::string close   = "Close";
    std::string by      = " by ";
    std::string of      = " of ";
} CommonMessages;     // Common messages, to be used anywhere in the application

typedef struct {
    std::string myLibrary   = "My Library";
    std::string pagination  = " is not paginated.\n\nPaginate it now?";
} BookListMessages;   // Unique Messages to the BookListViewController

typedef struct {
    std::string goToPage    = "Go to page ";
} BookReaderMessages; // Unique Messages to the BookReaderViewController 

typedef struct {
    CommonMessages     common;
    BookListMessages   bookList;
    BookReaderMessages bookReader;
} Messages;

static std::map<arduino::String, Messages> MESSAGES = {
    { "en", Messages() },
    { "ia",
        Messages { 
            CommonMessages {
                "Esyay",            // yes
                "Onay",             // no
                "Enopay",           // open
                "Ose-Clay",         // close
                " byay ",           // by
                " ofay "            // of
            },
            BookListMessages {
                "Myay Ibrary-lay", // myLibrary
                " isay otnay aginated-pay.\n\nAginate-pay itay ownay?" // pagination
            },
            BookReaderMessages {
                "Ogay otay agepay " // toToPage
            }
        }
    }
};

class I18n {
public:
    static I18n *i18n() {
        static I18n instance;
        return &instance;
    }

    /**
     * Used to get the currently configured langauge's message set
     * @return A Messages struct for the current langage
    */
    static Messages get() {
        return MESSAGES[Config::I18N_TAG()];
    }
private:
    I18n();
};

#endif // I18n_h
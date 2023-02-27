#ifndef BookListViewController_h
#define BookListViewController_h

#include <string>
#include <vector>
#include "Focus.h"
#include "FocusWidgets.h"
#include "OpenBookWidgets.h"
#include "OpenBookDatabase.h"

class BookListViewController : public ViewController {
public:
    BookListViewController(std::shared_ptr<Application> application);
    virtual void viewWillAppear() override;

    void selectBook(Event event);
    void dismiss(Event event);
    void paginate(Event event);
    void updateBatteryIcon(Event event = {0});
    void nextPage(Event event);
    void previousPage(Event event);
protected:
    virtual void createView() override;
    uint16_t numberOfBooks = 0;
    uint16_t numberOfPages = 0;
    uint16_t currentPage = 0;
    BookRecord currentBook = {0};
    std::string paginationLabel = "";
    int16_t paginationLabelXPos = 145;

    std::shared_ptr<OpenBookTable> table;
    std::shared_ptr<BorderedView> modal;
    std::shared_ptr<BitmapView> batteryIcon;
    std::shared_ptr<BitmapView> usbIcon;
    std::shared_ptr<Label> voltageLabel;
    std::shared_ptr<Label> pagination;

    void _updateView();
    void _updatePagination();
};

#endif // BookListViewController_h

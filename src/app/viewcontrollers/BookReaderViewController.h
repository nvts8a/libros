#ifndef BookReaderViewController_h
#define BookReaderViewController_h

#include "Application.h"
#include "Widgets.h"
#include "OpenBookDatabase.h"

const int PROGRESS_BAR_YPOS     = 400 - 4;
const int PROGRESS_LABEL_YPOS   = 400 - 8 - 4;

class BookReaderViewController : public ViewController {
public:
    BookReaderViewController(std::shared_ptr<Application> application, BookRecord book);
    virtual void viewWillAppear() override;

    void nextPage(Event event);
    void previousPage(Event event);
    void returnHome(Event event);
    void saveProgress(Event event);
    void showMenu(Event event);
    
    void handleModal(Event event);

protected:
    virtual void createView() override;
    void _updateView();
    void _getProgressLabelDetails();

    BookRecord book = {0};
    int32_t currentPage = 0;
    int32_t numPages = 1;
    int16_t progressLabelXPos = 0;
    float progressPercentage = 0.0;
    std::string progressText = "";

    std::shared_ptr<Control> eventReceiver;
    std::shared_ptr<ProgressView> progressView;
    std::shared_ptr<Label> progressLabel;
    std::shared_ptr<TypesetterLabel> bookText;

    std::shared_ptr<Control> modal;
    std::shared_ptr<ProgressView> modalSlider;
    std::shared_ptr<TypesetterLabel> gotoPageLabel;
};

#endif // BookReaderViewController_h

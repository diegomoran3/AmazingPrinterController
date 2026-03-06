#include <wx/wx.h>
#include "MainFrame.hpp"

class MyApp : public wxApp
{
public:
    virtual bool OnInit() override
    {
        MainFrame* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

// This macro creates the main() entry point
wxIMPLEMENT_APP(MyApp);

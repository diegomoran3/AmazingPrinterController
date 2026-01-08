#include <wx/wx.h>      // Required for wxApp and wxWidgets macros
#include "MainFrame.h"  // Required so the compiler knows what a "MainFrame" is

class MyApp : public wxApp
{
public:
    virtual bool OnInit() override
    {
        // Now the compiler knows what MainFrame is
        MainFrame* frame = new MainFrame();
        frame->Show(true);
        return true;
    }
};

// This macro creates the main() entry point
wxIMPLEMENT_APP(MyApp);

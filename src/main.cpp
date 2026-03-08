#include <wx/wx.h>
#include "MainFrame.hpp"
#include "JsonHandler.hpp"

class MyApp : public wxApp
{
public:
    virtual bool OnInit() override
    {
        m_configHandler.load<AppSettings>(m_settings);
        MainFrame* frame = new MainFrame(&m_settings);
        frame->Show(true);
        return true;
    }

    virtual int OnExit() override
    {
        m_configHandler.save<AppSettings>(m_settings);
        return 0;
    }
private:
    JsonHandler m_configHandler;
    AppSettings m_settings;
};

// This macro creates the main() entry point
wxIMPLEMENT_APP(MyApp);

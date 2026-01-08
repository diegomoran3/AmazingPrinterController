#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <memory>
#include "SerialPortManager.h"
#include <functional>

class MainFrame : public wxFrame
{
public:
    MainFrame();

private:
    // Event Handlers
    void OnConnect(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void UpdatePortList();
    void OnSend(wxCommandEvent& event);

    // UI Components
    wxComboBox* m_portCombo;
    wxButton* m_connectBtn;
    wxButton* m_refreshBtn;
    wxTextCtrl* m_logCtrl;

    wxTextCtrl* m_cmdInput;
    wxButton* m_sendBtn;

    // Logic
    std::unique_ptr<SerialPortManager> m_serialManager;

    // Event IDs
    enum {
        ID_CONNECT = 1,
        ID_REFRESH = 2,
        ID_SEND = 3
    };

    wxDECLARE_EVENT_TABLE();
};

#endif // MAINFRAME_H

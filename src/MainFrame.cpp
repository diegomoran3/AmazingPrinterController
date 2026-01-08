#include "MainFrame.h"

// Event Table
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_CONNECT, MainFrame::OnConnect)
    EVT_BUTTON(ID_REFRESH, MainFrame::OnRefresh)
    EVT_BUTTON(ID_SEND, MainFrame::OnSend)
    EVT_TEXT_ENTER(ID_SEND, MainFrame::OnSend)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(NULL, wxID_ANY, "Amazing Printer Controller", wxDefaultPosition, wxSize(600, 500)),
      m_serialManager(std::make_unique<SerialPortManager>())
{
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // --- PORT SELECTION AREA ---
    wxStaticBoxSizer* portSizer = new wxStaticBoxSizer(wxHORIZONTAL, panel, "Connection Settings");

    m_portCombo = new wxComboBox(portSizer->GetStaticBox(), wxID_ANY, "",
                                 wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);

    m_refreshBtn = new wxButton(portSizer->GetStaticBox(), ID_REFRESH, "Refresh");
    m_connectBtn = new wxButton(portSizer->GetStaticBox(), ID_CONNECT, "Connect");

    portSizer->Add(m_portCombo, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_refreshBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_connectBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    // --- LOG AREA ---
    m_logCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    m_logCtrl->SetBackgroundColour(wxColour(30, 30, 30));
    m_logCtrl->SetForegroundColour(wxColour(0, 255, 0));

    // Set up standard logging to redirect to this box
    wxLog::SetActiveTarget(new wxLogTextCtrl(m_logCtrl));

    wxBoxSizer* cmdSizer = new wxBoxSizer(wxHORIZONTAL);

    // Create text input with TE_PROCESS_ENTER so pressing 'Enter' triggers the event
    m_cmdInput = new wxTextCtrl(panel, ID_SEND, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_sendBtn = new wxButton(panel, ID_SEND, "Send");

    cmdSizer->Add(m_cmdInput, 1, wxEXPAND | wxRIGHT, 5);
    cmdSizer->Add(m_sendBtn, 0, wxEXPAND);

    // Combine everything
    mainSizer->Add(portSizer, 0, wxEXPAND | wxALL, 10);
    mainSizer->Add(new wxStaticText(panel, wxID_ANY, "Console Output:"), 0, wxLEFT, 10);
    mainSizer->Add(m_logCtrl, 1, wxEXPAND | wxALL, 10);
    mainSizer->Add(cmdSizer, 0, wxEXPAND | wxALL, 10);

    panel->SetSizer(mainSizer);

    // Initial Port Scan
    UpdatePortList();

    Centre();
}

void MainFrame::UpdatePortList() {
    m_portCombo->Clear();
    std::vector<std::string> ports = m_serialManager->ScanPorts();

    for (const auto& port : ports) {
        m_portCombo->Append(port);
    }

    if (!ports.empty()) {
        m_portCombo->SetSelection(0);
        wxLogMessage("Found %zu serial ports.", ports.size());
    } else {
        wxLogMessage("No serial ports found. Check connections.");
    }
}

void MainFrame::OnRefresh(wxCommandEvent& event) {
    UpdatePortList();
}

void MainFrame::OnConnect(wxCommandEvent& event) {
    if (m_serialManager->IsOpen()) {
        m_serialManager->ClosePort();
        m_connectBtn->SetLabel("Connect");
        m_portCombo->Enable(true);
        wxLogMessage("Disconnected.");
    } else {
        wxString selectedPort = m_portCombo->GetStringSelection();
        if (selectedPort.IsEmpty()) {
            wxLogError("Please select a port first!");
            return;
        }

        // Try to open at 115200 baud (standard for GRBL)
        if (m_serialManager->OpenPort(selectedPort.ToStdString(), 115200)) {
            m_connectBtn->SetLabel("Disconnect");
            m_portCombo->Enable(false);
            wxLogMessage("Successfully connected to %s", selectedPort);

            // Start listening. The lambda is called in a BACKGROUND thread.
            m_serialManager->StartAsyncRead([this](const std::string& line) {

                // We use CallAfter to send this 'job' to the Main GUI Thread.
                // This ensures wxLogMessage is called safely.
                this->CallAfter([this, line]() {
                    // This code now runs on the main thread
                    wxLogMessage("RX: %s", line);
                });
            });

        } else {
            wxLogError("Failed to open port %s", selectedPort);
        }
    }
}

void MainFrame::OnSend(wxCommandEvent& event) {
    if (!m_serialManager->IsOpen()) {
        wxLogError("Not connected to any port!");
        return;
    }

    wxString cmd = m_cmdInput->GetValue();
    if (cmd.IsEmpty()) return; // Don't send empty strings

    // Convert to std::string
    std::string commandToSend = cmd.ToStdString();

    // Log what we are sending (TX)
    wxLogMessage("TX: %s", commandToSend);

    // CRITICAL: GRBL/Printers usually need a newline character to process the command
    commandToSend += "\n";

    // Send the data
    if (m_serialManager->Write(commandToSend)) {
        // Clear input box only on success
        m_cmdInput->Clear();
    } else {
        wxLogError("Failed to send command.");
    }
}

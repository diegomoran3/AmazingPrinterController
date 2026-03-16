#include "MainFrame.hpp"

// Event Table
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_CONNECT,          MainFrame::OnConnect)
    EVT_BUTTON(ID_REFRESH,          MainFrame::OnRefresh)
    EVT_BUTTON(ID_SEND,             MainFrame::OnSend)
    EVT_TEXT_ENTER(ID_SEND,         MainFrame::OnSend)
    EVT_TOOL(ID_SETTINGS_TOOL,      MainFrame::OnOpenSettings)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(AppSettings* settings)
    : wxFrame(NULL, wxID_ANY, "Amazing Printer Controller", wxDefaultPosition, wxSize(600, 500)),
      m_grbl(std::make_shared<GrblController>()),
        m_settings(settings)
{
    // --- Create Toolbar ---
    wxToolBar* toolbar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL);
    
    // Add a Settings Tool (You can use a custom icon, here using a standard one)
    toolbar->AddTool(ID_SETTINGS_TOOL, "GRBL Settings", 
                     wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_TOOLBAR));
                     
    toolbar->Realize();

    // 1. Create the Splitter Window as the main child of the Frame
    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3D);

    // 2. Create the Left and Right panels as children of the Splitter
    m_leftPanel = new wxPanel(m_splitter);
    m_rightPanel = new wxPanel(m_splitter);

    // 3. Build the internals of each panel
    BuildLeftPanel(m_leftPanel);
    BuildRightPanel(m_rightPanel);

    // 4. Configure the Splitter
    m_splitter->SetMinimumPaneSize(200); // Prevent dragging too far
    m_splitter->SplitVertically(m_leftPanel, m_rightPanel, 350); // 350px is the initial position of the divider

    // 5. Logic Setup (Same as before)
    SetupGrblCallbacks();
    UpdatePortList();
    Centre();
}

void MainFrame::BuildLeftPanel(wxPanel* parent)
{
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);

    // 1. Connection Box stays at the top (always visible)
    leftSizer->Add(CreateConnectionBox(parent), 0, wxEXPAND | wxALL, 5);

    // 2. Create Notebook for switching between Manual and Scan
    m_sidebarTabs = new wxNotebook(parent, wxID_ANY);
    
    // Create the control panel as a notebook page
    m_controlPanel = new GrblControlPanel(m_sidebarTabs, m_grbl, &m_settings->jogSettings);

    m_scanPanel = new GrblScanPanel(
        m_sidebarTabs,
        std::make_shared<ScanHandler>(m_grbl),
        [this](const std::vector<ScanLine>& scanLines) { this->SetScanGridPreview(scanLines); },
        &m_settings->lastUsedPattern
    );

    m_settingsPanel = new AppSettingsPanel(
        m_sidebarTabs,
        m_settings,
        [this]() { this->ApplySettings(); }
    );

    m_sidebarTabs->AddPage(m_controlPanel, "Manual");
    m_sidebarTabs->AddPage(m_scanPanel, "Scanner");
    m_sidebarTabs->AddPage(m_settingsPanel, "Settings");

    leftSizer->Add(m_sidebarTabs, 0, wxEXPAND | wxALL, 5);

    // 3. Console stays at the bottom
    SetupLogArea(parent, leftSizer);

    parent->SetSizer(leftSizer);
}

void MainFrame::BuildRightPanel(wxPanel* parent)
{
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    rightSizer->Add(new wxStaticText(parent, wxID_ANY, "Coordinate System:"), 0, wxLEFT | wxTOP, 10);

    // CoordinatePanel now lives inside the right panel
    m_coordPanel = new CoordinatePanel(parent);

    m_coordPanel->SetOnPointClicked([this](double x, double y) {
        
        // Check connection first
        if (!m_grbl || !m_grbl->IsConnected()) {
            wxMessageBox("Connect to machine first!", "Error", wxICON_WARNING);
            return;
        }
        
        m_grbl->MoveTo(x, y);
        
        // Optional: Update the "Target" dot immediately for visual feedback
        // (The machine status update will eventually overwrite this)
        m_coordPanel->ClearPoints();
        m_coordPanel->AddPoint(x, y, *wxRED); 
    });
    
    rightSizer->Add(m_coordPanel, 1, wxEXPAND | wxALL, 10);

    parent->SetSizer(rightSizer); // Critical: Set the sizer for this specific panel
}

wxSizer* MainFrame::CreateConnectionBox(wxPanel* parent)
{
    wxStaticBoxSizer* portSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Connection Settings");

    m_portCombo = new wxComboBox(portSizer->GetStaticBox(), wxID_ANY, "",
                                 wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_READONLY);
    m_refreshBtn = new wxButton(portSizer->GetStaticBox(), ID_REFRESH, "Refresh");
    m_connectBtn = new wxButton(portSizer->GetStaticBox(), ID_CONNECT, "Connect");

    portSizer->Add(m_portCombo, 1, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_refreshBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    portSizer->Add(m_connectBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    return portSizer;
}

void MainFrame::SetupLogArea(wxPanel* parent, wxBoxSizer* mainVerticalSizer)
{
    // 1. Label
    mainVerticalSizer->Add(new wxStaticText(parent, wxID_ANY, "Console Output:"), 0, wxLEFT | wxTOP, 5);

    // 2. Log Window
    m_logCtrl = new wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                               wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    m_logCtrl->SetBackgroundColour(wxColour(30, 30, 30));
    m_logCtrl->SetForegroundColour(wxColour(0, 255, 0));
    
    // Redirect wxLog
    delete wxLog::SetActiveTarget(new wxLogTextCtrl(m_logCtrl));
    
    mainVerticalSizer->Add(m_logCtrl, 1, wxEXPAND | wxALL, 5);

    // 3. Command Input Area
    wxBoxSizer* cmdSizer = new wxBoxSizer(wxHORIZONTAL);
    m_cmdInput = new wxTextCtrl(parent, ID_SEND, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_sendBtn = new wxButton(parent, ID_SEND, "Send");

    cmdSizer->Add(m_cmdInput, 1, wxEXPAND | wxRIGHT, 5);
    cmdSizer->Add(m_sendBtn, 0, wxEXPAND);

    mainVerticalSizer->Add(cmdSizer, 0, wxEXPAND | wxALL, 5);
}

void MainFrame::SetupGrblCallbacks()
{
    // Keep backend logic separate from UI construction
    m_grbl->SetOnMessageReceived([this](const std::string& line) {
        this->CallAfter([this, line]() {
            wxLogMessage("RX: %s", line);

        bool isSetting = m_grbl->ParseSetting(line);

        // 2. ONLY update the window if it is actually open
        if (isSetting && m_configDlg) {
            m_configDlg->ReloadGrid(); 
        }
        });
    });

    m_grbl->SetOnStatusUpdate([this](const GrblStatus& status) {
        this->CallAfter([this, status]() {
            m_coordPanel->SetMachinePosition(status.x, status.y);
        });
    });
}

void MainFrame::UpdatePortList() {
    m_portCombo->Clear();
    auto ports = m_grbl->GetAvailablePorts();
    for (const auto& port : ports) m_portCombo->Append(port);

    if (!ports.empty()) m_portCombo->SetSelection(0);
}

void MainFrame::SetPreviewRegion(double x, double y, double width, double height)
{
    m_currentPreviewRegion = {x, y, width, height};
    if(m_coordPanel)
        m_coordPanel->DrawPreviewRegion(x, y, width, height);
}

void MainFrame::SetScanGridPreview(const std::vector<ScanLine>& scanLines)
{
    if (m_coordPanel) {
        if (m_settings->showPreview)
            m_coordPanel->SetScanGridPreview(scanLines);
        else
            m_coordPanel->ClearScanGridPreview();
    }
}

void MainFrame::ApplySettings()
{
    // Polling rate -> GrblController polling interval
    if (m_settings->statusPollRateHz > 0) {
        int intervalMs = 1000 / m_settings->statusPollRateHz;
        m_grbl->SetPollingInterval(intervalMs);
    }

    // Show/hide scan preview
    if (m_coordPanel) {
        if (m_settings->showPreview) {
            // Re-send the current scan grid so it reappears
            if (m_scanPanel)
                m_scanPanel->RefreshPreview();
        } else {
            m_coordPanel->ClearScanGridPreview();
            m_coordPanel->ClearPreviewRegion();
        }
    }
}

void MainFrame::OnOpenSettings(wxCommandEvent &event)
{
    // Create the dialog
    GrblConfigDialog dlg(this, m_grbl);
    
    // Register it so we can feed it data
    m_configDlg = &dlg;
    
    // Show it (Blocking / Modal)
    dlg.ShowModal();
    
    // Cleanup after it closes
    m_configDlg = nullptr;
}

void MainFrame::OnRefresh(wxCommandEvent& event) {
    UpdatePortList();
}

void MainFrame::OnConnect(wxCommandEvent& event) {
    if (m_grbl->IsConnected()) {
        m_grbl->Disconnect();
        m_connectBtn->SetLabel("Connect");
        m_portCombo->Enable(true);
        wxLogMessage("Disconnected.");                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
    } else {
        wxString selectedPort = m_portCombo->GetStringSelection();
        std::string portStd = selectedPort.ToStdString(); 

        m_connectBtn->SetLabel("Connecting...");
        m_connectBtn->Disable(); 
        m_portCombo->Enable(false);

        std::thread([this, portStd]() {
            bool success = m_grbl->Connect(portStd);
            
            wxTheApp->CallAfter([this, success, portStd]() {
                
                m_connectBtn->Enable();

                if (success) {
                    m_connectBtn->SetLabel("Disconnect");
                    wxLogMessage("Successfully connected to %s", portStd);

                    if (m_settings->autoHomeOnConnect) {
                        m_grbl->SendCommand(Grbl::Home);
                        wxLogMessage("Auto-homing ($H)...");
                    }
                } else {
                    m_connectBtn->SetLabel("Connect");
                    m_portCombo->Enable(true);
                    wxLogError("Failed to open port %s", portStd);
                }
            });

        }).detach();
    }
}

void MainFrame::OnSend(wxCommandEvent& event) {
    if (!m_grbl->IsConnected()) return;

    wxString cmd = m_cmdInput->GetValue();
    if (cmd.IsEmpty()) return;

    wxLogMessage("TX: %s", cmd);
    m_grbl->SendCommand(cmd.ToStdString());
    m_cmdInput->Clear();
}
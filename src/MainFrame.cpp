#include "MainFrame.h"

// Event Table
wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_CONNECT, MainFrame::OnConnect)
    EVT_BUTTON(ID_REFRESH, MainFrame::OnRefresh)
    EVT_BUTTON(ID_SEND, MainFrame::OnSend)
    EVT_TEXT_ENTER(ID_SEND, MainFrame::OnSend)
    EVT_BUTTON(ID_GOTO, MainFrame::OnGoTo)
    EVT_BUTTON(ID_HOME, MainFrame::OnHome)
    EVT_BUTTON(ID_UNLOCK, MainFrame::OnUnlock)
    EVT_BUTTON(ID_PAUSE, MainFrame::OnPause)
    EVT_BUTTON(ID_RESUME, MainFrame::OnResume)
    EVT_BUTTON(ID_RESET, MainFrame::OnReset)
    EVT_TOOL(ID_SETTINGS_TOOL, MainFrame::OnOpenSettings)
wxEND_EVENT_TABLE()

MainFrame::MainFrame()
    : wxFrame(NULL, wxID_ANY, "Amazing Printer Controller", wxDefaultPosition, wxSize(600, 500)),
      m_grbl(std::make_unique<GrblController>())
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

    // Use the specific 'parent' (m_leftPanel) for controls
    leftSizer->Add(CreateConnectionBox(parent), 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(CreateControlBox(parent), 0, wxEXPAND | wxALL, 5);
    leftSizer->Add(CreateMotionBox(parent), 0, wxEXPAND | wxALL, 5);
    
    // Add your Manual Controls here if you kept them
    // leftSizer->Add(CreateManualBox(parent), ...);

    SetupLogArea(parent, leftSizer);

    parent->SetSizer(leftSizer); // Critical: Set the sizer for this specific panel
}

void MainFrame::BuildRightPanel(wxPanel* parent)
{
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    rightSizer->Add(new wxStaticText(parent, wxID_ANY, "Coordinate System:"), 0, wxLEFT | wxTOP, 10);

    // CoordinatePanel now lives inside the right panel
    m_coordPanel = new CoordinatePanel(parent);
    m_coordPanel->SetMinSize(wxSize(400, 400));

    m_coordPanel->SetOnPointClicked([this](double x, double y) {
        
        // Check connection first
        if (!m_grbl || !m_grbl->IsConnected()) {
            wxMessageBox("Connect to machine first!", "Error", wxICON_WARNING);
            return;
        }

        // Send Rapid Move (G0) to the clicked location
        // Using "G90" ensures we are in Absolute Mode
        wxString cmd = wxString::Format("G90 G0 X%.3f Y%.3f", x, y);
        
        m_grbl->SendCommand(cmd.ToStdString());
        
        wxLogMessage("Interactive Move: %s", cmd);
        
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

wxSizer* MainFrame::CreateControlBox(wxPanel* parent)
{
    wxStaticBoxSizer* boxSizer = new wxStaticBoxSizer(wxVERTICAL, parent, "Machine Control");
    
    // Create a Grid Sizer: 2 columns, 5px gap between items
    wxGridSizer* grid = new wxGridSizer(2, 5, 5);

    // 1. Setup & Recovery
    grid->Add(new wxButton(boxSizer->GetStaticBox(), ID_HOME, "Home ($H)"), 1, wxEXPAND);
    grid->Add(new wxButton(boxSizer->GetStaticBox(), ID_UNLOCK, "Unlock ($X)"), 1, wxEXPAND);

    // 2. Flow Control
    grid->Add(new wxButton(boxSizer->GetStaticBox(), ID_PAUSE, "Hold (!)"), 1, wxEXPAND);
    grid->Add(new wxButton(boxSizer->GetStaticBox(), ID_RESUME, "Resume (~)"), 1, wxEXPAND);
    
    // 3. Emergency Stop (Full Width)
    wxButton* stopBtn = new wxButton(boxSizer->GetStaticBox(), ID_RESET, "SOFT RESET");

    stopBtn->SetBackgroundColour(wxColour(200, 50, 50)); // Red warning color
    stopBtn->SetForegroundColour(*wxWHITE);
    
    // Add grid to the static box
    boxSizer->Add(grid, 1, wxEXPAND | wxALL, 5);
    
    // Add Stop button separately at the bottom
    boxSizer->Add(stopBtn, 0, wxEXPAND | wxALL, 5);

    return boxSizer;
}

wxSizer* MainFrame::CreateMotionBox(wxPanel* parent)
{
    wxStaticBoxSizer* coordInputSizer = new wxStaticBoxSizer(wxHORIZONTAL, parent, "Go To Position");

    m_xInput = new wxTextCtrl(coordInputSizer->GetStaticBox(), wxID_ANY, "0");
    m_yInput = new wxTextCtrl(coordInputSizer->GetStaticBox(), wxID_ANY, "0");
    m_gotoBtn = new wxButton(coordInputSizer->GetStaticBox(), ID_GOTO, "Go To");

    coordInputSizer->Add(new wxStaticText(coordInputSizer->GetStaticBox(), wxID_ANY, "X:"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
    coordInputSizer->Add(m_xInput, 1, wxALL, 5);
    coordInputSizer->Add(new wxStaticText(coordInputSizer->GetStaticBox(), wxID_ANY, "Y:"), 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
    coordInputSizer->Add(m_yInput, 1, wxALL, 5);
    coordInputSizer->Add(m_gotoBtn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);

    return coordInputSizer;
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
            m_coordPanel->ClearPoints();
            m_coordPanel->AddPoint(status.x, status.y, *wxBLUE);
        });
    });
}

void MainFrame::UpdatePortList() {
    m_portCombo->Clear();
    auto ports = m_grbl->GetAvailablePorts();
    for (const auto& port : ports) m_portCombo->Append(port);

    if (!ports.empty()) m_portCombo->SetSelection(0);
}

void MainFrame::OnOpenSettings(wxCommandEvent& event) {
    if (!m_grbl->IsConnected()) {
        wxMessageBox("Please connect to the machine first.", "Error", wxICON_ERROR);
        return;
    }

    // Create the dialog
    GrblConfigDialog dlg(this, m_grbl.get());
    
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

void MainFrame::OnGoTo(wxCommandEvent& event) {
    double x, y;
    if (!m_xInput->GetValue().ToDouble(&x) || !m_yInput->GetValue().ToDouble(&y)) {
        wxLogError("Invalid coordinates!");
        return;
    }

    // Abstracted logic: we just say MoveTo, we don't care about "G0" or "\n"
    m_grbl->MoveTo(x, y);
    wxLogMessage("Moving to X:%.2f Y:%.2f", x, y);
}

void MainFrame::OnHome(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendCommand(Grbl::Home); 
        wxLogMessage("Sent Homing Command ($H)");
    }
}

void MainFrame::OnUnlock(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendCommand(Grbl::Unlock);
        wxLogMessage("Sent Unlock Command ($X)");
    }
}

void MainFrame::OnPause(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::FeedHold); 
        wxLogMessage("Sent Feed Hold (!)");
    }
}

void MainFrame::OnResume(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::CycleStart); 
        wxLogMessage("Sent Cycle Start (~)");
    }
}

void MainFrame::OnReset(wxCommandEvent& event) {
    if(m_grbl && m_grbl->IsConnected()) {
        m_grbl->SendRealtimeCommand(Grbl::SoftReset);
        wxLogMessage("Soft Reset Sent!");
    }
}

#include "GrblScanWindow.h"

// IDs for events
enum {
    ID_BTN_START = 1001,
    ID_BTN_CLOSE = 1002
};

wxBEGIN_EVENT_TABLE(GrblScanWindow, wxDialog)
    EVT_BUTTON(ID_BTN_START, GrblScanWindow::OnStart)
    EVT_BUTTON(ID_BTN_CLOSE, GrblScanWindow::OnBtnClose)
    EVT_CLOSE(GrblScanWindow::OnClose)
wxEND_EVENT_TABLE()

GrblScanWindow::GrblScanWindow(wxWindow* parent, GrblController* controller)
    : wxDialog(parent, wxID_ANY, "Scanner Configuration", wxDefaultPosition, wxSize(350, 450)),
      m_controller(controller)
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* formSizer = new wxFlexGridSizer(2, 5, 10); // 2 cols, 5px hgap, 10px vgap
    formSizer->AddGrowableCol(1, 1);

    // --- Input Helper ---
    auto AddInput = [&](const wxString& label, wxTextCtrl*& ptr, const wxString& def) {
        formSizer->Add(new wxStaticText(this, wxID_ANY, label), 0, wxALIGN_CENTER_VERTICAL);
        ptr = new wxTextCtrl(this, wxID_ANY, def);
        formSizer->Add(ptr, 1, wxEXPAND);
    };

    // 1. Create Fields
    AddInput("Start X:", m_txtStartX, "0.0");
    AddInput("Start Y:", m_txtStartY, "0.0");
    AddInput("Rows:",    m_txtRows,   "5");
    AddInput("Cols:",    m_txtCols,   "5");
    AddInput("Step X:",  m_txtStepX,  "10.0");
    AddInput("Step Y:",  m_txtStepY,  "10.0");
    AddInput("Speed:",   m_txtSpeed,  "1000.0");

    // 2. Direction
    wxString choices[] = { "Horizontal", "Vertical" };
    m_rbDirection = new wxRadioBox(this, wxID_ANY, "Direction", wxDefaultPosition, wxDefaultSize, 2, choices);

    m_chkZigzag = new wxCheckBox(this, wxID_ANY, "Zigzag Mode (Snake Scan)");
    m_chkZigzag->SetValue(true); // Default to on, it's faster

    // 4. Buttons
    auto* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnStart = new wxButton(this, ID_BTN_START, "Start Scan");
    m_btnClose = new wxButton(this, ID_BTN_CLOSE, "Close");
    btnSizer->Add(m_btnStart, 1, wxRIGHT, 5);
    btnSizer->Add(m_btnClose, 0);

    // Layout
    mainSizer->Add(formSizer, 1, wxALL | wxEXPAND, 15);
    mainSizer->Add(m_rbDirection, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 15);
    mainSizer->Add(m_chkZigzag, 0, wxALL | wxEXPAND, 15);
    mainSizer->Add(btnSizer, 0, wxALL | wxALIGN_RIGHT, 15);

    SetSizer(mainSizer);
    Layout();
    CenterOnParent();
}

GrblScanWindow::~GrblScanWindow() {
    // Ensure thread is joined if window is destroyed
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void GrblScanWindow::ToggleControls(bool enable) {
    m_txtStartX->Enable(enable);
    m_txtStartY->Enable(enable);
    m_txtRows->Enable(enable);
    m_txtCols->Enable(enable);
    m_txtStepX->Enable(enable);
    m_txtStepY->Enable(enable);
    m_txtSpeed->Enable(enable);
    m_rbDirection->Enable(enable);
    m_chkZigzag->Enable(enable);

    if (enable) {
        m_btnStart->Enable(true);
    }
}

void GrblScanWindow::OnStart(wxCommandEvent& event) {
    if (m_isScanning) {
        m_controller->CancelScan();
        m_btnStart->Disable();
        m_btnStart->SetLabel("Stopping...");
        return;
    }

    try {
        double startX = std::stod(m_txtStartX->GetValue().ToStdString());
        double startY = std::stod(m_txtStartY->GetValue().ToStdString());
        int rows = std::stoi(m_txtRows->GetValue().ToStdString());
        int cols = std::stoi(m_txtCols->GetValue().ToStdString());
        double stepX = std::stod(m_txtStepX->GetValue().ToStdString());
        double stepY = std::stod(m_txtStepY->GetValue().ToStdString());
        double speed = std::stod(m_txtSpeed->GetValue().ToStdString());
        Direction dir = (m_rbDirection->GetSelection() == 0) ? DIR_Horizontal : DIR_Vertical;
        bool zigzag = m_chkZigzag->GetValue();
        
        m_isScanning = true;
        ToggleControls(false); // Disable inputs
        
        m_btnStart->Enable(true); 
        m_btnStart->SetLabel("STOP SCAN"); // Change label
        
        Layout();

        m_workerThread = std::thread([=, this]() {
            
            // This now respects the m_shouldCancel flag
            m_controller->StartScanCycle(startX, startY, rows, cols, stepX, stepY, 
                [](int r, int c, double x, double y) {}, 
                dir, zigzag, speed
            );

            // Update UI when finished (or cancelled)
            this->CallAfter([this]() {
                m_isScanning = false;
                
                // Restore UI state
                m_btnStart->SetLabel("Start Scan");
                ToggleControls(true); // Re-enables inputs and buttons
                Layout();
                
                // Optional: Check if it was a real finish or a cancel
                // (You'd need a getter for m_shouldCancel if you want different messages)
                wxMessageBox("Scan cycle ended.", "Info");
            });
        });

    } catch (...) {
        wxMessageBox("Invalid input", "Error");
    }
}

void GrblScanWindow::OnClose(wxCloseEvent& event) {
    if (m_isScanning) {
        if (event.CanVeto()) {
            wxMessageBox("Cannot close while scanning is in progress.", "Busy");
            event.Veto();
            return;
        }
    }
    // Proceed with destruction
    Destroy();
}

void GrblScanWindow::OnBtnClose(wxCommandEvent& event) {
    Close(); // Triggers OnClose
}
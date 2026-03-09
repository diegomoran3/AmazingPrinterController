#include "GrblScanWindow.hpp"

// IDs for events
enum {
    ID_BTN_START = 1001,
    ID_BTN_CLOSE = 1002
};

wxBEGIN_EVENT_TABLE(GrblScanWindow, wxPanel)
    EVT_BUTTON(ID_BTN_START, GrblScanWindow::OnStart)
wxEND_EVENT_TABLE()

GrblScanWindow::GrblScanWindow(wxWindow* parent, std::shared_ptr<ScanHandler> controller, PreviewCallback onPreviewUpdate, GridPatternSettings* initialSettings)
    : wxPanel(parent, wxID_ANY),
      m_controller(controller),
      OnPreviewUpdate(onPreviewUpdate),
      m_settings(initialSettings)
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* formSizer = new wxFlexGridSizer(0, 2, 5, 10); 
    formSizer->AddGrowableCol(1, 1);

    // 1. Create Fields
    wxHelpers::AddInputDouble(this, formSizer, "Start X:", m_txtStartX, &m_settings->startX, [this]() { this->OnUIChange(); }, -400);
    wxHelpers::AddInputDouble(this, formSizer, "Start Y:", m_txtStartY, &m_settings->startY, [this]() { this->OnUIChange(); }, -380);
    wxHelpers::AddInputInt(this, formSizer, "Rows:",    m_txtRows,   &m_settings->rows, [this]() { this->OnUIChange(); }, -1);
    wxHelpers::AddInputInt(this, formSizer, "Cols:",    m_txtCols,   &m_settings->cols, [this]() { this->OnUIChange(); }, 1);
    wxHelpers::AddInputDouble(this, formSizer, "Step X:",  m_txtStepX,  &m_settings->stepX, [this]() { this->OnUIChange(); }, 0.1);
    wxHelpers::AddInputDouble(this, formSizer, "Step Y:",  m_txtStepY,  &m_settings->stepY, [this]() { this->OnUIChange(); }, 0.1);
    wxHelpers::AddInputInt(this, formSizer, "Speed:",   m_txtSpeed,  &m_settings->speed, [this]() { this->OnUIChange(); }, 1);

    // 2. Direction
    wxString choices[] = { "Horizontal", "Vertical" };
    m_rbDirection = new wxRadioBox(this, wxID_ANY, "Direction", wxDefaultPosition, wxDefaultSize, 2, choices, 1, wxRA_SPECIFY_ROWS,
                                   wxGenericValidator((int*)&m_settings->direction));


    wxHelpers::AddCheckBox(this, formSizer, "Zigzag Mode (Snake Scan)", m_chkZigzag, &m_settings->isZigzag, [this]() { this->OnUIChange(); });

    m_rbDirection->Bind(wxEVT_RADIOBOX, [this](wxCommandEvent&) { this->OnUIChange(); });

    // 4. Buttons
    auto* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    m_btnStart = new wxButton(this, ID_BTN_START, "Start Scan");
    btnSizer->Add(m_btnStart, 1, wxRIGHT, 5);

    // Layout
    mainSizer->Add(formSizer, 0, wxALL | wxEXPAND, 15);
    mainSizer->Add(m_rbDirection, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 15);
    mainSizer->Add(btnSizer, 0, wxALL | wxALIGN_RIGHT, 15);

    SetSizer(mainSizer);
    SetSettings(*m_settings);

    Layout();
}

GrblScanWindow::~GrblScanWindow() {
    if (m_isScanning) {
        m_controller->CancelScan();
    }
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

void GrblScanWindow::OnUIChange()
{
    if (this->TransferDataFromWindow()) 
    {        
        m_btnStart->Enable(true);

    if (OnPreviewUpdate) {
            double width = (m_settings->cols - 1) * m_settings->stepX;
            double height = (m_settings->rows - 1) * m_settings->stepY;
            
            OnPreviewUpdate(m_settings->startX, m_settings->startY, width, height);
        }
    } 
    else 
    {
        m_btnStart->Enable(false);
    }
}

void GrblScanWindow::SetSettings(GridPatternSettings &pattern)
{
    m_settings = &pattern;

    m_chkZigzag->SetValue(m_settings->isZigzag);
    m_rbDirection->SetSelection((int)m_settings->direction);
    TransferDataToWindow();
}

void GrblScanWindow::OnStart(wxCommandEvent &event)
{
    if (m_isScanning) {
        m_controller->CancelScan();
        m_btnStart->Disable();
        m_btnStart->SetLabel("Stopping...");
        return;
    }

    try {

        if (m_workerThread.joinable()) {
            m_workerThread.join();
        }
        
        m_isScanning = true;
        ToggleControls(false); // Disable inputs
        
        m_btnStart->Enable(true); 
        m_btnStart->SetLabel("STOP SCAN"); // Change label
        
        Layout();

        m_workerThread = std::thread([=, this]() {

            auto scanLines = m_controller->CreateScanLines(m_settings->rows, m_settings->cols, m_settings->startX, m_settings->startY, 
                m_settings->stepX, m_settings->stepY, m_settings->direction, m_settings->isZigzag);
            
            m_controller->StartScanCycle(scanLines,
                [](int r, int c, double x, double y) {
                    //Jonathan: TODO for point reached callback
                }, 
                m_settings->speed
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
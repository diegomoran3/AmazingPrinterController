#include "GrblConfigDialog.h"

enum { ID_GRID_REFRESH = 100, ID_GRID_SAVE };

wxBEGIN_EVENT_TABLE(GrblConfigDialog, wxDialog)
    EVT_BUTTON(ID_GRID_REFRESH, GrblConfigDialog::OnRefresh)
    EVT_BUTTON(ID_GRID_SAVE, GrblConfigDialog::OnSave)
    EVT_BUTTON(wxID_CLOSE, GrblConfigDialog::OnClose)
wxEND_EVENT_TABLE()

GrblConfigDialog::GrblConfigDialog(wxWindow* parent, GrblController* controller)
    : wxDialog(parent, wxID_ANY, "GRBL Configuration", wxDefaultPosition, wxSize(500, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER),
      m_controller(controller)
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // 1. The Grid
    m_grid = new wxGrid(this, wxID_ANY);
    m_grid->CreateGrid(0, 3); // 0 rows to start, 3 columns
    m_grid->SetColLabelValue(0, "ID");
    m_grid->SetColLabelValue(1, "Value");
    m_grid->SetColLabelValue(2, "Description");
    
    // Formatting
    m_grid->SetColSize(0, 50);
    m_grid->SetColSize(1, 100);
    m_grid->SetColSize(2, 300);
    m_grid->SetRowLabelSize(0); // Hide row numbers
    
    mainSizer->Add(m_grid, 1, wxEXPAND | wxALL, 10);

    // 2. Buttons
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->Add(new wxButton(this, ID_GRID_REFRESH, "Read Settings ($$)"), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, ID_GRID_SAVE, "Write Changes"), 0, wxALL, 5);
    btnSizer->Add(new wxButton(this, wxID_CANCEL, "Close"), 0, wxALL, 5);

    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 5);

    SetSizer(mainSizer);
    
    // Auto-load settings on open
    if(m_controller->IsConnected()) {
        m_controller->SendCommand("$$");
    }
}

void GrblConfigDialog::ParseLine(const std::string& line) {
    // Expected format: $100=250.000 (x, step/mm)
    if (line.empty() || line[0] != '$') return;

    // Find the '=' and the comment parenthesis
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) return;

    std::string id = line.substr(0, eqPos);
    
    // Extract Value and Description
    std::string value, desc;
    size_t parenPos = line.find('(');
    
    if (parenPos != std::string::npos) {
        value = line.substr(eqPos + 1, parenPos - eqPos - 1);
        // Remove trailing space from value
        while(!value.empty() && isspace(value.back())) value.pop_back();
        
        desc = line.substr(parenPos); // Keep the parenthesis
    } else {
        value = line.substr(eqPos + 1);
    }

    // Add to Grid
    m_grid->AppendRows(1);
    int row = m_grid->GetNumberRows() - 1;
    m_grid->SetCellValue(row, 0, id);
    m_grid->SetCellValue(row, 1, value);
    m_grid->SetCellValue(row, 2, desc);
    
    // Make ID and Desc read-only
    m_grid->SetReadOnly(row, 0);
    m_grid->SetReadOnly(row, 2);
}

void GrblConfigDialog::ReloadGrid() {
    auto settings = m_controller->GetSettings();
    
    if (m_grid->GetNumberRows() > 0) m_grid->DeleteRows(0, m_grid->GetNumberRows());

    for (auto const& [id, val] : settings) {
        m_grid->AppendRows(1);
        int row = m_grid->GetNumberRows() - 1;
        m_grid->SetCellValue(row, 0, id);
        m_grid->SetCellValue(row, 1, val);
    }
}

void GrblConfigDialog::OnRefresh(wxCommandEvent& event) {
    if (m_grid->GetNumberRows() > 0) m_grid->DeleteRows(0, m_grid->GetNumberRows());
    m_controller->SendCommand("$$");
}

void GrblConfigDialog::OnSave(wxCommandEvent& event) {
    // Iterate over rows and send updates
    for (int i = 0; i < m_grid->GetNumberRows(); ++i) {
        std::string id = m_grid->GetCellValue(i, 0).ToStdString();
        std::string val = m_grid->GetCellValue(i, 1).ToStdString();
        
        // Construct command: $100=250.000
        std::string cmd = id + "=" + val;
        m_controller->SendCommand(cmd);
    }
    wxMessageBox("Configuration Sent!", "Info", wxOK | wxICON_INFORMATION);
}

void GrblConfigDialog::OnClose(wxCommandEvent& event) {
    EndModal(wxID_CANCEL);
}
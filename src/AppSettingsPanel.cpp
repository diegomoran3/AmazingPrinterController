#include "AppSettingsPanel.hpp"

AppSettingsPanel::AppSettingsPanel(wxWindow* parent, AppSettings* settings, SettingsChangedCallback onChanged)
    : wxPanel(parent, wxID_ANY),
      m_settings(settings),
      m_onChanged(onChanged)
{
    BuildUI();
}

void AppSettingsPanel::BuildUI()
{
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // ---- Display Section ----
    auto* displayBox = new wxStaticBoxSizer(wxVERTICAL, this, "Display");
    auto* displayParent = displayBox->GetStaticBox();

    m_chkShowPreview = new wxCheckBox(displayParent, wxID_ANY, "Show scan grid preview");
    m_chkShowPreview->SetValue(m_settings->showPreview);
    m_chkShowPreview->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        m_settings->showPreview = m_chkShowPreview->GetValue();
        OnSettingChanged();
    });
    displayBox->Add(m_chkShowPreview, 0, wxALL, 5);

    mainSizer->Add(displayBox, 0, wxEXPAND | wxALL, 10);

    // ---- Connection Section ----
    auto* connBox = new wxStaticBoxSizer(wxVERTICAL, this, "Connection");
    auto* connParent = connBox->GetStaticBox();

    m_chkAutoHome = new wxCheckBox(connParent, wxID_ANY, "Auto-home on connect ($H)");
    m_chkAutoHome->SetValue(m_settings->autoHomeOnConnect);
    m_chkAutoHome->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent&) {
        m_settings->autoHomeOnConnect = m_chkAutoHome->GetValue();
        OnSettingChanged();
    });
    connBox->Add(m_chkAutoHome, 0, wxALL, 5);

    mainSizer->Add(connBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // ---- Status Polling Section ----
    auto* pollBox = new wxStaticBoxSizer(wxVERTICAL, this, "Status Polling");
    auto* pollParent = pollBox->GetStaticBox();

    auto* pollRow = new wxBoxSizer(wxHORIZONTAL);
    pollRow->Add(new wxStaticText(pollParent, wxID_ANY, "Poll Rate:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    m_spinPollRate = new wxSpinCtrl(pollParent, wxID_ANY, wxString::Format("%d", m_settings->statusPollRateHz),
                                     wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS,
                                     1, 50, m_settings->statusPollRateHz);
    m_spinPollRate->Bind(wxEVT_SPINCTRL, [this](wxSpinEvent&) {
        m_settings->statusPollRateHz = m_spinPollRate->GetValue();
        OnSettingChanged();
    });
    pollRow->Add(m_spinPollRate, 0, wxRIGHT, 5);
    pollRow->Add(new wxStaticText(pollParent, wxID_ANY, "Hz"), 0, wxALIGN_CENTER_VERTICAL);
    pollBox->Add(pollRow, 0, wxALL, 5);

    mainSizer->Add(pollBox, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
}

void AppSettingsPanel::OnSettingChanged()
{
    if (m_onChanged) {
        m_onChanged();
    }
}
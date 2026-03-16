#pragma once
#include <wx/wx.h>
#include <vector>
#include <functional>
#include "ScanHandler.hpp"

struct DataPoint {
    double x;
    double y;
    wxColour color;
};


class CoordinatePanel : public wxPanel {
public:
    CoordinatePanel(wxWindow* parent, double minX = 0.0, double maxX = 400.0, double minY = 0.0, double maxY = 380.0);
    
    void DrawPreviewRegion(double x, double y, double width, double height);
    void ClearPreviewRegion();

    void SetScanGridPreview(const std::vector<ScanLine>& scanLines);
    void ClearScanGridPreview();

    void AddPoint(double x, double y, wxColour color = *wxRED);
    void ClearPoints();

    void SetMachinePosition(double x, double y);

    void SetOnPointClicked(std::function<void(double, double)> callback) {
        m_onClick = callback;
    }
    
private:
    void OnPaint(wxPaintEvent& evt);
    wxPoint CoordToScreen(double x, double y);

    double minX, maxX, minY, maxY;
    std::vector<DataPoint> points;

    // Persistent machine position (updated by status polling, drawn separately)
    double m_machineX = 0.0;
    double m_machineY = 0.0;
    bool m_hasMachinePos = false;

    void ScreenToCoord(int px, int py, double& outX, double& outY);
    void OnMouseClick(wxMouseEvent& event);

    std::function<void(double, double)> m_onClick;
    wxRect2DDouble m_previewRect;
    bool m_hasPreview = false;

    std::vector<ScanLine> m_scanGridLines;
    bool m_hasScanGrid = false;
};
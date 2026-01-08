#include "CoordinatePanel.h"
#include <cmath>

CoordinatePanel::CoordinatePanel(wxWindow* parent) : wxPanel(parent) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &CoordinatePanel::OnPaint, this);
    
    // Set up coordinate system bounds
    minX = -5.0; maxX = 5.0;
    minY = -5.0; maxY = 5.0;
}

void CoordinatePanel::AddPoint(double x, double y, wxColour color) {
    points.push_back({x, y, color});
    Refresh();
}

void CoordinatePanel::ClearPoints() {
    points.clear();
    Refresh();
}

wxPoint CoordinatePanel::CoordToScreen(double x, double y) {
    wxSize size = GetClientSize();
    int margin = 40;
    
    int screenX = margin + (x - minX) / (maxX - minX) * (size.x - 2 * margin);
    int screenY = size.y - margin - (y - minY) / (maxY - minY) * (size.y - 2 * margin);
    
    return wxPoint(screenX, screenY);
}

void CoordinatePanel::OnPaint(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    
    wxSize size = GetClientSize();
    int margin = 40;
    
    // Draw axes
    dc.SetPen(wxPen(*wxBLACK, 2));
    wxPoint origin = CoordToScreen(0, 0);
    wxPoint xEnd = CoordToScreen(maxX, 0);
    wxPoint yEnd = CoordToScreen(0, maxY);
    wxPoint xStart = CoordToScreen(minX, 0);
    wxPoint yStart = CoordToScreen(0, minY);
    
    // X-axis
    dc.DrawLine(xStart, origin);
    dc.DrawLine(origin, xEnd);
    // Y-axis
    dc.DrawLine(yStart, origin);
    dc.DrawLine(origin, yEnd);
    
    // Draw grid lines
    dc.SetPen(wxPen(wxColour(200, 200, 200), 1));
    for (int i = std::ceil(minX); i <= std::floor(maxX); i++) {
        if (i == 0) continue;
        wxPoint p1 = CoordToScreen(i, minY);
        wxPoint p2 = CoordToScreen(i, maxY);
        dc.DrawLine(p1, p2);
    }
    for (int i = std::ceil(minY); i <= std::floor(maxY); i++) {
        if (i == 0) continue;
        wxPoint p1 = CoordToScreen(minX, i);
        wxPoint p2 = CoordToScreen(maxX, i);
        dc.DrawLine(p1, p2);
    }
    
    // Draw axis labels
    dc.SetTextForeground(*wxBLACK);
    wxFont font = dc.GetFont();
    font.SetPointSize(8);
    dc.SetFont(font);
    
    for (int i = std::ceil(minX); i <= std::floor(maxX); i++) {
        wxPoint p = CoordToScreen(i, 0);
        dc.DrawText(wxString::Format("%d", i), p.x - 5, p.y + 5);
    }
    for (int i = std::ceil(minY); i <= std::floor(maxY); i++) {
        if (i == 0) continue;
        wxPoint p = CoordToScreen(0, i);
        dc.DrawText(wxString::Format("%d", i), p.x + 5, p.y - 5);
    }
    
    // Draw points
    for (const auto& pt : points) {
        wxPoint screenPt = CoordToScreen(pt.x, pt.y);
        dc.SetBrush(wxBrush(pt.color));
        dc.SetPen(wxPen(pt.color, 2));
        dc.DrawCircle(screenPt, 5);
        
        // Draw coordinates label
        dc.SetTextForeground(pt.color);
        dc.DrawText(wxString::Format("(%.1f, %.1f)", pt.x, pt.y), 
                   screenPt.x + 8, screenPt.y - 8);
    }
}
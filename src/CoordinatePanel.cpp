#include "CoordinatePanel.hpp"
#include <cmath>
#include <wx/dcbuffer.h>

CoordinatePanel::CoordinatePanel(wxWindow* parent, double minX, double maxX, double minY, double maxY) : wxPanel(parent), minX(minX), maxX(maxX), minY(minY), maxY(maxY) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Set the requested window size
    SetMinSize(wxSize(400, 380));
    SetSize(wxSize(400, 380));

    Bind(wxEVT_PAINT, &CoordinatePanel::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &CoordinatePanel::OnMouseClick, this);
    
    // Set coordinate system to match requested range
    this->minX = minX;
    this->maxX = maxX;
    this->minY = minY;
    this->maxY = maxY;
}

void CoordinatePanel::DrawPreviewRegion(double x, double y, double w, double h) {
    m_previewRect = wxRect2DDouble(x, y, w, h);
    m_hasPreview = true;
    Refresh();
}

void CoordinatePanel::ClearPreviewRegion() {
    m_hasPreview = false;
    Refresh(); 
}

void CoordinatePanel::SetScanGridPreview(const std::vector<ScanLine>& scanLines) {
    m_scanGridLines = scanLines;
    m_hasScanGrid = true;
    m_hasPreview = false; // Replace old rectangle preview
    Refresh();
}

void CoordinatePanel::ClearScanGridPreview() {
    m_scanGridLines.clear();
    m_hasScanGrid = false;
    Refresh();
}

wxPoint CoordinatePanel::CoordToScreen(double x, double y) {
    wxSize size = GetClientSize();
    
    // 1. Define base margins
    int marginLeft = 45, marginBottom = 35, marginRight = 20, marginTop = 20;
    int availableWidth = size.x - marginLeft - marginRight;
    int availableHeight = size.y - marginBottom - marginTop;

    if (availableWidth <= 0 || availableHeight <= 0) return wxPoint(0,0);

    // 2. Calculate aspect ratios
    double dataAspectRatio = (maxX - minX) / (maxY - minY);
    double screenAspectRatio = (double)availableWidth / availableHeight;

    int drawableWidth, drawableHeight;
    int xOffset = marginLeft;
    int yOffset = marginTop;

    if (screenAspectRatio > dataAspectRatio) {
        // Window is too wide: height is the constraint
        drawableHeight = availableHeight;
        drawableWidth = availableHeight * dataAspectRatio;
        // Center horizontally in the available space
        xOffset += (availableWidth - drawableWidth) / 2;
    } else {
        // Window is too tall: width is the constraint
        drawableWidth = availableWidth;
        drawableHeight = availableWidth / dataAspectRatio;
        // Center vertically in the available space
        yOffset += (availableHeight - drawableHeight) / 2;
    }

    // 3. Transform coordinates
    int screenX = xOffset + ((x - minX) / (maxX - minX)) * drawableWidth;
    // Note: yOffset + drawableHeight gives the "bottom" of our centered graph
    int screenY = (yOffset + drawableHeight) - ((y - minY) / (maxY - minY)) * drawableHeight;
    
    return wxPoint(screenX, screenY);
}

void CoordinatePanel::ScreenToCoord(int px, int py, double& outX, double& outY) {
    wxSize size = GetClientSize();
    
    // Must match CoordToScreen exactly
    int marginLeft = 45, marginBottom = 35, marginRight = 20, marginTop = 20;
    int availableWidth = size.x - marginLeft - marginRight;
    int availableHeight = size.y - marginBottom - marginTop;

    if (availableWidth <= 0 || availableHeight <= 0) return;

    // Replicate the same aspect ratio logic as CoordToScreen
    double dataAspectRatio = (maxX - minX) / (maxY - minY);
    double screenAspectRatio = (double)availableWidth / availableHeight;

    int drawableWidth, drawableHeight;
    int xOffset = marginLeft;
    int yOffset = marginTop;

    if (screenAspectRatio > dataAspectRatio) {
        drawableHeight = availableHeight;
        drawableWidth = availableHeight * dataAspectRatio;
        xOffset += (availableWidth - drawableWidth) / 2;
    } else {
        drawableWidth = availableWidth;
        drawableHeight = availableWidth / dataAspectRatio;
        yOffset += (availableHeight - drawableHeight) / 2;
    }

    if (drawableWidth <= 0 || drawableHeight <= 0) return;

    // Inverse of: screenX = xOffset + ((x - minX) / (maxX - minX)) * drawableWidth
    double xPercent = (double)(px - xOffset) / drawableWidth;
    outX = minX + xPercent * (maxX - minX);

    // Inverse of: screenY = (yOffset + drawableHeight) - ((y - minY) / (maxY - minY)) * drawableHeight
    double yPercent = (double)((yOffset + drawableHeight) - py) / drawableHeight;
    outY = minY + yPercent * (maxY - minY);

    // Clamp to bounds
    if (outX < minX) outX = minX;
    if (outX > maxX) outX = maxX;
    if (outY < minY) outY = minY;
    if (outY > maxY) outY = maxY;
}

void CoordinatePanel::OnMouseClick(wxMouseEvent& event) {
    // Only act if a callback is registered
    if (m_onClick) {
        double x, y;
        ScreenToCoord(event.GetX(), event.GetY(), x, y);
        
        // Trigger the callback with the calculated coordinates
        m_onClick(x, y);
    }
    event.Skip();
}

void CoordinatePanel::AddPoint(double x, double y, wxColour color) {
    points.push_back({x, y, color});
    Refresh();
}

void CoordinatePanel::ClearPoints() {
    points.clear();
    Refresh();
}

void CoordinatePanel::OnPaint(wxPaintEvent& evt) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    
    // Draw Grid Lines every 50 units
    dc.SetPen(wxPen(wxColour(230, 230, 230), 1));
    int step = 50;

    for (int i = (int)minX; i <= (int)maxX; i += step) {
        dc.DrawLine(CoordToScreen(i, minY), CoordToScreen(i, maxY));
    }
    for (int i = (int)minY; i <= (int)maxY; i += step) {
        dc.DrawLine(CoordToScreen(minX, i), CoordToScreen(maxX, i));
    }
    
    // Draw Main Axes
    dc.SetPen(wxPen(*wxBLACK, 2));
    wxPoint origin = CoordToScreen(0, 0);
    wxPoint xEnd = CoordToScreen(maxX, 0);
    wxPoint yEnd = CoordToScreen(0, maxY);
    
    dc.DrawLine(origin, xEnd);
    dc.DrawLine(origin, yEnd);
    
    // Axis Labels
    dc.SetTextForeground(*wxBLACK);
    wxFont font = dc.GetFont();
    font.SetPointSize(7);
    dc.SetFont(font);
    
    // X-axis numbers
    for (int i = (int)minX; i <= (int)maxX; i += step) {
        wxPoint p = CoordToScreen(i, 0);
        dc.DrawText(wxString::Format("%d", i), p.x - 10, p.y + 5);
    }
    
    // Y-axis numbers
    for (int i = (int)minY; i <= (int)maxY; i += step) {
        if (i == 0) continue; 
        wxPoint p = CoordToScreen(0, i);
        dc.DrawText(wxString::Format("%d", i), p.x - 35, p.y - 7);
    }
    
    // Draw Points
    for (const auto& pt : points) {
        wxPoint screenPt = CoordToScreen(pt.x, pt.y);
        
        // Only draw if within visible range
        if (screenPt.x >= 0 && screenPt.y >= 0) {
            dc.SetBrush(wxBrush(pt.color));
            dc.SetPen(wxPen(pt.color, 1));
            dc.DrawCircle(screenPt, 3);
            
            // Optional: coordinate label
            dc.SetTextForeground(pt.color);
            dc.DrawText(wxString::Format("(%.0f,%.0f)", pt.x, pt.y), 
                       screenPt.x + 5, screenPt.y - 12);
        }
    }

    // Draw scan grid preview
    if (m_hasScanGrid) {
        wxPen linePen(wxColour(100, 100, 255), 1, wxPENSTYLE_DOT);
        wxBrush pointBrush(wxColour(60, 120, 255));

        dc.SetPen(linePen);

        // Draw connecting lines and points for each scan line
        for (const auto& line : m_scanGridLines) {
            // Draw lead-in: physicalStart -> first point
            if (!line.points.empty()) {
                wxPen leadPen(wxColour(255, 160, 0), 2, wxPENSTYLE_SHORT_DASH);
                dc.SetPen(leadPen);
                wxPoint leadStart = CoordToScreen(line.physicalStartX, line.physicalStartY);
                wxPoint firstPt = CoordToScreen(line.points.front().x, line.points.front().y);
                if (leadStart != firstPt) {
                    dc.DrawLine(leadStart, firstPt);
                    // Draw a small marker at the physical start
                    dc.SetPen(*wxTRANSPARENT_PEN);
                    dc.SetBrush(wxBrush(wxColour(255, 160, 0)));
                    dc.DrawCircle(leadStart, 2);
                }
            }

            // Draw scan path between measurement points
            dc.SetPen(linePen);
            for (size_t i = 1; i < line.points.size(); ++i) {
                wxPoint from = CoordToScreen(line.points[i - 1].x, line.points[i - 1].y);
                wxPoint to = CoordToScreen(line.points[i].x, line.points[i].y);
                dc.DrawLine(from, to);
            }

            // Draw lead-out: last point -> physicalEnd
            if (!line.points.empty()) {
                wxPen leadPen(wxColour(255, 160, 0), 2, wxPENSTYLE_SHORT_DASH);
                dc.SetPen(leadPen);
                wxPoint lastPt = CoordToScreen(line.points.back().x, line.points.back().y);
                wxPoint leadEnd = CoordToScreen(line.physicalEndX, line.physicalEndY);
                if (lastPt != leadEnd) {
                    dc.DrawLine(lastPt, leadEnd);
                    dc.SetPen(*wxTRANSPARENT_PEN);
                    dc.SetBrush(wxBrush(wxColour(255, 160, 0)));
                    dc.DrawCircle(leadEnd, 2);
                }
            }

            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(pointBrush);
            for (const auto& pt : line.points) {
                wxPoint screenPt = CoordToScreen(pt.x, pt.y);
                dc.DrawCircle(screenPt, 3);
            }
            dc.SetPen(linePen);
        }

        // Draw traverse lines between scan lines (grey dotted)
        wxPen traversePen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT);
        dc.SetPen(traversePen);
        for (size_t i = 1; i < m_scanGridLines.size(); ++i) {
            const auto& prevLine = m_scanGridLines[i - 1];
            const auto& currLine = m_scanGridLines[i];
            wxPoint from = CoordToScreen(prevLine.physicalEndX, prevLine.physicalEndY);
            wxPoint to = CoordToScreen(currLine.physicalStartX, currLine.physicalStartY);
            dc.DrawLine(from, to);
        }

        // Mark start point in green
        if (!m_scanGridLines.empty() && !m_scanGridLines[0].points.empty()) {
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetBrush(wxBrush(wxColour(0, 200, 0)));
            wxPoint startPt = CoordToScreen(m_scanGridLines[0].points[0].x, m_scanGridLines[0].points[0].y);
            dc.DrawCircle(startPt, 5);
        }
    }

    if (m_hasPreview) {
        dc.SetPen(wxPen(*wxRED, 1, wxPENSTYLE_SHORT_DASH));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);

        wxPoint topLeft = CoordToScreen(m_previewRect.m_x, m_previewRect.m_y + m_previewRect.m_height);
        wxPoint bottomRight = CoordToScreen(m_previewRect.m_x + m_previewRect.m_width, m_previewRect.m_y);

        int rectWidth = bottomRight.x - topLeft.x;
        int rectHeight = bottomRight.y - topLeft.y; 

        dc.DrawRectangle(topLeft.x, topLeft.y, rectWidth, rectHeight);
    }
}
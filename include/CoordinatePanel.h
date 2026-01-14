#ifndef COORDINATEPANEL_H
#define COORDINATEPANEL_H

#include <wx/wx.h>
#include <vector>
#include <functional>

struct DataPoint {
    double x;
    double y;
    wxColour color;
};

class CoordinatePanel : public wxPanel {
public:
    CoordinatePanel(wxWindow* parent);

    void AddPoint(double x, double y, wxColour color = *wxRED);
    void ClearPoints();

    void SetOnPointClicked(std::function<void(double, double)> callback) {
        m_onClick = callback;
    }
    
private:
    void OnPaint(wxPaintEvent& evt);
    wxPoint CoordToScreen(double x, double y);

    double minX, maxX, minY, maxY;
    std::vector<DataPoint> points;

    void ScreenToCoord(int px, int py, double& outX, double& outY);
    void OnMouseClick(wxMouseEvent& event);

    std::function<void(double, double)> m_onClick;
};

#endif

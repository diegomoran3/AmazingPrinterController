#include "ScanHandler.hpp"

ScanHandler::ScanHandler(std::shared_ptr<GrblController> grbl) 
    : m_grbl(std::move(grbl)) {}

void ScanHandler::StartScanCycle(std::vector<ScanLine> scanLines, std::function<void(int, int, double, double)> onPointReached, int feedRate)
{
    m_shouldCancel = false;

    m_grbl->MoveTo(scanLines[0].physicalStartX, scanLines[0].physicalStartY, feedRate);
    m_grbl->WaitForArrival(scanLines[0].physicalStartX, scanLines[0].physicalStartY);

    for (const auto& line : scanLines) {
        for (const auto& point : line.points) {
            
            if (m_shouldCancel) return;

            m_grbl->MoveTo(point.x, point.y, feedRate);
            m_grbl->WaitForArrival(point.x, point.y);

            onPointReached(point.row, point.col, point.x, point.y);            
        }
    }
}

void ScanHandler::StartContinuousScanCycle(std::vector<ScanLine> scanLines, std::function<void(int, int, double, double)> onPointReached, int feedRate)
{
    m_shouldCancel = false;
    double unitsPerSecond = feedRate / 60.0;

    for (const auto& line : scanLines) {
        if (m_shouldCancel) return;

        m_grbl->MoveTo(line.physicalStartX, line.physicalStartY, feedRate);
        m_grbl->WaitForArrival(line.physicalStartX, line.physicalStartY);

        m_grbl->MoveTo(line.physicalEndX, line.physicalEndY, feedRate);
        
        auto lineStartTime = std::chrono::steady_clock::now();

        for (const auto& pt : line.points) {
            if (m_shouldCancel) return;

            double dx = pt.x - line.physicalStartX;
            double dy = pt.y - line.physicalStartY;
            double distFromStart = std::sqrt(dx*dx + dy*dy);
            double expectedSeconds = distFromStart / unitsPerSecond;

            while (true) {
                if (m_shouldCancel) return;
                
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - lineStartTime;
                
                if (elapsed.count() >= expectedSeconds) {
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            onPointReached(pt.row, pt.col, pt.x, pt.y);
        }

        m_grbl->WaitForArrival(line.physicalEndX, line.physicalEndY);
    }
}

std::vector<ScanLine> ScanHandler::CreateScanLines(int rows, int cols, double startX, double startY, double stepX, double stepY, bool direction, bool zigzag, double leadDistance)
{
    std::vector<ScanLine> lines;

    bool isHorizontal = (direction == ScanDirection::Horizontal);
    int outerLimit = isHorizontal ? rows : cols;
    int innerLimit = isHorizontal ? cols : rows;

    for (int i = 0; i < outerLimit; ++i) {
        ScanLine currentLine;
        bool isReverse = zigzag && (i % 2 != 0);

        for (int k = 0; k < innerLimit; ++k) 
        {
            int j = k;

            if (isReverse) {
                j = (innerLimit - 1) - k; 
            }

            int r = isHorizontal ? i : j;
            int c = isHorizontal ? j : i;

            ScanPoint pt;
            pt.row = r;
            pt.col = c;
            pt.x = startX + (c * stepX);
            pt.y = startY + (r * stepY);

            currentLine.points.push_back(pt);  
        }

        lines.push_back(currentLine);
    }

    return lines;
}

void ScanHandler::CancelScan()
{
    m_shouldCancel = true;
}
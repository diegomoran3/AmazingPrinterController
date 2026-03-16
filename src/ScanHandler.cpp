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

    // Feed rate is mm/min in GRBL; convert to mm/sec for time calculations.
    double vCruise = feedRate / 60.0;  // mm/sec (target cruise velocity)

    for (const auto& line : scanLines) {
        if (m_shouldCancel) return;
        if (line.points.empty()) continue;

        // --- Determine the scan-axis acceleration ---
        // We figure out which axis dominates the line direction.
        double lineDx = line.physicalEndX - line.physicalStartX;
        double lineDy = line.physicalEndY - line.physicalStartY;
        double lineLength = std::sqrt(lineDx * lineDx + lineDy * lineDy);

        if (lineLength < 1e-6) continue; // degenerate line

        // Unit direction vector of the line
        double ux = lineDx / lineLength;
        double uy = lineDy / lineLength;

        // Blend X and Y accelerations proportionally to their contribution.
        // This approximates GRBL's per-axis acceleration limiting.
        double accelX = m_grbl->GetAxisAcceleration('X');
        double accelY = m_grbl->GetAxisAcceleration('Y');

        // Weighted harmonic-style blend: the limiting axis dominates.
        // For a pure X line this gives accelX, for pure Y it gives accelY.
        double absUx = std::abs(ux);
        double absUy = std::abs(uy);
        double accel;
        if (absUx < 1e-9) {
            accel = accelY;
        } else if (absUy < 1e-9) {
            accel = accelX;
        } else {
            // The axis with the larger component fraction limits accel.
            // a_eff = min(aX / |ux|, aY / |uy|) clamped to physical limit
            accel = std::min(accelX / absUx, accelY / absUy);
        }

        // --- Kinematic phase calculations ---
        // Time and distance to accelerate from 0 to vCruise:
        double tAccel = vCruise / accel;             // seconds
        double dAccel = 0.5 * accel * tAccel * tAccel; // mm

        // If the line is so short we never reach full speed, handle triangular profile
        // (unlikely with proper lead distance, but be safe).
        bool reachesCruise = (2.0 * dAccel <= lineLength);
        double tDecel = tAccel; // symmetric accel/decel
        double dDecel = dAccel;

        // --- Rapid to the physical start (lead-in point), then sweep ---
        m_grbl->MoveTo(line.physicalStartX, line.physicalStartY, feedRate);
        m_grbl->WaitForArrival(line.physicalStartX, line.physicalStartY);

        // Single G1 command from physicalStart to physicalEnd at feedRate.
        m_grbl->MoveTo(line.physicalEndX, line.physicalEndY, feedRate);

        auto lineStartTime = std::chrono::steady_clock::now();

        for (const auto& pt : line.points) {
            if (m_shouldCancel) return;

            // Distance of this point from the physical start of the line
            double dx = pt.x - line.physicalStartX;
            double dy = pt.y - line.physicalStartY;
            double distFromStart = std::sqrt(dx * dx + dy * dy);

            // --- Compute expected arrival time using trapezoidal velocity profile ---
            double expectedSeconds;

            if (reachesCruise) {
                double dCruise = lineLength - dAccel - dDecel;
                double tCruise = dCruise / vCruise;

                if (distFromStart <= dAccel) {
                    // Still in acceleration phase: d = 0.5 * a * t^2  =>  t = sqrt(2d/a)
                    expectedSeconds = std::sqrt(2.0 * distFromStart / accel);
                } else if (distFromStart <= dAccel + dCruise) {
                    // In cruise phase
                    expectedSeconds = tAccel + (distFromStart - dAccel) / vCruise;
                } else {
                    // In deceleration phase
                    double distIntoDecel = distFromStart - dAccel - dCruise;
                    // v_entry = vCruise, decelerating at 'accel'
                    // d = vCruise*t - 0.5*a*t^2  =>  solve quadratic
                    // t = (vCruise - sqrt(vCruise^2 - 2*a*d)) / a
                    double discriminant = vCruise * vCruise - 2.0 * accel * distIntoDecel;
                    if (discriminant < 0.0) discriminant = 0.0;
                    double tInDecel = (vCruise - std::sqrt(discriminant)) / accel;
                    expectedSeconds = tAccel + tCruise + tInDecel;
                }
            } else {
                // Triangular profile: never reaches cruise speed
                double dPeak = lineLength / 2.0;
                double vPeak = std::sqrt(2.0 * accel * dPeak);
                double tPeak = vPeak / accel;

                if (distFromStart <= dPeak) {
                    expectedSeconds = std::sqrt(2.0 * distFromStart / accel);
                } else {
                    double distIntoDecel = distFromStart - dPeak;
                    double discriminant = vPeak * vPeak - 2.0 * accel * distIntoDecel;
                    if (discriminant < 0.0) discriminant = 0.0;
                    double tInDecel = (vPeak - std::sqrt(discriminant)) / accel;
                    expectedSeconds = tPeak + tInDecel;
                }
            }

            // --- Wait until the expected time ---
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

double ScanHandler::CalculateLeadDistance(int feedRate, char axis) const
{
    // v = feedRate in mm/min => convert to mm/sec
    double v = feedRate / 60.0;
    double a = m_grbl->GetAxisAcceleration(axis); // mm/sec^2

    // Distance to accelerate from 0 to v:  d = v^2 / (2a)
    // Add a small margin (10%) for safety.
    return (v * v) / (2.0 * a) * 1.1;
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

        // Set physical start/end positions for this line (used by continuous scan)
        if (!currentLine.points.empty()) {
            const auto& first = currentLine.points.front();
            const auto& last = currentLine.points.back();

            // Determine lead direction (extend start/end along the scan axis)
            if (isHorizontal) {
                double leadSign = isReverse ? 1.0 : -1.0;
                currentLine.physicalStartX = first.x + leadSign * leadDistance;
                currentLine.physicalStartY = first.y;
                currentLine.physicalEndX = last.x - leadSign * leadDistance;
                currentLine.physicalEndY = last.y;
            } else {
                double leadSign = isReverse ? 1.0 : -1.0;
                currentLine.physicalStartX = first.x;
                currentLine.physicalStartY = first.y + leadSign * leadDistance;
                currentLine.physicalEndX = last.x;
                currentLine.physicalEndY = last.y - leadSign * leadDistance;
            }
        }

        lines.push_back(currentLine);
    }

    return lines;
}

void ScanHandler::CancelScan()
{
    m_shouldCancel = true;
}
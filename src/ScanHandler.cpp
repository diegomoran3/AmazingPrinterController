#include "ScanHandler.hpp"

ScanHandler::ScanHandler(std::shared_ptr<GrblController> grbl) 
    : m_grbl(std::move(grbl)) {}

void ScanHandler::StartScanCycle(double startX, double startY, int rows, int cols, double stepX, double stepY, std::function<void(int, int, double, double)> onPointReached, ScanDirection direction, bool zigzag, double feedRate)
{
    m_shouldCancel = false;

    m_grbl->MoveTo(startX, startY, feedRate);
    m_grbl->WaitForArrival(startX, startY);

    bool isHorizontal = (direction == ScanDirection::Horizontal);
    int outerLimit = isHorizontal ? rows : cols;
    int innerLimit = isHorizontal ? cols : rows;

    for (int i = 0; i < outerLimit; ++i) {
        for (int k = 0; k < innerLimit; ++k) {
            
            if (m_shouldCancel) return;

            int j = k;
            if (zigzag && (i % 2 != 0)) {
                j = (innerLimit - 1) - k; 
            }

            int r = isHorizontal ? i : j;
            int c = isHorizontal ? j : i;

            double targetX = startX + (c * stepX);
            double targetY = startY + (r * stepY);

            m_grbl->MoveTo(targetX, targetY, feedRate);
            m_grbl->WaitForArrival(targetX, targetY);

            onPointReached(r, c, targetX, targetY);
        }
    }
}

void ScanHandler::StartContinuousScanCycle(double startX, double startY, int rows, int cols, double stepX, double stepY, std::function<void(int, int, double, double)> onPointReached, ScanDirection direction, bool zigzag, double feedRate, double leadDistance = 5.0)
{
    m_shouldCancel = false;

    // Convert units/min to units/sec for the timer
    double unitsPerSecond = feedRate / 60.0;

    bool isHorizontal = (direction == ScanDirection::Horizontal);
    int outerLimit = isHorizontal ? rows : cols;
    int innerLimit = isHorizontal ? cols : rows;

    for (int i = 0; i < outerLimit; ++i) {
        if (m_shouldCancel) return;

        bool isReverse = zigzag && (i % 2 != 0);

        // 1. Identify the logical start and end indices of the actual data points
        int startK = isReverse ? (innerLimit - 1) : 0;
        int endK = isReverse ? 0 : (innerLimit - 1);

        // Calculate the exact coordinates of the first and last data points on this line
        double firstPointX = startX + ((isHorizontal ? startK : i) * stepX);
        double firstPointY = startY + ((isHorizontal ? i : startK) * stepY);
        
        double lastPointX = startX + ((isHorizontal ? endK : i) * stepX);
        double lastPointY = startY + ((isHorizontal ? i : endK) * stepY);

        // 2. Determine the direction vector for the lead-in and lead-out
        double moveDX = 0.0;
        double moveDY = 0.0;
        
        if (isHorizontal) {
            moveDX = isReverse ? -1.0 : 1.0;
        } else {
            moveDY = isReverse ? -1.0 : 1.0;
        }

        // Calculate the physical start (lead-in) and end (lead-out) coordinates
        double physicalStartX = firstPointX - (moveDX * leadDistance);
        double physicalStartY = firstPointY - (moveDY * leadDistance);

        double physicalEndX = lastPointX + (moveDX * leadDistance);
        double physicalEndY = lastPointY + (moveDY * leadDistance);

        // 3. Move to the physical start position and wait for the machine to settle
        m_grbl->MoveTo(physicalStartX, physicalStartY, feedRate);
        m_grbl->WaitForArrival(physicalStartX, physicalStartY);

        // 4. Send the single continuous movement command for the entire extended line
        m_grbl->MoveTo(physicalEndX, physicalEndY, feedRate);

        // 5. Start the precision timer for the sweep
        auto lineStartTime = std::chrono::steady_clock::now();

        // 6. Iterate through the points and trigger the callback based on calculated time
        for (int k = 0; k < innerLimit; ++k) {
            if (m_shouldCancel) return;

            int currentK = isReverse ? (innerLimit - 1 - k) : k;
            int r = isHorizontal ? i : currentK;
            int c = isHorizontal ? currentK : i;

            double ptX = startX + (c * stepX);
            double ptY = startY + (r * stepY);

            // Calculate the distance from the physical start point to this data point
            double distFromStart = std::sqrt(std::pow(ptX - physicalStartX, 2) + std::pow(ptY - physicalStartY, 2));
            
            // Calculate how long it should take to cover that distance at constant velocity
            double expectedSeconds = distFromStart / unitsPerSecond;

            // Block until the expected time has elapsed
            while (true) {
                if (m_shouldCancel) return;
                
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - lineStartTime;
                
                if (elapsed.count() >= expectedSeconds) {
                    break;
                }
                
                // Sleep for 1ms to prevent pegging the CPU at 100% during the polling loop
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // Trigger the callback exactly as the machine flies over the coordinate
            onPointReached(r, c, ptX, ptY);
        }

        // 7. Wait for the machine to finish the lead-out before moving to the next line
        m_grbl->WaitForArrival(physicalEndX, physicalEndY);
    }
}

void ScanHandler::CancelScan()
{
    m_shouldCancel = true;
}
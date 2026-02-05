// --- Include Guard --- //
#pragma once

// --- Includes --- //
#include <opencv2/opencv.hpp>
#include <vector>
#include "DrumColorList.hpp"
#include "DrumDetectorConfig.hpp"

namespace DrumDetector
{
    /**
     * @class DrumDetector
     * @brief Singleton for hardware-accelerated drum detection on Wombat.
     * * Handles the entire pipeline from frame acquisition to color classification.
     * Accesses hardware and profile parameters via Types::ScannerConfig.
     */
    class DrumDetector
    {
        public:
            /** * @brief Access the global singleton instance of the detector.
             * @return DrumDetector& Reference to the instance.
             */
            static DrumDetector& getInstance();

            DrumDetector(const DrumDetector&) = delete;
            void operator=(const DrumDetector&) = delete;

            /** * @brief Initializes or re-initializes the camera.
             * Uses the CameraIndex and Exposure from ScannerConfig.
             */
            void init();

            /** * @brief Executes the detection pipeline.
             * Captures a frame, finds the tray, warps it and classifies the 8 drum slots.
             * @return Types::DrumColorList The list of 8 detected colors.
             */
            Types::DrumColorList getDrumColors();

        private:
            /** @brief Private constructor for Singleton. */
            DrumDetector();

            /** @brief Destructor. Ensures camera resource is released. */
            ~DrumDetector();

            cv::VideoCapture m_cap;
            Types::DrumDetectorConfig& config;
            std::shared_ptr<spdlog::logger> logger;

            // --- Internal Processing Steps ---

            /** @brief Flushes the camera buffer and retrieves the latest frame. */
            [[nodiscard]] cv::Mat getSnapshot();

            /** @brief Sorts 4 points in clockwise order for perspective transform. */
            [[nodiscard]] static std::vector<cv::Point2f> sortRadial(std::vector<cv::Point2f> pts);

            /** @brief Validates tray geometry based on aspect ratio and parallelism. */
            [[nodiscard]] bool checkShape(std::vector<cv::Point2f> pts);

            /** @brief Boosts image saturation using a high-performance LUT. */
            [[nodiscard]] cv::Mat enhanceSaturation(const cv::Mat& src) const;

            /** @brief Calculates the median value of an ROI for robust color detection. */
            [[nodiscard]] static int getMedian(const cv::Mat& channel);
    };
}
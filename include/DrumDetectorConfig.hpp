// --- Include Guard --- //
#pragma once

// --- Includes --- //
#include <string>
#include <memory>
#include <spdlog/spdlog.h>

// --- Code --- //
/**
* @namespace DrumDetector
* @brief Namespace for all drum detection related code.
*/

/**
 * @namespace Types
 * @brief Namespace for all drum detection related types.
 */
namespace DrumDetector::Types
{
    /**
     * @class DrumDetectorConfig
     * @brief Singleton class holding DrumDetector parameters.
     * ### Expected JSON Structure:
     * @code
     * {
     * "DrumDetector": {
     * "Internal": {
     * "CameraPath": '/dev/v4l/by-id/usb-SunplusIT_Inc_Integrated_RGB_Camera_01.00.00-video-index0',
     * "TrayWidth": 1000,
     * "TrayHeight": 250,
     * "MinMarkerArea": 200,
     * "MaxMarkerArea": 10000
     * },
     * "CurrentProfile": "ProfileA",
     * "ProfileList": [
     * {
     * "name": "ProfileA",
     * "brightness": 150,
     * "exposure": 120,
     * "b_thresh_yellow": 155,
     * "saturation_boost": 2.5,
     * "blue_max": 125,
     * "pink_min": 160
     * },
     * {
     * "name": "ProfileB",
     * "brightness": 130,
     * "exposure": 150,
     * "b_thresh_yellow": 130,
     * "saturation_boost": 2.8,
     * "blue_max": 125,
     * "pink_min": 150
     * }
     * ]
     * }
     * }
     * @endcode
     */
    class DrumDetectorConfig
    {
        public:
            /** @brief Returns the global instance of the config. */
            static DrumDetectorConfig& getInstance();

            DrumDetectorConfig(const DrumDetectorConfig&) = delete;
            void operator=(const DrumDetectorConfig&) = delete;

            /**
             * @brief Loads a specific profile from a JSON configuration file.
             * @param filePath Path to the config.json (e.g., "config.json").
             * @return true if the current profile was loaded successfully, false otherwise.
             */
            void load(const std::string& filePath);

            /**
             * @brief Sets a spdlogger for debug purposes.
             * @param logger Logger object used in the main program.
             */
            void setLogger(std::shared_ptr<spdlog::logger> logger);

            // --- Profile Getters ---
            [[nodiscard]] std::string getName() const { return m_name; }
            [[nodiscard]] int getBrightness() const { return m_brightness; }
            [[nodiscard]] int getExposure() const { return m_exposure; }
            [[nodiscard]] int getBThreshYellow() const { return m_b_thresh_yellow; }
            [[nodiscard]] double getSaturationBoost() const { return m_saturation_boost; }
            [[nodiscard]] int getBlueMax() const { return m_blue_max; }
            [[nodiscard]] int getPinkMin() const { return m_pink_min; }

            // --- Internal/Hardware Getters ---
            [[nodiscard]] std::shared_ptr<spdlog::logger> getLogger() const { return m_logger; }
            [[nodiscard]] std::string getCameraPath() const { return m_cameraPath; }
            [[nodiscard]] int getTrayWidth() const { return m_trayWidth; }
            [[nodiscard]] int getTrayHeight() const { return m_trayHeight; }
            [[nodiscard]] double getMinMarkerArea() const { return m_minMarkerArea; }
            [[nodiscard]] double getMaxMarkerArea() const { return m_maxMarkerArea; }
            [[nodiscard]] double getKeepPercentage() const { return m_keepPercentage; }

            // --- Others --- //
            [[nodiscard]] std::string getConfigPath() const { return config_path; }

        private:
            DrumDetectorConfig();

            // --- Profile params --- //
            std::string m_name{};
            int m_brightness{};
            int m_exposure{};
            int m_b_thresh_yellow{};
            double m_saturation_boost{};
            int m_blue_max{};
            int m_pink_min{};

            // --- Internal hardware params --- //
            std::shared_ptr<spdlog::logger> m_logger;
            std::string m_cameraPath{};
            int m_trayWidth{};
            int m_trayHeight{};
            double m_minMarkerArea{};
            double m_maxMarkerArea{};
            double m_keepPercentage{};

            // --- Others --- //
            std::string config_path{};
    };
}
// --- Includes --- //
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>
#include "../include/DrumDetectorConfig.hpp"

// --- Code --- //
namespace DrumDetector::Types
{
    DrumDetectorConfig& DrumDetectorConfig::getInstance()
    {
        static DrumDetectorConfig instance;
        return instance;
    }

    DrumDetectorConfig::DrumDetectorConfig()
    {
        m_logger = spdlog::default_logger();
    }

    bool DrumDetectorConfig::load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            this->m_logger->error("[DrumDetectorConfig] Could not open file: {}", filePath);
            return false;
        }
        
        this->config_path = filePath;

        try
        {
            nlohmann::json config;
            file >> config;

            if (!config.contains("DrumDetector"))
            {
                this->m_logger->error("[DrumDetectorConfig] 'DrumDetector' section missing in {}", filePath);
                return false;
            }

            auto drumSection = config["DrumDetector"];

            if (!drumSection.contains("Internal"))
            {
                this->m_logger->error("[DrumDetectorConfig] Error: 'DrumDetector -> Internal' section missing");
                return false;
            }

            auto internal = drumSection["Internal"];

            m_cameraIndex   = internal.value("CameraIndex", m_cameraIndex);
            m_trayWidth     = internal.value("TrayWidth", m_trayWidth);
            m_trayHeight    = internal.value("TrayHeight", m_trayHeight);
            m_minMarkerArea = internal.value("MinMarkerArea", m_minMarkerArea);
            m_maxMarkerArea = internal.value("MaxMarkerArea", m_maxMarkerArea);
            m_keepPercentage = internal.value("KeepPercentage", m_keepPercentage);
            if (!drumSection.contains("CurrentProfile") || !drumSection.contains("ProfileList"))
            {
                this->m_logger->error("[DrumDetectorConfig] Error: 'DrumDetector -> CurrentProfile / ProfileList' section missing");
                return false;
            }

            std::string targetProfile = drumSection["CurrentProfile"];
            bool profileFound = false;

            for (const auto& profile : drumSection["ProfileList"])
            {
                if (profile["name"] == targetProfile)
                {
                    m_name = targetProfile;
                    m_exposure         = profile["exposure"];
                    m_b_thresh_yellow  = profile["b_thresh_yellow"];
                    m_saturation_boost = profile["saturation_boost"];
                    m_blue_max         = profile["blue_max"];
                    m_pink_min         = profile["pink_min"];

                    profileFound = true;
                    break;
                }
            }

            if (!profileFound)
            {
                this->m_logger->error("[DrumDetectorConfig] Error: Profile '{}' not found in list.", targetProfile);
            }

            return profileFound;
        }

        catch (const nlohmann::json::exception& e)
        {
            this->m_logger->error("[DrumDetectorConfig] JSON Parse Error: {} ", e.what());
            return false;
        }
    }

    void DrumDetectorConfig::setLogger(std::shared_ptr<spdlog::logger> logger)
    {
        this->m_logger = std::move(logger);
        this->m_logger->info("[DrumDetectorConfig] logger set successfully!");
    }
}
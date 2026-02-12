// --- Includes --- //
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>
#include <stdexcept>
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

    void DrumDetectorConfig::load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            const std::string errMsg = "[DrumDetectorConfig] Could not open file: " + filePath;
            this->m_logger->error(errMsg);
            throw std::runtime_error(errMsg);
        }
        
        this->config_path = filePath;

        try
        {
            nlohmann::json config;
            file >> config;

            if (!config.contains("DrumDetector"))
            {
                throw std::runtime_error("[DrumDetectorConfig] 'DrumDetector' section missing in " + filePath);
            }

            auto drumSection = config["DrumDetector"];

            if (!drumSection.contains("Internal"))
            {
                throw std::runtime_error("[DrumDetectorConfig] 'Internal' section missing in JSON");
            }

            auto internal = drumSection["Internal"];

            m_cameraPath   = internal.value("CameraPath", m_cameraPath);
            m_trayWidth     = internal.value("TrayWidth", m_trayWidth);
            m_trayHeight    = internal.value("TrayHeight", m_trayHeight);
            m_minMarkerArea = internal.value("MinMarkerArea", m_minMarkerArea);
            m_maxMarkerArea = internal.value("MaxMarkerArea", m_maxMarkerArea);
            m_keepPercentage = internal.value("KeepPercentage", m_keepPercentage);
            if (!drumSection.contains("CurrentProfile") || !drumSection.contains("ProfileList"))
            {
                throw std::runtime_error("[DrumDetectorConfig] 'CurrentProfile' or 'ProfileList' missing");
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
                throw std::runtime_error("[DrumDetectorConfig] Profile '" + targetProfile + "' not found in list.");
            }
        }

        catch (const nlohmann::json::exception& e)
        {
            const std::string errMsg = "[DrumDetectorConfig] JSON Parse Error: " + std::string(e.what());
            this->m_logger->error(errMsg);
            throw std::runtime_error(errMsg);
        }
    }

    void DrumDetectorConfig::setLogger(std::shared_ptr<spdlog::logger> logger)
    {
        this->m_logger = std::move(logger);
        this->m_logger->info("[DrumDetectorConfig] logger set successfully!");
    }
}
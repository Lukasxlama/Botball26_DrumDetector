// --- Includes --- //
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "../include/DrumDetectorConfig.hpp"

// --- Code --- //
namespace DrumDetector::Types
{
    DrumDetectorConfig& DrumDetectorConfig::getInstance()
    {
        static DrumDetectorConfig instance;
        return instance;
    }

    bool DrumDetectorConfig::load(const std::string& filePath)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            std::cerr << "[DrumDetectorConfig] Error: Could not open " << filePath << std::endl;
            return false;
        }

        try
        {
            nlohmann::json config;
            file >> config;

            if (!config.contains("DrumDetector"))
            {
                std::cerr << "[DrumDetectorConfig] Error: 'DrumDetector' section missing" << std::endl;
                return false;
            }

            auto drumSection = config["DrumDetector"];

            if (!drumSection.contains("Internal"))
            {
                std::cerr << "[DrumDetectorConfig] Error: 'DrumDetector -> Internal' section missing" << std::endl;
                return false;
            }

            auto internal = drumSection["Internal"];

            m_cameraIndex   = internal.value("CameraIndex", m_cameraIndex);
            m_trayWidth     = internal.value("TrayWidth", m_trayWidth);
            m_trayHeight    = internal.value("TrayHeight", m_trayHeight);
            m_minMarkerArea = internal.value("MinMarkerArea", m_minMarkerArea);
            m_maxMarkerArea = internal.value("MaxMarkerArea", m_maxMarkerArea);

            if (!drumSection.contains("CurrentProfile") || !drumSection.contains("ProfileList"))
            {
                std::cerr << "[DrumDetectorConfig] Error: 'DrumDetector -> CurrentProfile / ProfileList' section missing" << std::endl;
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
                std::cerr << "[DrumDetectorConfig] Error: Profile '" << targetProfile << "' not found in list." << std::endl;
            }

            return profileFound;
        }

        catch (const nlohmann::json::exception& e)
        {
            std::cerr << "[DrumDetectorConfig] JSON Parse Error: " << e.what() << std::endl;
            return false;
        }
    }
}
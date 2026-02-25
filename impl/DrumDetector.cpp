// --- Includes --- //
#include <algorithm>
#include <cmath>
#include <thread>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include "../include/DrumDetector.hpp"
#include "../include/DrumDetectorConfig.hpp"

namespace DrumDetector
{
    DrumDetector& DrumDetector::getInstance()
    {
        static DrumDetector instance;
        return instance;
    }

    DrumDetector::DrumDetector() : config(Types::DrumDetectorConfig::getInstance())
    {
        this->init();
    }

    DrumDetector::~DrumDetector()
    {
        if (this->m_cap.isOpened())
        {
            this->m_cap.release();
        }
    }

    void DrumDetector::init()
    {
        if (this->m_cap.isOpened())
        {
            this->config.getLogger()->info("[DrumDetector] Closing existing camera connection.");
            this->m_cap.release();
        }

        this->config.getLogger()->info("[DrumDetector] Opening camera at path '{}'...", this->config.getCameraPath());
        this->m_cap.open(this->config.getCameraPath(), cv::CAP_V4L2);

        if (!this->m_cap.isOpened())
        {
            const std::string err = "[DrumDetector] Failed to open camera at path: " + this->config.getCameraPath();
            this->config.getLogger()->error(err);
            throw std::runtime_error(err);
        }

        this->config.getLogger()->info("[DrumDetector] Camera opened successfully.");

        this->m_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        this->config.getLogger()->debug("[DrumDetector] Set PROP_FOURCC value: MJPG");

        this->m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        this->config.getLogger()->debug("[DrumDetector] Set PROP_FRAME_WIDTH value: 1920");

        this->m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
        this->config.getLogger()->debug("[DrumDetector] Set PROP_FRAME_HEIGHT value: 1080");

        this->m_cap.set(cv::CAP_PROP_BRIGHTNESS, this->config.getBrightness());
        this->config.getLogger()->debug("[DrumDetector] Set PROP_BRIGHTNESS value: {}", this->config.getBrightness());

        this->m_cap.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
        this->config.getLogger()->debug("[DrumDetector] Set PROP AUTO_EXPOSURE value: 1");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        this->m_cap.set(cv::CAP_PROP_EXPOSURE, this->config.getExposure());
        this->config.getLogger()->debug("[DrumDetector] Set PROP_EXPOSURE value: {}", this->config.getExposure());

        this->config.getLogger()->debug("[DrumDetector] Camera initialized with {}x{}, exposure {} and brightness {}",
                                            1920, 1080, this->config.getExposure(), this->config.getBrightness());

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    cv::Mat DrumDetector::getSnapshot()
    {
        cv::Mat temp;
        for (int i = 0; i < 10; i++)
        {
            this->m_cap.read(temp);
        }
        this->m_cap.read(temp);

        if (temp.empty())
        {
            this->config.getLogger()->error("[DrumDetector] Failed to capture frame from camera!");
            return temp;
        }

        const double ratio = this->config.getKeepPercentage();
        const int newHeight = static_cast<int>(temp.rows * ratio);
        const int yStart = temp.rows - newHeight;

        this->config.getLogger()->debug("[DrumDetector] ROI applied: Keep bottom {}%", static_cast<int>(ratio * 100));

        const cv::Rect roi(0, yStart, temp.cols, newHeight);
        return temp(roi).clone();
    }

    cv::Mat DrumDetector::enhanceSaturation(const cv::Mat& src) const
    {
        cv::Mat lab;
        cv::cvtColor(src, lab, cv::COLOR_BGR2Lab);

        cv::Mat lut(1, 256, CV_8U);
        uint8_t* p = lut.ptr();
        const auto factor = static_cast<float>(this->config.getSaturationBoost());
        for (int i = 0; i < 256; ++i)
        {
            p[i] = cv::saturate_cast<uint8_t>(128.0f + (static_cast<float>(i) - 128.0f) * factor);
        }

        std::vector<cv::Mat> channels;
        cv::split(lab, channels);
        cv::LUT(channels[1], lut, channels[1]);
        cv::LUT(channels[2], lut, channels[2]);
        cv::merge(channels, lab);
        return lab;
    }

    Types::DrumColorList DrumDetector::getDrumColors()
    {
        Types::DrumColorList result;
        cv::Mat frame = getSnapshot();

        if (frame.empty())
        {
            this->config.getLogger()->warn("[DrumDetector] Snapshot failed - frame is empty.");
            return result;
        }

        std::filesystem::path configPath(this->config.getConfigPath());
        std::filesystem::path debugDir = configPath.parent_path() / "DrumDetectorDebug";

        std::filesystem::create_directories(debugDir);

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S");
        std::string timestamp = ss.str();

        cv::imwrite((debugDir / (timestamp + "_1_raw.png")).string(), frame);
        this->config.getLogger()->debug("[DrumDetector] Snapshot captured. Saving debug images to {}", debugDir.string());

        cv::Mat processed, mask;
        cv::GaussianBlur(frame, processed, cv::Size(5, 5), 0);
        cv::cvtColor(processed, processed, cv::COLOR_BGR2Lab);
        cv::inRange(processed, cv::Scalar(0, 0, this->config.getBThreshYellow()),
            cv::Scalar(255, 255, 255), mask);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<cv::Point2f> candidates;
        for (const auto& cnt : contours)
        {
            if (double area = cv::contourArea(cnt); area > this->config.getMinMarkerArea()
                && area < this->config.getMaxMarkerArea())
            {
                if (cv::Moments m = cv::moments(cnt); m.m00 != 0)
                    candidates.emplace_back(m.m10 / m.m00, m.m01 / m.m00);
            }
        }
        this->config.getLogger()->trace("[DrumDetector] Found {} raw contours.", contours.size());

        if (candidates.size() < 4)
        {
            this->config.getLogger()->warn("[DrumDetector] Not enough marker candidates! Found {}, need 4.", candidates.size());
            return result;
        }

        std::vector<cv::Point2f> best_pts;
        double max_area = 0;
        for (size_t i = 0; i < candidates.size(); i++)
        {
            for (size_t j = i + 1; j < candidates.size(); j++)
            {
                for (size_t k = j + 1; k < candidates.size(); k++)
                {
                    for (size_t l = k + 1; l < candidates.size(); l++)
                    {
                        if (std::vector quad = {candidates[i], candidates[j], candidates[k], candidates[l]}; checkShape(quad))
                        {
                            quad = sortRadial(quad);
                            if (double a = cv::contourArea(quad); a > max_area) { max_area = a; best_pts = quad; }
                        }
                    }
                }
            }
        }

        if (best_pts.empty())
        {
            this->config.getLogger()->warn("[DrumDetector] Geometry check failed: No valid tray-shaped quadrilateral found "
                               "among {} candidates.", candidates.size());
            return result;
        }

        this->config.getLogger()->info("[DrumDetector] Tray detected! Processing color slots...");

        cv::Mat warped;
        cv::Point2f dst_pts[4] = {
            {0, 0},
            {static_cast<float>(this->config.getTrayWidth()), 0},
            {static_cast<float>(this->config.getTrayWidth()), static_cast<float>(this->config.getTrayHeight())},
            {0, static_cast<float>(this->config.getTrayHeight())}
        };
        cv::Mat trans = cv::getPerspectiveTransform(best_pts.data(), dst_pts);
        cv::warpPerspective(frame, warped, trans, cv::Size(this->config.getTrayWidth(),
            this->config.getTrayHeight()));

        cv::Mat final_lab = enhanceSaturation(warped);

        cv::Mat debugWarp;
        cv::cvtColor(final_lab, debugWarp, cv::COLOR_Lab2BGR);
        cv::imwrite((debugDir / (timestamp + "_2_warped_boosted.png")).string(), debugWarp);

        std::vector<cv::Mat> chs;
        cv::split(final_lab, chs);

        int slot_w = this->config.getTrayWidth() / 8;
        for (int i = 0; i < 8; i++)
        {
            cv::Rect roi(i * slot_w + 40, 40, slot_w - 80, this->config.getTrayHeight() - 80);
            int a = getMedian(chs[1](roi));
            int b = getMedian(chs[2](roi));

            if (b < this->config.getBlueMax())
                result.items.push_back(Types::DrumColor::Blue);
            else if (a > this->config.getPinkMin())
                result.items.push_back(Types::DrumColor::Pink);
            else
                result.items.push_back(Types::DrumColor::Empty);
        }

        return result;
    }

    std::vector<cv::Point2f> DrumDetector::sortRadial(std::vector<cv::Point2f> pts)
    {
        cv::Point2f center(0, 0);
        for (const auto& p : pts) center += p;
        center.x /= static_cast<float>(pts.size());
        center.y /= static_cast<float>(pts.size());

        std::sort(pts.begin(), pts.end(), [center](const cv::Point2f& a, const cv::Point2f& b) {
            return std::atan2(a.y - center.y, a.x - center.x) < std::atan2(b.y - center.y, b.x - center.x);
        });
        return pts;
    }

    bool DrumDetector::checkShape(std::vector<cv::Point2f> pts)
    {
        if (pts.size() != 4) return false;
        pts = sortRadial(pts);

        const double d1 = cv::norm(pts[0] - pts[1]);
        const double d2 = cv::norm(pts[1] - pts[2]);
        const double d3 = cv::norm(pts[2] - pts[3]);
        const double d4 = cv::norm(pts[3] - pts[0]);

        const double width = (d1 + d3) / 2.0;
        const double height = (d2 + d4) / 2.0;

        if (height < 5.0) return false;

        if (const double ratio = width / height; ratio < 2.5 || ratio > 6.0)
        {
            this->config.getLogger()->trace("[DrumDetector] Shape rejected: Aspect ratio {:.2f} out of bounds.", ratio);
            return false;
        }

        if (std::abs(d1 - d3) > (width * 0.3)) return false;

        return true;
    }

    int DrumDetector::getMedian(const cv::Mat& channel)
    {
        if (channel.empty()) return 128;

        const cv::Mat continuous = channel.clone();

        std::vector<uint8_t> vec;
        if (continuous.isContinuous()) {
            vec.assign(continuous.datastart, continuous.dataend);
        } else {
            continuous.copyTo(vec);
        }

        const auto m = vec.begin() + static_cast<long>(vec.size()) / 2;
        std::nth_element(vec.begin(), m, vec.end());
        return *m;
    }
}
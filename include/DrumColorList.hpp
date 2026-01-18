// --- Include Guard --- //
#pragma once

// --- Includes --- //
#include <string>
#include <vector>
#include "DrumColor.hpp"

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
     * @brief Structure to hold a list of detected drum colors.
     */
    struct DrumColorList
    {
        /** @brief The vector containing the 8 detected colors. */
        std::vector<DrumColor> items;

        /**
         * @brief Serializes the list to a Python-style string format.
         * @return std::string Formatted string, e.g., "['blue', 'pink', 'empty']".
         */
        [[nodiscard]] std::string toString() const;
    };
}
// --- Include Guards --- //
#pragma once

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
     * @brief Represents the possible colors of a drum.
     */
    enum class DrumColor
    {
        Blue,   ///< A blue drum.
        Pink,   ///< A pink drum.
        Empty   ///< Section is neither blue nor pink.
    };
}

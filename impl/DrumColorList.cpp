// --- Includes --- //
#include "../include/DrumColorList.hpp"

// --- Code --- //
namespace DrumDetector::Types
{
    std::string DrumColorList::toString() const
    {
        std::string s = "[";
        for (const DrumColor& item : items)
        {
            switch (item)
            {
                 case DrumColor::Blue:
                    s += "'blue', ";
                    break;

                case DrumColor::Pink:
                    s += "'pink', ";
                    break;

                case DrumColor::Empty:
                default:
                    s += "'empty', ";
                    break;
            }
        }

        if (!items.empty())
        {
            s.pop_back();
            s.pop_back();
        }

        s += "]";
        return s;
    }
}
#include "color_diff.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

int main(int argc, char *argv[])
{
    if (argc == 1 || argc > 3)
    {
        std::cerr << "Usage: make_palette num_colors [config_file]"
            << std::endl;
        return 0;
    }

    // Get number of colors to generate.
    long num_colors = std::strtol(argv[1], 0, 10);

    // Read LCh(ab) filter and initial palette if specified.
    color::PaletteGenerator palette;
    color::Color next;
    double min_L = 0;
    double max_L = 100;
    double min_C = 0;
    double max_C = 150;
    double min_h = 0;
    double max_h = 360;
    if (argc == 3)
    {
        std::ifstream config_file(argv[2]);
        config_file >> min_L >> max_L;
        config_file >> min_C >> max_C;
        config_file >> min_h >> max_h;
        palette.reset(min_L, max_L, min_C, max_C, min_h, max_h);
        unsigned red, green, blue;
        while (config_file >> red >> green >> blue)
        {
            next = palette.add(color::Color(red, green, blue));
        }
    }
    if (next.min_delta == std::numeric_limits<double>::infinity())
    {
        next = palette.add(color::Color(255, 255, 255));
    }
    
    // Generate palette.
    for (long n = 0; n < num_colors; ++n)
    {
        std::cout << next.min_delta << " "
            << static_cast<unsigned>(next.rgb[0]) << " "
            << static_cast<unsigned>(next.rgb[1]) << " "
            << static_cast<unsigned>(next.rgb[2]) << std::endl;
        if (n < num_colors)
        {
            next = palette.add(next);
        }
    }
}

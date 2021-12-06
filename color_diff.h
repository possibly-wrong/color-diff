#ifndef COLOR_DIFF_H
#define COLOR_DIFF_H

#include <cmath>
#include <limits>
#include <vector>

namespace color
{
    // Convert sRGB color in [0..255]^3 to CIE-XYZ.
    void srgb_to_xyz(const unsigned char *srgb, double *xyz)
    {
        double rgb[3] = {0};
        for (int channel = 0; channel < 3; ++channel)
        {
            double c = srgb[channel] / 255.0;
            if (c > 0.04045)
            {
                c = std::pow((c + 0.055) / 1.055, 2.4);
            }
            else
            {
                c = c / 12.92;
            }
            rgb[channel] = 100 * c;
        }
        xyz[0] = rgb[0] * 0.4124 + rgb[1] * 0.3576 + rgb[2] * 0.1805;
        xyz[1] = rgb[0] * 0.2126 + rgb[1] * 0.7152 + rgb[2] * 0.0722;
        xyz[2] = rgb[0] * 0.0193 + rgb[1] * 0.1192 + rgb[2] * 0.9505;
    }

    // Convert CIE-XYZ color to CIE-Lab.
    void xyz_to_lab(const double *xyz, double *lab)
    {
        const double delta = 6.0 / 29;
        const double reference[3] = { 95.047, 100.0, 108.883 }; // D65 (2 deg)
        double f[3] = {0};
        for (int channel = 0; channel < 3; ++channel)
        {
            double c = xyz[channel] / reference[channel];
            if (c > std::pow(delta, 3))
            {
                f[channel] = pow(c, 1.0 / 3);
            }
            else
            {
                f[channel] = c / (3 * delta * delta) + 4.0 / 29;
            }
        }
        lab[0] = 116 * f[1] - 16;
        lab[1] = 500 * (f[0] - f[1]);
        lab[2] = 200 * (f[1] - f[2]);
    }

    // Return CIEDE2000 color difference.
    double diff_de00(const double *lab1, const double *lab2)
    {
        const double kL = 1, kC = 1, kH = 1;
        double L1 = lab1[0], a1 = lab1[1], b1 = lab1[2];
        double L2 = lab2[0], a2 = lab2[1], b2 = lab2[2];

        double C1 = std::sqrt(a1 * a1 + b1 * b1);
        double C2 = std::sqrt(a2 * a2 + b2 * b2);
        double Cbar7 = std::pow((C1 + C2) / 2, 7);
        double G = 0.5 * (1 - std::sqrt(Cbar7 / (Cbar7 + std::pow(25.0, 7))));
        double ap1 = (1 + G) * a1;
        double ap2 = (1 + G) * a2;
        double Cp1 = std::sqrt(ap1 * ap1 + b1 * b1);
        double Cp2 = std::sqrt(ap2 * ap2 + b2 * b2);
        double hp1 = std::atan2(b1, ap1);
        const double pi = 3.141592653589793;
        if (hp1 < 0)
        {
            hp1 += 2 * pi;
        }
        double hp2 = std::atan2(b2, ap2);
        if (hp2 < 0)
        {
            hp2 += 2 * pi;
        }

        double dLp = L2 - L1;
        double dCp = Cp2 - Cp1;
        double dh = hp2 - hp1;
        if (dh > pi)
        {
            dh -= 2 * pi;
        }
        else if (dh < -pi)
        {
            dh += 2 * pi;
        }
        double dHp = 2 * std::sqrt(Cp1 * Cp2) * std::sin(dh / 2);

        double Lpbar = (L1 + L2) / 2;
        double Cpbar = (Cp1 + Cp2) / 2;
        double Hpbar = (hp1 + hp2) / 2;
        if (std::abs(hp1 - hp2) > pi)
        {
            Hpbar -= pi;
        }
        if (Hpbar < 0)
        {
            Hpbar += 2 * pi;
        }
        if (Cp1 * Cp2 == 0)
        {
            Hpbar = hp1 + hp2;
        }
        Hpbar *= 180 / pi;

        double T = (1
            - 0.17 * std::cos(pi / 180 * (Hpbar - 30))
            + 0.24 * std::cos(pi / 180 * (2 * Hpbar))
            + 0.32 * std::cos(pi / 180 * (3 * Hpbar + 6))
            - 0.20 * std::cos(pi / 180 * (4 * Hpbar - 63)));
        double angle = pi / 6 * std::exp(-std::pow((Hpbar - 275) / 25, 2));
        double Cpbar7 = std::pow(Cpbar, 7);
        double RC = 2 * std::sqrt(Cpbar7 / (Cpbar7 + std::pow(25, 7)));
        double Lpbar502 = std::pow(Lpbar - 50, 2);
        double SL = 1 + 0.015 * Lpbar502 / std::sqrt(20 + Lpbar502);
        double SC = 1 + 0.045 * Cpbar;
        double SH = 1 + 0.015 * Cpbar * T;
        double RT = -std::sin(2 * angle) * RC;
        double x = dLp / (kL * SL);
        double y = dCp / (kC * SC);
        double z = dHp / (kH * SH);
        return std::sqrt(x * x + y * y + z * z + RT * y * z);
    }

    // Color specified in sRGB and CIE-Lab coordinates.
    // When used with PaletteGenerator, min_delta (default +inf) is the
    // minimum CIEDE2000 distance to the colors already in the palette.
    struct Color
    {
        Color(unsigned red = 0, unsigned green = 0, unsigned blue = 0) :
            min_delta(std::numeric_limits<double>::infinity())
        {
            rgb[0] = red;
            rgb[1] = green;
            rgb[2] = blue;
            double xyz[3] = {0};
            srgb_to_xyz(rgb, xyz);
            xyz_to_lab(xyz, lab);
        }
        unsigned char rgb[3];
        double lab[3];
        double min_delta;
    };

    // Glasbey sequential algorithm to generate color palette.
    class PaletteGenerator
    {
    public:
        PaletteGenerator() :
            colors()
        {
            for (unsigned red = 0; red < 256; ++red)
            {
                for (unsigned green = 0; green < 256; ++green)
                {
                    for (unsigned blue = 0; blue < 256; ++blue)
                    {
                        colors.push_back(Color(red, green, blue));
                    }
                }
            }
        }

        // Reset this generator with the given CIE-LCh(ab) filter.
        void reset(
            double min_L = 0, double max_L = 100,
            double min_C = 0, double max_C = 150,
            double min_h = 0, double max_h = 360)
        {
            for (auto&& color : colors)
            {
                double chroma = std::sqrt(color.lab[1] * color.lab[1]
                    + color.lab[2] * color.lab[2]);
                const double pi = 3.141592653589793;
                double hue = std::atan2(
                    color.lab[2], color.lab[1]) * 180 / pi;
                if (hue < 0)
                {
                    hue += 360;
                }
                bool reject = (color.lab[0] < min_L || color.lab[0] > max_L
                    || chroma < min_C || chroma > max_C
                    || ((min_h <= max_h) ? (hue < min_h || hue > max_h)
                                         : (hue < min_h && hue > max_h)));
                color.min_delta =
                    (reject ? 0 : std::numeric_limits<double>::infinity());
            }
        }

        // Add new color to palette, returning next available color maximizing
        // minimum CIEDE2000 distance from the current palette.
        Color add(const Color& color)
        {
            Color next;
            next.min_delta = 0;
            for (auto&& candidate : colors)
            {
                if (candidate.min_delta > 0)
                {
                    double delta = diff_de00(color.lab, candidate.lab);
                    if (delta < candidate.min_delta)
                    {
                        candidate.min_delta = delta;
                    }
                    if (candidate.min_delta > next.min_delta)
                    {
                        next = candidate;
                    }
                }
            }
            return next;
        }

    protected:
        std::vector<Color> colors;
    };
} // namespace color

#endif // COLOR_DIFF_H

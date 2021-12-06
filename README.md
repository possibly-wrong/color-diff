Color Differences using CIEDE2000
=================================

`color_diff.h` implements the [CIEDE2000](https://en.wikipedia.org/wiki/Color_difference#CIEDE2000)
formula for the difference between reference and sample colors, specified
in CIE-LAB, or via included conversions from sRGB or CIE-XYZ (D65 / 2 deg).

This header also implements the greedy heuristic described by Glasbey for
generating a palette of visually distinguishable colors, by iteratively
selecting a color maximizing the minimum CIEDE2000 distance from the colors
already in the palette.

`make_palette.cpp` demonstrates use of these functions to generate such a
palette of visually distinguishable colors. To use,

```
make_palette num_colors [config_file]
```

where `num_colors` specifies the number of colors to generate, and the
optional text file `config_file` specifies minimum and maximum values of
CIE-LCh(ab) lightness, chroma, and hue, followed by a list of sRGB colors,
which may be interpreted as either an initial palette to be extended, or as
a set of colors to be "avoided" when generating the palette. The default
configuration is to allow all CIE-LCh(ab) values, starting with white:

```
0 100
0 150
0 360
255 255 255
```

The output is a list of the requested number of colors, one color per line,
each specified by its minimum CIEDE2000 distance from the colors generated
so far, followed by its sRGB values.

References
----------

1. Glasbey, C., van der Heijden, G., Toh, V., and Gray, A., [Colour
   Displays for Categorical Images](https://doi.org/10.1002/col.20327),
   *Color Research and Application*, **32**(4) 2007, 304-309
2. Sharma, G., Wu, W., and Dalal, E., [The CIEDE2000 Color-Difference
   Formula: Implementation Notes, Supplementary Test Data, and Mathematical
   Observations](https://doi.org/10.1002/col.20070), *Color Research and
   Application*, **30**(1) 2005, 21-30

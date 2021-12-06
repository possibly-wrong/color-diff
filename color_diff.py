import math

def srgb_to_xyz(srgb):
    """Convert sRGB color in [0..255]^3 to CIE-XYZ."""
    rgb = [c / 255 for c in srgb]
    rgb = [math.pow((c + 0.055) / 1.055, 2.4) if c > 0.04045 else (c / 12.92)
           for c in rgb]
    r, g, b = [100 * c for c in rgb]
    return (r * 0.4124 + g * 0.3576 + b * 0.1805,
            r * 0.2126 + g * 0.7152 + b * 0.0722,
            r * 0.0193 + g * 0.1192 + b * 0.9505)

def xyz_to_lab(xyz, reference=(95.047, 100.0, 108.883)):
    """Convert CIE-XYZ color to CIE-Lab."""
    delta = 6 / 29
    f = [xyz[k] / reference[k] for k in range(3)]
    x, y, z = [math.pow(t, 1 / 3) if t > math.pow(delta, 3)
               else (t / (3 * delta * delta) + 4 / 29)
               for t in f]
    return (116 * y - 16, 500 * (x - y), 200 * (y - z))

def diff_de00(lab1, lab2, kL=1, kC=1, kH=1):
    """Return CIEDE2000 color difference."""
    L1, a1, b1 = lab1
    L2, a2, b2 = lab2

    C1 = math.sqrt(a1 * a1 + b1 * b1)
    C2 = math.sqrt(a2 * a2 + b2 * b2)
    Cbar7 = math.pow((C1 + C2) / 2, 7)
    G = 0.5 * (1 - math.sqrt(Cbar7 / (Cbar7 + math.pow(25, 7))))
    ap1 = (1 + G) * a1
    ap2 = (1 + G) * a2
    Cp1 = math.sqrt(ap1 * ap1 + b1 * b1)
    Cp2 = math.sqrt(ap2 * ap2 + b2 * b2)
    hp1 = math.atan2(b1, ap1)
    if hp1 < 0:
        hp1 = hp1 + 2 * math.pi
    hp2 = math.atan2(b2, ap2)
    if hp2 < 0:
        hp2 = hp2 + 2 * math.pi

    dLp = L2 - L1
    dCp = Cp2 - Cp1
    dh = hp2 - hp1
    if dh > math.pi:
        dh = dh - 2 * math.pi
    elif dh < -math.pi:
        dh = dh + 2 * math.pi
    dHp = 2 * math.sqrt(Cp1 * Cp2) * math.sin(dh / 2)

    Lpbar = (L1 + L2) / 2
    Cpbar = (Cp1 + Cp2) / 2
    Hpbar = (hp1 + hp2) / 2
    if abs(hp1 - hp2) > math.pi:
        Hpbar = Hpbar - math.pi
    if Hpbar < 0:
        Hpbar = Hpbar + 2 * math.pi
    if Cp1 * Cp2 == 0:
        Hpbar = hp1 + hp2
    Hpbar = math.degrees(Hpbar)
    
    T = (1
         - 0.17 * math.cos(math.radians(Hpbar - 30))
         + 0.24 * math.cos(math.radians(2 * Hpbar))
         + 0.32 * math.cos(math.radians(3 * Hpbar + 6))
         - 0.20 * math.cos(math.radians(4 * Hpbar - 63)))
    angle = math.radians(30) * math.exp(-math.pow((Hpbar - 275) / 25, 2))
    Cpbar7 = math.pow(Cpbar, 7)
    RC = 2 * math.sqrt(Cpbar7 / (Cpbar7 + math.pow(25, 7)))
    Lpbar502 = math.pow(Lpbar - 50, 2)
    SL = 1 + 0.015 * Lpbar502 / math.sqrt(20 + Lpbar502)
    SC = 1 + 0.045 * Cpbar
    SH = 1 + 0.015 * Cpbar * T
    RT = -math.sin(2 * angle) * RC
    (x, y, z) = (dLp / (kL * SL), dCp / (kC * SC), dHp / (kH * SH))
    return math.sqrt(x * x + y * y + z * z + RT * y * z)

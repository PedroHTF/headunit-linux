#!/usr/bin/env python3
"""
Gera o logo padrão da inicialização: "HR-V" em estilo cromado (letras desenhadas aqui,
itálicas e largas), com o reflexo em terracota, fundo preto e um asterisco em forma de
"faísca" no fim. Uso pessoal.

    python3 tools/make_logo.py saida.png [largura altura]

Requer Pillow e NumPy. Mesmo arquivo nos repositórios headunit-linux e headunit-android.
"""
import math
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

OUT = sys.argv[1] if len(sys.argv) > 1 else "boot_logo.png"
W = int(sys.argv[2]) if len(sys.argv) > 2 else 1920
H = int(sys.argv[3]) if len(sys.argv) > 3 else 1080
SS = 2                      # supersampling
CW, CH = W * SS, H * SS
CAP = CH * 0.20             # altura das letras
SLANT = math.tan(math.radians(14))
TERRACOTTA = np.array([217, 119, 87], float)

# ---------------------------------------------------------------- letras (unidade = altura)
T = 0.22  # espessura das hastes


def letter_H(x):
    w = 1.40
    return [("rect", x, 0, x + T * 1.1, 1), ("rect", x + w - T * 1.1, 0, x + w, 1),
            ("rect", x, 0.41, x + w, 0.61)], w


def letter_R(x):
    w = 1.30
    shapes = [("rrect", x, 0, x + w, 0.60, 0.26, "right"),
              ("clear_rrect", x + T * 1.1, 0.19, x + w - T, 0.42, 0.10, "right"),
              ("rect", x, 0, x + T * 1.1, 1),
              ("poly", [(x + 0.50, 0.42), (x + 0.86, 0.42), (x + 1.38, 1.0), (x + 1.00, 1.0)])]
    return shapes, w


def letter_dash(x):
    w = 0.52
    return [("rect", x, 0.47, x + w, 0.63)], w


def letter_V(x):
    w = 1.45
    return [("poly", [(x, 0), (x + 0.30, 0), (x + 0.50, 0.70), (x + 1.17, 0), (x + 1.45, 0),
                      (x + 0.62, 1.0), (x + 0.20, 1.0)])], w


def layout():
    shapes, x = [], 0.0
    for fn, gap in ((letter_H, 0.14), (letter_R, 0.10), (letter_dash, 0.10), (letter_V, 0.0)):
        s, w = fn(x)
        shapes += s
        x += w + gap
    return shapes, x  # largura total sem inclinação


def to_px(ox, oy, x, y):
    """Unidades → pixels, com a inclinação itálica (topo vai para a direita)."""
    return ox + (x + (1 - y) * SLANT) * CAP, oy + y * CAP


def draw_letters(mask, ox, oy):
    d = ImageDraw.Draw(mask)
    for s in layout()[0]:
        kind = s[0]
        if kind in ("rect",):
            _, x0, y0, x1, y1 = s
            d.polygon([to_px(ox, oy, *p) for p in ((x0, y0), (x1, y0), (x1, y1), (x0, y1))], fill=255)
        elif kind == "poly":
            d.polygon([to_px(ox, oy, *p) for p in s[1]], fill=255)
        elif kind in ("rrect", "clear_rrect"):
            _, x0, y0, x1, y1, r, _side = s
            pts = []
            pts += [(x0, y0), (x1 - r, y0)]
            pts += [(x1 - r + r * math.sin(a), y0 + r - r * math.cos(a)) for a in np.linspace(0, math.pi / 2, 12)]
            pts += [(x1 - r + r * math.cos(a), y1 - r + r * math.sin(a)) for a in np.linspace(0, math.pi / 2, 12)]
            pts += [(x0, y1)]
            d.polygon([to_px(ox, oy, *p) for p in pts], fill=0 if kind.startswith("clear") else 255)


# ---------------------------------------------------------------- faísca (asterisco)
def draw_spark(mask, cx, cy, r):
    """Asterisco orgânico: raios afunilados de comprimentos variados (desenho original)."""
    d = ImageDraw.Draw(mask)
    rays = [(-92, 1.00), (-48, 0.80), (-8, 0.96), (38, 0.74), (82, 1.00),
            (126, 0.82), (171, 0.97), (214, 0.72)]
    for deg, length in rays:
        a = math.radians(deg)
        ca, sa = math.cos(a), math.sin(a)
        inner, outer = r * 0.10, r * length
        w = r * 0.13 * (0.85 + 0.3 * length)
        left, right = [], []
        for k in range(17):
            t = k / 16
            rr = inner + (outer - inner) * t
            half = w * (0.45 + 0.55 * math.sin(t * math.pi / 2))
            left.append((cx + rr * ca - half * sa, cy + rr * sa + half * ca))
            right.append((cx + rr * ca + half * sa, cy + rr * sa - half * ca))
        d.polygon(left + right[::-1], fill=255)
        tx, ty = cx + outer * ca, cy + outer * sa
        d.ellipse([tx - w, ty - w, tx + w, ty + w], fill=255)
    d.ellipse([cx - r * 0.24, cy - r * 0.24, cx + r * 0.24, cy + r * 0.24], fill=255)


# ---------------------------------------------------------------- material cromado
def lerp(a, b, t):
    return a + (b - a) * t[..., None]


def chrome(mask_arr, top, cap):
    """Cromo: "céu" claro em cima, horizonte nítido e "chão" refletido em terracota."""
    hgt, wid = mask_arr.shape
    yy, xx = np.mgrid[0:hgt, 0:wid].astype(float)
    # Horizonte levemente ondulado, como o reflexo num metal real.
    horizon = 0.5 + 0.035 * np.sin(xx / wid * 9.0) + 0.02 * np.sin(xx / wid * 23.0 + 1.3)
    v = np.clip((yy - top) / cap, 0, 1)
    v = np.where(v < horizon, v / horizon * 0.5, 0.5 + (v - horizon) / (1 - horizon) * 0.5)
    light_top = np.array([250, 250, 252], float)
    light_mid = np.array([170, 172, 178], float)
    dark = TERRACOTTA * 0.55
    mid = TERRACOTTA
    low = np.array([245, 190, 165], float)
    sky = lerp(light_top, light_mid, np.clip(v / 0.5, 0, 1) ** 1.4)
    ground_t = np.clip((v - 0.5) / 0.5, 0, 1)
    ground = np.where(ground_t[..., None] < 0.45,
                      lerp(dark, mid, ground_t / 0.45),
                      lerp(mid, low, (ground_t - 0.45) / 0.55))
    col = np.where((v < 0.5)[..., None], sky, ground)
    # Brilhos diagonais, como reflexos de luz no metal.
    for c, wdt, k in ((0.18, 0.025, 0.45), (0.55, 0.05, 0.25), (0.83, 0.02, 0.35)):
        s = np.exp(-(((xx / wid) - (yy / hgt) * 0.35 - c) / wdt) ** 2) * k
        col = col + (255 - col) * s[..., None]
    return col


def bevel(mask):
    """Sombreamento da borda (luz de cima/esquerda) e contorno claro."""
    m = np.asarray(mask, float) / 255
    blur = np.asarray(mask.filter(ImageFilter.GaussianBlur(CAP * 0.025)), float) / 255
    gy, gx = np.gradient(blur)
    norm = np.hypot(gx, gy) + 1e-9
    light = np.array([-0.45, -0.89])
    shade = -(gx * light[0] + gy * light[1]) / norm  # -1..1
    inner = np.asarray(mask.filter(ImageFilter.MinFilter(int(CAP * 0.045) | 1)), float) / 255
    rim = np.clip(m - inner, 0, 1)
    return m, rim, shade, inner


def main():
    shapes, total_w = layout()
    spark_r = CAP * 0.40
    group_w = (total_w + SLANT) * CAP + spark_r * 2.2
    ox = (CW - group_w) / 2
    oy = CH * 0.46 - CAP / 2

    mask = Image.new("L", (CW, CH), 0)
    draw_letters(mask, ox, oy)
    m, rim, shade, inner = bevel(mask)
    col = chrome(m, oy, CAP)
    rim_col = np.clip(col * (0.75 + 0.55 * shade[..., None]) + 40 * np.clip(shade, 0, 1)[..., None], 0, 255)
    letters = col * (1 - rim[..., None]) + rim_col * rim[..., None]
    # Linha escura entre a borda e a face, para dar profundidade.
    groove = np.clip(inner - np.asarray(Image.fromarray((inner * 255).astype(np.uint8))
                                        .filter(ImageFilter.MinFilter(5)), float) / 255, 0, 1)
    letters = letters * (1 - 0.45 * groove[..., None])

    # Faísca: terracota sólida com leve luz no centro.
    spark_mask = Image.new("L", (CW, CH), 0)
    sx = ox + (total_w + SLANT * 0.85) * CAP + spark_r * 1.15
    sy = oy + spark_r * 0.55
    draw_spark(spark_mask, sx, sy, spark_r)
    sm = np.asarray(spark_mask, float) / 255
    yy, xx = np.mgrid[0:CH, 0:CW].astype(float)
    glow = np.clip(1 - np.hypot(xx - sx, yy - sy) / (spark_r * 1.1), 0, 1)
    spark_col = TERRACOTTA + (np.array([250, 180, 150]) - TERRACOTTA) * (glow ** 2 * 0.6)[..., None]

    bg = np.zeros((CH, CW, 3)) + 4
    img = bg * (1 - m[..., None]) + letters * m[..., None]
    img = img * (1 - sm[..., None]) + spark_col * sm[..., None]

    # Reflexo no "chão" preto, bem discreto.
    base = int(oy + CAP)
    span = int(CAP * 0.55)
    if base + span < CH:
        alpha_mask = np.maximum(m, sm)
        refl = img[base - span:base][::-1] * alpha_mask[base - span:base][::-1][..., None]
        fade = (np.linspace(0.22, 0, span) ** 1.5)[:, None, None]
        img[base + 6:base + 6 + span] += refl * fade

    out = Image.fromarray(np.clip(img, 0, 255).astype(np.uint8)).resize((W, H), Image.LANCZOS)
    out.save(OUT)
    print("Logo salvo em", OUT)


if __name__ == "__main__":
    main()

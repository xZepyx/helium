#!/usr/bin/env python3
"""Generate simple placeholder images for Helium services using Pillow."""
import os
from PIL import Image, ImageDraw, ImageFont

SERVICES = [
    "applicationservice", "audioservice", "backlightservice", "bluetoothservice",
    "hyprlandservice", "mprisservice", "networkservice", "notificationservice",
    "powerprofilesservice", "systemtrayservice", "wallpaperservice",
]

COLORS = {
    "applicationservice": "#4fae9a",
    "audioservice": "#e67e22",
    "backlightservice": "#f1c40f",
    "bluetoothservice": "#3498db",
    "hyprlandservice": "#9b59b6",
    "mprisservice": "#e74c3c",
    "networkservice": "#2ecc71",
    "notificationservice": "#1abc9c",
    "powerprofilesservice": "#34495e",
    "systemtrayservice": "#7f8c8d",
    "wallpaperservice": "#16a085",
}

ICONS = {
    "applicationservice": "App",
    "audioservice": "♪",
    "backlightservice": "☀",
    "bluetoothservice": "B",
    "hyprlandservice": "H",
    "mprisservice": "▶",
    "networkservice": "◉",
    "notificationservice": "🔔",
    "powerprofilesservice": "⚡",
    "systemtrayservice": "⊞",
    "wallpaperservice": "🖼",
}

OUT = os.path.join(os.path.dirname(__file__), "assets", "services")
os.makedirs(OUT, exist_ok=True)

W, H = 400, 200

for svc in SERVICES:
    color = COLORS[svc]
    img = Image.new("RGBA", (W, H), (30, 33, 35, 255))
    draw = ImageDraw.Draw(img)

    # Draw a colored bar at the top
    r, g, b = int(color[1:3], 16), int(color[3:5], 16), int(color[5:7], 16)
    for y in range(0, 6):
        for x in range(0, W):
            alpha = int(255 * (1 - y / 5))
            img.putpixel((x, y), (r, g, b, alpha))

    # Try to use a font
    try:
        font_large = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48)
        font_small = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18)
    except:
        font_large = ImageFont.load_default()
        font_small = ImageFont.load_default()

    # Icon
    icon = ICONS[svc]
    bbox = draw.textbbox((0, 0), icon, font=font_large)
    iw, ih = bbox[2] - bbox[0], bbox[3] - bbox[1]
    draw.text(((W - iw) / 2, (H - ih) / 2 - 20), icon, fill=color, font=font_large)

    # Label
    name = svc.replace("service", "").title()
    bbox = draw.textbbox((0, 0), name, font=font_small)
    tw = bbox[2] - bbox[0]
    draw.text(((W - tw) / 2, (H + ih) / 2 + 10), name, fill="#a0a4a1", font=font_small)

    dest = os.path.join(OUT, f"{svc}.png")
    img.save(dest)
    print(f"  {svc}.png")

print("Done generating service images.")

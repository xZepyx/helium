#!/usr/bin/env python3
"""Download GTK4 widget screenshots for Helium type pages."""
import os, sys, urllib.request, time

BASE = "https://docs.gtk.org/gtk4"

# Maps helium type filenames (without .md) -> GTK4 image filenames
TYPE_IMAGES = {
    "arrow": None,  # not a GTK4 widget
    "arrowbutton": "menu-button.png",
    "box": "box.png",
    "button": "button.png",
    "calendar": "calendar.png",
    "centerbox": "centerbox.png",
    "checkbutton": "check-button.png",
    "corner": None,
    "dropdown": "dropdown.png",
    "entry": "entry.png",
    "eventbox": None,  # GTK3 only
    "fixed": "fixed.png",
    "grid": "grid.png",
    "headerbar": "headerbar.png",
    "icon": None,
    "image": "image.png",
    "label": "label.png",
    "listbox": "list-box.png",
    "listboxrow": None,
    "materialsymbol": None,
    "overlay": "overlay.png",
    "panel": None,
    "picture": "picture.png",
    "popovermenu": "menu-button.png",
    "regularwindow": "window.png",
    "revealer": "revealer.png",
    "revealerwindow": "window.png",
    "scale": "scales.png",
    "scrolledwindow": "scrolledwindow.png",
    "separator": "separator.png",
    "spinbutton": "spinbutton.png",
    "stack": "stack.png",
    "stackswitcher": "stack-switcher.png",
    "switch": "switch.png",
    "togglebutton": "toggle-button.png",
    "widget": None,
    "window": "window.png",
}

OUT = os.path.join(os.path.dirname(__file__), "assets", "types")
os.makedirs(OUT, exist_ok=True)

downloaded = 0
failed = 0
skipped = 0

for name, img in TYPE_IMAGES.items():
    if img is None:
        skipped += 1
        continue
    url = f"{BASE}/{img}"
    dest = os.path.join(OUT, f"{name}.png")
    if os.path.exists(dest):
        print(f"  EXISTS {name}.png")
        downloaded += 1
        continue
    try:
        urllib.request.urlretrieve(url, dest)
        print(f"  OK  {name}.png <- {url}")
        downloaded += 1
        time.sleep(0.3)
    except Exception as e:
        print(f"  FAIL {name}.png ({e})")
        failed += 1

print(f"\nDone: {downloaded} downloaded, {failed} failed, {skipped} skipped")

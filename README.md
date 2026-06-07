# Helium 

Helium (gtk4) lets you build desktop shell widgets with Python/C++ and GTK4. Bars, dashboards, notification popups, control panels — anything that sits on your desktop via the layer-shell protocol.

> [!CAUTION]
This version has been deprecated and no support will be provided. It still works but I would suggest ignis or fabric.

### Features
* All gtk4 widgets wrapped.
* Hot-reloading css. (config to be hot-reloaded soon)
* Compositor IPCs.
* System Services Wrapper.

## Getting Started

### Prerequisites
* gtk4
* gtk4-layer-shell
* pybind11
* nlohmann-json

### Install
* Through pip:
```bash
pip install git+https://github.com/xZepyx/helium
```
* AUR:
```bash
yay -S python-helium
```

Also read the [Documentation](./docs).

## Release
Helium is currently in beta. It crashes sometimes.
But most of the parts are stable.

## Configuration
Helium can be configured in python but it also supports C++ for maximum speed. There is no documenation for C++ so prefer source code directly

## Supported Clients
* Hyprland (Only client with WS model support as of now)
* Wlroot-Clients
* Supports all except the ones which don't support layer-shell protocol.

## Examples
* I have wrote some examples here: [./examples](./examples)
* The best source is probably [nucleus-shell-v2](https://github.com/nucleus-hq/nucleus-shell/tree/v2) (actively being worked on)

## Contributing
Feel free to contribute. I will write the guides soon.

## License
© 2026-PRESENT xZepyx — Licensed under the GNU GPL v3

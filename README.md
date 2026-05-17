# Helium 

Helium lets you build desktop shell widgets with Python/C++ and GTK4. Bars, dashboards, notification popups, control panels — anything that sits on your desktop via the layer-shell protocol.

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
```bash
pip install git+https://github.com/xZepyx/helium
```
Also read the [Documentation](./docs).

## Release
Helium is currently unstable/beta. It crashes sometimes.
But most of the parts are stable.

## Supported Clients
* Hyprland (Only client with WS model support as of now)
* Wlroot-Clients
* Supports all except the ones which don't support layer-shell protocol.

## Examples
* I have wrote some examples here: [./examples](./examples)
* Probably the best source will be [nucleus-shell](https://github.com/nucleus-hq/nucleus-shell) once it's rewritten in helium.

## Contributing
Feel free to contribute. I will write the guides soon.

## License
© 2026-PRESENT xZepyx — Licensed under the GNU GPL v3

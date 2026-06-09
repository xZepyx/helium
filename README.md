# Helium

Helium wraps `layer-shika` and gets out of your way. Clean API, nested config
macros, compositor auto-detection, and system services — so you can build the
shell, not the scaffolding.

Helium does **not** replace `layer-shika`. The raw API is always accessible at `helium_wsl::raw`.

## Quick start
Add this to your Cargo.toml:
```toml
[dependencies]
helium-wsl = "0.1.5"
```
or: 
```
cargo add helium-wsl 
```


## Features

- **Anchor API** — tuple-based `.anchor()` with 1–4 `AnchorEdge` values
- **Config macro** — `helium_config!` generates nested structs with serde + defaults
- **Compositors** — unified `Compositor` trait with Hyprland, Niri, Sway backends
- **Services** — audio, time, backlight, power, power profiles, network, bluetooth
- **Adapters** — trait-based system for connecting config to surface properties

## Documentation

[xzepyx.github.io/helium](https://xzepyx.github.io/helium)

## Thank You
* [layer-shika](https://github.com/waydeerwm/layer-shika): This is what helium works on.

## License
MIT

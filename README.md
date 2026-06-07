# Helium

A modern Wayland shell library wrapping `layer-shika`. Helium gives you a
cleaner api, a macro-based config system, compositor auto-detection,
system service wrappers, and an adapter system for wiring config to Slint UI
properties.

Helium does **not** replace `layer-shika`. The raw API is always accessible
at `helium::raw`.

## Quick start

```toml
[dependencies]
helium = "0.1"
```

```rust
use helium::{
    helium_config, AnchorEdge, Helium,
    adapters::{self, ClockAdapter, WorkspacesAdapter},
};

helium_config! {
    HeliumConfig {
        bar: {
            height: u32 = 42,
            modules: {
                clock: {
                    format: String = "%H:%M",
                },
                workspaces: {
                    max: u8 = 9,
                },
            },
        },
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let config = HeliumConfig::load("config.json").unwrap_or_default();
    let helium = Helium::from_config(config)?;

    let _surface = helium
        .surface("heliumbar")
        .size(1920, 42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .exclusive()
        .build()?;

    let _registry = adapters! {
        "clock" => ClockAdapter { format: "%H:%M".into(), ..Default::default() },
        "workspaces" => WorkspacesAdapter { max: 9 },
    };

    Ok(())
}
```

## Features

- **Anchor API** — tuple-based `.anchor()` with 1–4 `AnchorEdge` values
- **Config macro** — `helium_config!` generates nested structs with serde + defaults
- **Compositors** — unified `Compositor` trait with Hyprland, Niri, Sway backends
- **Services** — audio, time, backlight, power, power profiles, network, bluetooth
- **Adapters** — trait-based system for connecting config to surface properties

## Documentation

Full docs are in the [`docs/`](docs/) directory:
[Getting Started](docs/getting-started.md),
[Anchors](docs/anchors.md),
[Config](docs/config.md),
[Services](docs/services.md),
[Compositors](docs/compositors.md),
[Adapters](docs/adapters.md).

## License

MIT

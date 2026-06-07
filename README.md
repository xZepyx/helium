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
    adapters::{ClockAdapter, WorkspacesAdapter},
};

helium_config! {
    Config {
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
    let config = Config::load("config.json")?;

    let mut runtime = Helium::from_file("bar.slint")
        .surface("main")
        .size(1920, 42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .exclusive()
        .build()?;

    runtime.run()?;
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
[API](docs/api.md),
[Config](docs/config.md),
[Services](docs/services.md),
[Compositors](docs/compositors.md),
[Adapters](docs/adapters.md).

## License

MIT

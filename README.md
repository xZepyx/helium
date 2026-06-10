# Helium

Helium wraps `layer-shika` and gets out of your way. Clean API, nested config
macros, compositor auto-detection, and system services — so you can build the
shell, not the scaffolding.

Helium does **not** replace `layer-shika`. The raw API is always accessible at `helium_wsl::raw`.

## Quick start

Add this to your Cargo.toml:

```toml
[dependencies]
helium-wsl = "0.2.0"
```

```rust
use helium_wsl::{AnchorEdge, Helium, Layer};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut shell = Helium::from_file("examples/minimal-bar.slint")
        .surface("main")
        .size(1920, 42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .layer(Layer::Top)
        .exclusive()
        .build()?;

    shell.on_tick(std::time::Duration::from_secs(1), |ctx| {
        ctx.set("main", "label", "hello world");
    })?;

    shell.run()?;
    Ok(())
}
```

## Features

- **Anchor API** — tuple-based `.anchor()` with 1–4 `AnchorEdge` values
- **Config macro** — `helium_config!` generates nested structs with serde + defaults
- **Compositors** — unified `Compositor` trait with Hyprland, Niri, Sway, MangoWM backends
- **Services** — time, backlight (always available); audio, bluetooth, network, power, power profiles (feature-gated, stubbed)
- **Adapters** — `Adapter` trait + built-in `ClockAdapter` and `WorkspacesAdapter` for connecting config to surface properties
- **Macros** — `helium_struct!` for plain structs, `helium_model!` for Slint-compatible models, `adapters!` for adapter registration

## Documentation

- [Getting Started](docs/getting-started.md)
- [Anchors](docs/anchors.md)
- [Config](docs/config.md)
- [Compositors](docs/compositors.md)
- [Services](docs/services.md)
- [Adapters](docs/adapters.md)
- [API Reference](docs/api.md)

## License

MIT

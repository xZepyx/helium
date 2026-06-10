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
or:
```rust
use helium_wsl::{Helium, AnchorEdge, ShellInitializer, SurfaceInitializer, ShellInstance, Layer};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut shell: ShellInstance = Helium::from_file("examples/minimal-bar.slint")
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
- **Compositors** — unified `Compositor` trait with Hyprland, Niri, Sway backends
- **Services** — audio, time, backlight, power, power profiles, network, bluetooth
- **Adapters** — trait-based system for connecting config to surface properties

## Documentation

[xzepyx.github.io/helium](https://xzepyx.github.io/helium)

## Thank You
* [layer-shika](https://github.com/waydeerwm/layer-shika): This is what helium works on.

## License
MIT

# Getting Started

Install Helium and build your first Wayland shell surface.

## Installation

Add Helium to your `Cargo.toml`:

```toml
[dependencies]
helium = "0.1"
```

If you only need the core wrapping (no D-Bus services), disable the default `services` feature:

```toml
[dependencies]
helium = { version = "0.1", default-features = false }
```

## Your first shell

Create a layer-shell surface with a clock and workspace indicators:

```rust
use helium::{
    helium_config,
    AnchorEdge,
    Helium,
    adapters,
    adapters::{AdapterRegistry, ClockAdapter, WorkspacesAdapter},
    config::HeliumConfig,
};

helium_config! {
    HeliumConfig {
        bar: {
            height: u32 = 42,
            modules: {
                clock: {
                    format: String = "%H:%M",
                    interval_ms: u64 = 1000,
                },
                workspaces: {
                    max: u8 = 9,
                },
            },
        },
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Load config from file, or use defaults
    let config = HeliumConfig::load("config.json").unwrap_or_default();

    // Create the shell
    let mut helium = Helium::from_config(config)?;

    // Build a bar surface
    let surface = helium
        .surface("heliumbar")
        .size(1920, 42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .exclusive()
        .build()?;

    // Register adapters
    let mut registry = adapters! {
        "clock" => ClockAdapter { format: "%H:%M".into(), ..Default::default() },
        "workspaces" => WorkspacesAdapter { max: 9 },
    };

    // In a real app, you'd wire adapters to the surface's Slint properties
    // and start the event loop.

    Ok(())
}
```

## Running

Make sure you have a Wayland compositor running. Helium connects to the
Wayland display via `layer-shika` and does not need any special permissions.

## What's next

- [API](api.md) — anchors, shell builder, and raw access
- [Config](config.md) — the `helium_config!` macro and loading
- [Services](services.md) — audio, backlight, network and more
- [Compositors](compositors.md) — IPC with your WM
- [Adapters](adapters.md) — wiring config to surface properties

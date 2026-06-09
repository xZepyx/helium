# Getting Started

Install Helium and build your first Wayland shell surface.

## Installation

Add Helium to your `Cargo.toml`:

```toml
[dependencies]
helium = "0.1"
```

### Minimal install (no D-Bus services)

If you just want the core shell wrapper and don't need audio, network, or
Bluetooth backends, disable the default features:

```toml
[dependencies]
helium = { version = "0.1", default-features = false }
```

This skips the `zbus` dependency tree entirely — faster compile, fewer
things to break at 2 AM.

## Feature flags overview

| Flag | What it enables |
|------|----------------|
| `compositors` (default) | All compositor backends (Hyprland, Niri, Sway, MangoWM) |
| `services` (default) | All D-Bus service modules (audio, bluetooth, network, power, power profiles) |
| `compositor-hyprland` | Hyprland IPC backend |
| `compositor-niri` | Niri IPC backend |
| `compositor-sway` | Sway/i3 IPC backend |
| `compositor-mangowm` | MangoWM (stub) |
| `service-audio` | Audio backend (PipeWire/PulseAudio via D-Bus) |
| `service-bluetooth` | Bluetooth (BlueZ via D-Bus) |
| `service-network` | NetworkManager via D-Bus |
| `service-power` | UPower via D-Bus |
| `service-powerprofiles` | power-profiles-daemon via D-Bus |

Time and backlight are always available — they use `chrono` and sysfs
respectively, no external deps needed.

## Your first shell

Here is a complete example that creates a bar surface with a clock and
workspace indicators. It uses `helium_config!` to declare a typed config,
then builds the shell and attaches adapters.

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

    Ok(())
}
```

## Running

Make sure you have a Wayland compositor running — Helium connects through
`layer-shika` and does not need any special permissions. If you see nothing
happen, check that `$WAYLAND_DISPLAY` is set (it usually is).

## What's next

- [API](api.md) — anchors, shell builder, HeliumRuntime, and raw access
- [Config](config.md) — the `helium_config!` macro with `load`, `save`, `reload`
- [Services](services.md) — audio, backlight, network, power, and more
- [Compositors](compositors.md) — unified IPC with your window manager
- [Adapters](adapters.md) — wiring config to surface properties

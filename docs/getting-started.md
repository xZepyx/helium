# Getting Started

Install Helium and build your first Wayland shell surface.

## Installation

Add Helium to your `Cargo.toml`:

```toml
[dependencies]
helium-wsl = "0.2.0"
```

### Minimal install (no D-Bus services)

If you just want the core shell wrapper and don't need audio, network, or
Bluetooth backends, disable the default features:

```toml
[dependencies]
helium-wsl = { version = "0.2.0", default-features = false }
```

## Feature flags overview

| Flag | What it enables |
|------|----------------|
| `compositors` (default) | All compositor backends (Hyprland, Niri, Sway, MangoWM) |
| `services` (default) | All D-Bus service modules (audio, bluetooth, network, power, power profiles) |
| `compositor-hyprland` | Hyprland IPC backend |
| `compositor-niri` | Niri IPC backend |
| `compositor-sway` | Sway/i3 IPC backend |
| `compositor-mangowm` | MangoWM (stub) |
| `service-audio` | Audio backend (PipeWire/PulseAudio via D-Bus, stubbed) |
| `service-bluetooth` | Bluetooth (BlueZ via D-Bus, stubbed) |
| `service-network` | NetworkManager via D-Bus (stubbed) |
| `service-power` | UPower via D-Bus (stubbed) |
| `service-powerprofiles` | power-profiles-daemon via D-Bus (stubbed) |

Time and backlight are always available — they use `chrono` and sysfs
respectively, no external deps needed.

## Your first shell

Write a `bar.slint` file:

```slint
export component Bar {
    in property <string> label;
    Rectangle {
        background: #141414;
        Text {
            text: label;
            color: #d4d4d4;
            vertical-alignment: center;
            horizontal-alignment: center;
        }
    }
}
```

Then create the surface in Rust:

```rust
use helium_wsl::{AnchorEdge, Helium, Layer};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut shell = Helium::from_file("bar.slint")
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

## Running

Make sure you have a Wayland compositor running — Helium connects through
`layer-shika` and does not need any special permissions. If you see nothing
happen, check that `$WAYLAND_DISPLAY` is set (it usually is).

## What's next

- [Anchors](anchors.md) — tuple-based anchor API
- [Config](config.md) — the `helium_config!` macro with `load`, `save`, `reload`
- [Services](services.md) — audio, backlight, network, power, and more
- [Compositors](compositors.md) — unified IPC with your window manager
- [Adapters](adapters.md) — wiring config to surface properties
- [Shell API](api.md) — every method on `ShellInstance`, `TickContext`, `IpcContext`

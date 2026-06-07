# Helium Examples

Slint UI definitions for Helium-powered Wayland shell surfaces.

## Prerequisites

- Rust with the `helium` crate
- A Wayland compositor (Hyprland, Niri, Sway, etc.)

## Usage

Each `.slint` file defines a reusable component. Load them in your Rust
code via `Helium::from_file`:

```rust
use helium::{AnchorEdge, Helium};

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut runtime = Helium::from_file("examples/minimal-bar.slint")
        .surface("main")
        .size(1920, 42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .exclusive()
        .build()?;

    runtime.run()?;
    Ok(())
}
```

## Files

| File | Description |
|------|-------------|
| `minimal-bar.slint` | Simple centered text bar |
| `clock-bar.slint` | Right-aligned clock display (Space Mono) |
| `workspace-bar.slint` | Left-aligned workspace indicators |
| `full-bar.slint` | Combines workspace + label + clock sections |

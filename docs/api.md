# API

Helium's public API surface covers anchors, the shell builder, and the raw
escape hatch to `layer-shika`.

## Anchors

Helium replaces `layer-shika`'s bitflag-based anchors with a variadic tuple API.

### AnchorEdge

```rust
pub enum AnchorEdge {
    Top,
    Bottom,
    Left,
    Right,
}
```

### Passing anchors to a surface

Use tuples of 1–4 `AnchorEdge` values with the `.anchor()` method:

```rust
use helium_wsl::{AnchorEdge, SurfaceBuilder};

surface.anchor((AnchorEdge::Top,));
surface.anchor((AnchorEdge::Top, AnchorEdge::Left));
surface.anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right));
surface.anchor((AnchorEdge::Top, AnchorEdge::Bottom, AnchorEdge::Left, AnchorEdge::Right));
```

### How it works

`IntoAnchorEdges` is a sealed trait implemented for tuples of 1, 2, 3, and 4
`AnchorEdge` values. Each tuple is converted to a `layer_shika::AnchorEdges`
under the hood. The sealed pattern keeps the conversion unambiguous.

## Shell builder

Use `Helium::from_file(path)` to start building a shell:

```rust
use helium_wsl::{Helium, AnchorEdge, MonitorPolicy};

Helium::from_file("ui/bar.slint")
    .surface("Main")
        .height(42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .exclusive()
        .monitors(MonitorPolicy::Primary)
    .build()?
    .run()?;
```

## Raw access

The full `layer-shika` API is available through `helium_wsl::raw`:

```rust
use helium_wsl::raw::AnchorEdges;

// Direct access when Helium's wrapper isn't enough
let edges = AnchorEdges::top_bar();
```

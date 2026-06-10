# Anchors

Helium replaces `layer-shika`'s bitflag-based anchors with a variadic tuple
API. Fewer ways to accidentally `OR` the wrong bits together.

## AnchorEdge

```rust
pub enum AnchorEdge {
    Top,
    Bottom,
    Left,
    Right,
}
```

## Passing anchors to a surface

Use tuples of 1–4 `AnchorEdge` values with the `.anchor()` method on
[`SurfaceInitializer`]:

```rust
use helium_wsl::{AnchorEdge, Helium};

Helium::from_file("bar.slint")
    .surface("main")
    .anchor((AnchorEdge::Top,))
    .surface("bar")
    .anchor((AnchorEdge::Top, AnchorEdge::Left))
    .surface("bar")
    .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
    .surface("bar")
    .anchor((AnchorEdge::Top, AnchorEdge::Bottom, AnchorEdge::Left, AnchorEdge::Right));
```

## How it works

`IntoAnchorEdges` is a sealed trait implemented for tuples of 1 through 4
`AnchorEdge` values. Under the hood, each tuple is converted to a
`layer_shika::AnchorEdges`. The sealed pattern means you cannot accidentally
pass a random 4-tuple and have it do something unexpected.

## Re-exports

Both `AnchorEdge` and `IntoAnchorEdges` are re-exported at the crate root
and in the prelude:

```rust
use helium_wsl::prelude::*;
// AnchorEdge and IntoAnchorEdges are in scope
```

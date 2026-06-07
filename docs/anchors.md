# Anchors

Helium replaces `layer-shika`'s bitflag-based anchors with a variadic tuple API.

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

Use tuples of 1–4 `AnchorEdge` values with the `.anchor()` method:

```rust
use helium::{AnchorEdge, SurfaceBuilder};

// Anchor to the top edge only
surface.anchor((AnchorEdge::Top,));

// Anchor to top and left
surface.anchor((AnchorEdge::Top, AnchorEdge::Left));

// Anchor to top, left, and right (a typical bar)
surface.anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right));

// Anchor to all four edges (fills the screen)
surface.anchor((AnchorEdge::Top, AnchorEdge::Bottom, AnchorEdge::Left, AnchorEdge::Right));
```

## How it works

`IntoAnchorEdges` is a sealed trait implemented for tuples of 1, 2, 3, and 4
`AnchorEdge` values. Each tuple is converted to a `layer_shika::AnchorEdges`
bitflag under the hood.

The sealed pattern means you cannot implement `IntoAnchorEdges` for your own
types outside Helium — this keeps the conversion unambiguous.

## Why not bitflags directly?

Bitflags are explicit but verbose. Passing a tuple reads more naturally in a
fluent builder chain and makes it harder to accidentally pass an empty set of
flags.

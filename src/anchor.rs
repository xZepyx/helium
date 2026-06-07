use layer_shika::AnchorEdges;

/// One edge of the screen.
pub enum AnchorEdge {
    Top,
    Bottom,
    Left,
    Right,
}

/// Conversion into [`AnchorEdges`].
///
/// Implemented for tuples of 1 to 4 [`AnchorEdge`] values so you can write
/// `.anchor((AnchorEdge::Top, AnchorEdge::Left))`.
pub trait IntoAnchorEdges: sealed::Sealed {
    fn into_anchor_edges(self) -> AnchorEdges;
}

mod sealed {
    pub trait Sealed {}
}

fn to_edges(edges: &[AnchorEdge]) -> AnchorEdges {
    let mut e = AnchorEdges::empty();
    for edge in edges {
        e = match edge {
            AnchorEdge::Top => e.with_top(),
            AnchorEdge::Bottom => e.with_bottom(),
            AnchorEdge::Left => e.with_left(),
            AnchorEdge::Right => e.with_right(),
        };
    }
    e
}

impl sealed::Sealed for (AnchorEdge,) {}
impl IntoAnchorEdges for (AnchorEdge,) {
    fn into_anchor_edges(self) -> AnchorEdges {
        to_edges(&[self.0])
    }
}

impl sealed::Sealed for (AnchorEdge, AnchorEdge) {}
impl IntoAnchorEdges for (AnchorEdge, AnchorEdge) {
    fn into_anchor_edges(self) -> AnchorEdges {
        to_edges(&[self.0, self.1])
    }
}

impl sealed::Sealed for (AnchorEdge, AnchorEdge, AnchorEdge) {}
impl IntoAnchorEdges for (AnchorEdge, AnchorEdge, AnchorEdge) {
    fn into_anchor_edges(self) -> AnchorEdges {
        to_edges(&[self.0, self.1, self.2])
    }
}

impl sealed::Sealed for (AnchorEdge, AnchorEdge, AnchorEdge, AnchorEdge) {}
impl IntoAnchorEdges for (AnchorEdge, AnchorEdge, AnchorEdge, AnchorEdge) {
    fn into_anchor_edges(self) -> AnchorEdges {
        to_edges(&[self.0, self.1, self.2, self.3])
    }
}

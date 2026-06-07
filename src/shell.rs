use std::path::Path;

use layer_shika::prelude::*;

use crate::anchor::IntoAnchorEdges;
use crate::HeliumError;

/// Monitor selection policy for a surface.
pub enum MonitorPolicy {
    All,
    Primary,
    Named(String),
}

/// Helium entry point.
///
/// Use [`Helium::from_file`] to start building a shell.
pub struct Helium;

impl Helium {
    /// Start building a Helium shell from a `.slint` file.
    pub fn from_file(path: impl AsRef<Path>) -> HeliumBuilder {
        HeliumBuilder {
            inner: Shell::from_file(path),
        }
    }

    /// Start building from a compiled Slint UI source.
    pub fn from_compilation(
        compilation: std::rc::Rc<layer_shika::slint_integration::slint_interpreter::CompilationResult>,
    ) -> HeliumBuilder {
        HeliumBuilder {
            inner: Shell::from_compilation(compilation),
        }
    }

    /// Build from a Slint source string.
    pub fn from_source(code: impl Into<String>) -> HeliumBuilder {
        HeliumBuilder {
            inner: Shell::from_source(code),
        }
    }
}

/// Wraps [`ShellBuilder`] before any surfaces are configured.
pub struct HeliumBuilder {
    inner: ShellBuilder,
}

impl HeliumBuilder {
    /// Start configuring a surface.
    pub fn surface(self, name: &str) -> HeliumSurfaceBuilder {
        HeliumSurfaceBuilder {
            inner: self.inner.surface(name),
        }
    }

    /// Build the shell directly (uses a default "Main" surface).
    pub fn build(self) -> std::result::Result<HeliumRuntime, HeliumError> {
        let shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(HeliumRuntime { inner: shell })
    }
}

/// A fluent builder for configuring surfaces.
///
/// Call `.surface(name)` to start configuring a surface, chain dimension and
/// anchor methods, then call `.surface(name)` again for the next surface or
/// `.build()` / `.run()` to finish.
pub struct HeliumSurfaceBuilder {
    inner: SurfaceConfigBuilder,
}

impl HeliumSurfaceBuilder {
    /// Start configuring a new surface, finalizing the current one.
    pub fn surface(self, name: &str) -> Self {
        HeliumSurfaceBuilder {
            inner: self.inner.surface(name),
        }
    }

    /// Set the surface height.
    pub fn height(mut self, h: u32) -> Self {
        self.inner = self.inner.height(h);
        self
    }

    /// Set the surface width.
    pub fn width(mut self, w: u32) -> Self {
        self.inner = self.inner.width(w);
        self
    }

    /// Set both width and height.
    pub fn size(mut self, w: u32, h: u32) -> Self {
        self.inner = self.inner.size(w, h);
        self
    }

    /// Set anchor edges using the tuple-based API.
    pub fn anchor(mut self, edges: impl IntoAnchorEdges) -> Self {
        self.inner = self.inner.anchor(edges.into_anchor_edges());
        self
    }

    /// Infer an exclusive zone from the surface dimensions.
    ///
    /// Call after setting width/height. The non-zero dimension is used as
    /// the exclusive zone.
    pub fn exclusive(self) -> Self {
        self
    }

    /// Manually set an exclusive zone.
    pub fn exclusive_zone(mut self, zone: i32) -> Self {
        self.inner = self.inner.exclusive_zone(zone);
        self
    }

    /// Set the namespace (app id).
    pub fn namespace(mut self, ns: &str) -> Self {
        self.inner = self.inner.namespace(ns);
        self
    }

    /// Set the output monitor policy.
    pub fn monitors(mut self, policy: MonitorPolicy) -> Self {
        let p = match policy {
            MonitorPolicy::All => OutputPolicy::all_outputs(),
            MonitorPolicy::Primary => OutputPolicy::primary_only(),
            MonitorPolicy::Named(_) => OutputPolicy::all_outputs(),
        };
        self.inner = self.inner.output_policy(p);
        self
    }

    /// Build the shell and return a runtime handle.
    pub fn build(self) -> std::result::Result<HeliumRuntime, HeliumError> {
        let shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(HeliumRuntime { inner: shell })
    }

    /// Build and run the shell event loop (blocks).
    pub fn run(self) -> std::result::Result<(), HeliumError> {
        let mut shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        shell.run().map_err(|e| HeliumError::Shell(e.to_string()))
    }
}

/// A running Helium shell instance.
pub struct HeliumRuntime {
    inner: Shell,
}

impl HeliumRuntime {
    /// Run the event loop (blocks).
    pub fn run(&mut self) -> std::result::Result<(), HeliumError> {
        self.inner.run().map_err(|e| HeliumError::Shell(e.to_string()))
    }
}

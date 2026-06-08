pub mod adapters;
pub mod anchor;
pub mod config;
pub mod compositors;
pub mod services;
pub mod shell;

/// Re-export of the underlying `layer-shika` crate.
///
/// Use this escape hatch when you need access to raw Wayland types that
/// Helium doesn't wrap yet.
pub mod raw {
    pub use layer_shika::*;
}

/// Re-export of the `chrono` crate for convenience.
///
/// Use this instead of adding `chrono` to your own `Cargo.toml`.
pub use chrono;

/// Re-export of the `slint-interpreter` crate for convenience.
///
/// Use this instead of adding `slint-interpreter` to your own `Cargo.toml`.
pub use layer_shika::slint_integration::slint_interpreter;

pub use anchor::{AnchorEdge, IntoAnchorEdges};
pub use shell::{
    Helium, HeliumBuilder, HeliumRuntime, HeliumSurfaceBuilder, IntoSlintValue, IpcContext,
    MonitorPolicy, TickContext,
};

/// Convenience prelude that re-exports the most common types.
///
/// ```
/// use helium_wsl::prelude::*;
/// ```
pub mod prelude {
    pub use crate::{
        AnchorEdge, Helium, HeliumRuntime, IntoSlintValue, IpcContext, MonitorPolicy, TickContext,
    };
}

// Re-export the proc macro so users can `use helium::helium_config`.
pub use helium_wsl_macros::helium_config;

/// Unified error type for Helium.
#[derive(Debug, thiserror::Error)]
pub enum HeliumError {
    /// The compositor could not be detected or connected to.
    #[error("compositor error: {0}")]
    Compositor(String),
    /// An error from the underlying shell library.
    #[error("shell error: {0}")]
    Shell(String),
    /// A service backend was unavailable or returned an error.
    #[error("service error: {0}")]
    Service(String),
    /// An I/O error.
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),
    /// A config error.
    #[error("config error: {0}")]
    Config(#[from] config::ConfigError),
}

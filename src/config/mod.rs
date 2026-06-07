/// Errors that can occur when loading or parsing configuration.
#[derive(Debug, thiserror::Error)]
pub enum ConfigError {
    /// The config file could not be read.
    #[error("failed to read config: {0}")]
    Io(#[from] std::io::Error),
    /// The config file could not be parsed as JSON.
    #[error("failed to parse config: {0}")]
    Parse(#[from] serde_json::Error),
}

impl ConfigError {
    /// Wrap an io::Error as a reading error.
    pub fn reading(e: std::io::Error) -> Self {
        ConfigError::Io(e)
    }

    /// Wrap a serde_json::Error as a parsing error.
    pub fn parsing(e: serde_json::Error) -> Self {
        ConfigError::Parse(e)
    }
}

/// Re-export of the [`helium_config!`](crate::helium_config) proc macro.
///
/// The macro generates a config struct with whatever name you give it
/// (typically `Config`). All generated types are re-exported here.
#[doc(inline)]
pub use helium_macros::*;

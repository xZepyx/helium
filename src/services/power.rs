use std::time::Duration;

/// Battery state.
#[derive(Debug, Clone)]
pub struct Battery {
    pub percentage: f32,
    pub charging: bool,
    pub time_to_empty: Option<Duration>,
    pub time_to_full: Option<Duration>,
}

/// Get the current battery states from UPower over D-Bus.
pub fn batteries() -> Result<Vec<Battery>, String> {
    Err("UPower not available — enable the \"services\" feature".into())
}

/// Register a callback for battery state changes.
///
/// UPower/D-Bus monitoring is not yet implemented. Returns an error until
/// a real backend is wired.
pub fn on_change(_cb: impl Fn(Vec<Battery>) + Send + 'static) -> Result<(), String> {
    Err("Power on_change: UPower/D-Bus monitoring not yet implemented".into())
}

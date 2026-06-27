/// A power profile.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Profile {
    Performance,
    Balanced,
    PowerSaver,
}

/// Get the active power profile from power-profiles-daemon.
pub fn active() -> Result<Profile, String> {
    Err("power-profiles-daemon not available — enable the \"services\" feature".into())
}

/// Set the active power profile.
pub fn set(_profile: Profile) -> Result<(), String> {
    Err("power-profiles-daemon not available — enable the \"services\" feature".into())
}

/// Register a callback for profile changes.
///
/// power-profiles-daemon/D-Bus monitoring is not yet implemented. Returns
/// an error until a real backend is wired.
pub fn on_change(_cb: impl Fn(Profile) + Send + 'static) -> Result<(), String> {
    Err("PowerProfiles on_change: D-Bus monitoring not yet implemented".into())
}

mod compositor;

#[cfg(feature = "compositor-hyprland")]
mod hyprland;
#[cfg(feature = "compositor-niri")]
mod niri;
#[cfg(feature = "compositor-sway")]
mod sway;
#[cfg(feature = "compositor-mangowm")]
mod mangowm;

pub use compositor::{Compositor, CompositorEvent, Monitor, Window, Workspace};

use crate::HeliumError;

/// Auto-detect the running compositor by checking environment variables.
///
/// Checks in order:
/// 1. `HYPRLAND_INSTANCE_SIGNATURE` → Hyprland
/// 2. `NIRI_SOCKET` → Niri
/// 3. `SWAYSOCK` → Sway
///
/// Returns an error if none of the compositors are detected.
pub fn detect() -> Result<Box<dyn Compositor>, HeliumError> {
    #[cfg(feature = "compositor-hyprland")]
    if hyprland::Hyprland::is_running() {
        return hyprland::Hyprland::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    #[cfg(feature = "compositor-niri")]
    if niri::Niri::is_running() {
        return niri::Niri::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    #[cfg(feature = "compositor-sway")]
    if sway::Sway::is_running() {
        return sway::Sway::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    #[cfg(feature = "compositor-mangowm")]
    if mangowm::MangoWM::is_running() {
        return mangowm::MangoWM::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    Err(HeliumError::Compositor(
        "no supported compositor detected — enable a compositor-* feature".into(),
    ))
}

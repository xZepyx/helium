mod compositor;
mod hyprland;
mod mangowm;
mod niri;
mod sway;

pub use compositor::{Compositor, Monitor, Window, Workspace};

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
    if hyprland::Hyprland::is_running() {
        return hyprland::Hyprland::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    if niri::Niri::is_running() {
        return niri::Niri::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    if sway::Sway::is_running() {
        return sway::Sway::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    if mangowm::MangoWM::is_running() {
        return mangowm::MangoWM::connect()
            .map(|c| Box::new(c) as Box<dyn Compositor>);
    }
    Err(HeliumError::Compositor(
        "no supported compositor detected — set HYPRLAND_INSTANCE_SIGNATURE, NIRI_SOCKET, or SWAYSOCK".into(),
    ))
}

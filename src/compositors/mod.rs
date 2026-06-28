mod compositor;

#[cfg(feature = "compositor-hyprland")]
pub mod hyprland;
#[cfg(feature = "compositor-niri")]
pub mod niri;

pub use compositor::{Compositor, CompositorEvent, Monitor, Window, WindowDiffusion, Workspace};

pub fn detect() -> Result<Box<dyn Compositor>, crate::HeliumError> {
    #[cfg(feature = "compositor-hyprland")]
    if hyprland::Hyprland::is_running() {
        return Ok(Box::new(hyprland::Hyprland::connect()?));
    }
    #[cfg(feature = "compositor-niri")]
    if niri::Niri::is_running() {
        return Ok(Box::new(niri::Niri::connect()?));
    }
    Err(crate::HeliumError::Compositor(
        "no supported compositor detected (set HYPRLAND_INSTANCE_SIGNATURE or NIRI_SOCKET)".into()
    ))
}

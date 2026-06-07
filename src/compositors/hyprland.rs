use std::os::unix::net::UnixStream;
use std::path::PathBuf;

use super::compositor::{Compositor, Monitor, Window, Workspace};

pub struct Hyprland {
    _socket: PathBuf,
}

impl Hyprland {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        let sig = std::env::var("HYPRLAND_INSTANCE_SIGNATURE")
            .map_err(|_| crate::HeliumError::Compositor("HYPRLAND_INSTANCE_SIGNATURE not set".into()))?;
        let runtime = std::env::var("XDG_RUNTIME_DIR")
            .unwrap_or_else(|_| "/run/user/1000".into());
        let socket = PathBuf::from(runtime)
            .join("hypr")
            .join(&sig)
            .join(".socket.sock");
        UnixStream::connect(&socket)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Hyprland: {e}")))?;
        Ok(Hyprland { _socket: socket })
    }

    pub fn is_running() -> bool {
        std::env::var("HYPRLAND_INSTANCE_SIGNATURE").is_ok()
    }
}

impl Compositor for Hyprland {
    fn workspaces(&self) -> Vec<Workspace> {
        vec![]
    }

    fn active_workspace(&self) -> Option<Workspace> {
        None
    }

    fn monitors(&self) -> Vec<Monitor> {
        vec![]
    }

    fn on_workspace_change(&mut self, _cb: Box<dyn Fn(Workspace) + Send>) {}

    fn on_window_focus(&mut self, _cb: Box<dyn Fn(Window) + Send>) {}
}

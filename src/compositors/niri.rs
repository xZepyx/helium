use std::os::unix::net::UnixStream;

use super::compositor::{Compositor, Monitor, Window, Workspace};

pub struct Niri {
    _socket_path: String,
}

impl Niri {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        let path = std::env::var("NIRI_SOCKET")
            .map_err(|_| crate::HeliumError::Compositor("NIRI_SOCKET not set".into()))?;
        UnixStream::connect(&path)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Niri: {e}")))?;
        Ok(Niri { _socket_path: path })
    }

    pub fn is_running() -> bool {
        std::env::var("NIRI_SOCKET").is_ok()
    }
}

impl Compositor for Niri {
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

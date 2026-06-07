use super::compositor::{Compositor, Monitor, Window, Workspace};

pub struct MangoWM;

impl MangoWM {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        if !Self::is_running() {
            return Err(crate::HeliumError::Compositor("MangoWM not running".into()));
        }
        Ok(MangoWM)
    }

    /// Check if MangoWM is running by looking for its env var.
    ///
    /// MangoWM doesn't have documented IPC yet, but it might in the future.
    pub fn is_running() -> bool {
        // MangoWM doesn't set a standard env var yet. Feel free to PR one.
        false
    }
}

impl Compositor for MangoWM {
    fn workspaces(&self) -> Vec<Workspace> {
        // No IPC docs yet. This is doing its best.
        todo!("MangoWM IPC not documented yet")
    }

    fn active_workspace(&self) -> Option<Workspace> {
        todo!("MangoWM IPC not documented yet")
    }

    fn monitors(&self) -> Vec<Monitor> {
        todo!("MangoWM IPC not documented yet")
    }

    fn on_workspace_change(&mut self, _cb: Box<dyn Fn(Workspace) + Send>) {
        todo!("MangoWM IPC not documented yet")
    }

    fn on_window_focus(&mut self, _cb: Box<dyn Fn(Window) + Send>) {
        todo!("MangoWM IPC not documented yet")
    }
}

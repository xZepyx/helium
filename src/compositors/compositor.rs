/// A workspace on a compositor.
#[derive(Debug, Clone)]
pub struct Workspace {
    pub id: u32,
    pub name: String,
    pub active: bool,
    pub monitor: String,
}

/// A monitor/output connected to the compositor.
#[derive(Debug, Clone)]
pub struct Monitor {
    pub name: String,
    pub width: u32,
    pub height: u32,
    pub scale: f64,
    pub primary: bool,
}

/// A window managed by the compositor.
#[derive(Debug, Clone)]
pub struct Window {
    pub title: String,
    pub class: String,
    pub workspace_id: u32,
}

/// Unified interface for compositor IPC.
///
/// Each supported compositor implements this trait by connecting to its
/// Unix socket and parsing its protocol.
pub trait Compositor: Send {
    /// List all workspaces.
    fn workspaces(&self) -> Vec<Workspace>;
    /// Get the currently active workspace, if any.
    fn active_workspace(&self) -> Option<Workspace>;
    /// List all monitors.
    fn monitors(&self) -> Vec<Monitor>;
    /// Register a callback for workspace changes.
    fn on_workspace_change(&mut self, cb: Box<dyn Fn(Workspace) + Send>);
    /// Register a callback for window focus changes.
    fn on_window_focus(&mut self, cb: Box<dyn Fn(Window) + Send>);
}

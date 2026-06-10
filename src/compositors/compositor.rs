/// A workspace on a compositor.
#[derive(Debug, Clone)]
pub struct Workspace {
    pub id: u32,
    pub name: String,
    pub active: bool,
    pub occupied: bool,
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

/// Events emitted by the compositor event socket.
#[derive(Debug, Clone)]
pub enum CompositorEvent {
    WorkspaceChanged(Workspace),
    WorkspacesUpdated(Vec<Workspace>),
    WindowFocused(Window),
    WindowClosed(Window),
    MonitorAdded(Monitor),
    MonitorRemoved(String),
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
    /// Get the currently focused window, if any.
    fn active_window(&self) -> Option<Window> {
        None
    }
    /// Return the event socket file descriptor for push-based events.
    fn event_fd(&self) -> Option<std::os::unix::io::RawFd> {
        None
    }
    /// Poll one event from the event socket, if available.
    fn poll_event(&mut self) -> Option<CompositorEvent> {
        None
    }
}

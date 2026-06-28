/// A workspace on a compositor.
#[derive(Debug, Clone)]
pub struct Workspace {
    pub id: u32,
    pub name: String,
    pub active: bool,
    pub occupied: bool,       // true if any window exists on this workspace
    pub window_count: u32,    // exact number of windows
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

/// Represents a window focus diffusion event — a window losing focus.
///
/// Tracks the previously focused window and the newly focused window.
/// Either or both may be `None` (e.g. no window focused before/after).
#[derive(Debug, Clone)]
pub struct WindowDiffusion {
    /// The window that was focused before the change, if any.
    pub unfocused: Option<Window>,
    /// The window that is focused now, if any.
    pub focused: Option<Window>,
}

/// Events emitted by the compositor event socket.
///
/// NOTE: In 0.2.3 `WorkspaceChanged` changed from a tuple variant
/// `WorkspaceChanged(Workspace)` to a struct variant
/// `WorkspaceChanged { workspace, focused_window }`.
/// If nucleus-shell or other consumers match on the old shape they
/// must be updated.
#[derive(Debug, Clone)]
pub enum CompositorEvent {
    WorkspaceChanged {
        workspace: Workspace,
        focused_window: Option<Window>,  // None if workspace is empty/no focus
    },
    WorkspacesUpdated(Vec<Workspace>),
    WindowFocused(Window),
    WindowDiffusion(WindowDiffusion),
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

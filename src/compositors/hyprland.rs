use std::io::{BufRead, BufReader, Read, Write};
use std::os::unix::io::{AsRawFd, RawFd};
use std::os::unix::net::UnixStream;
use std::path::PathBuf;

use serde::Deserialize;

use super::compositor::{Compositor, CompositorEvent, Monitor, Window, Workspace};

#[derive(Deserialize)]
struct HyprlandWorkspace {
    id: i64,
    name: String,
    monitor: String,
    windows: i64,
}

#[derive(Deserialize)]
struct HyprlandClient {
    class: Option<String>,
    title: Option<String>,
    workspace: Option<HyprlandWorkspaceId>,
}

#[derive(Deserialize)]
struct HyprlandWorkspaceId {
    id: i64,
}

#[derive(Deserialize)]
struct HyprlandMonitor {
    name: String,
    width: Option<u32>,
    height: Option<u32>,
    scale: Option<f64>,
    #[serde(default)]
    focused: bool,
}

pub struct Hyprland {
    socket: PathBuf,
    #[allow(dead_code)]
    event_socket: PathBuf,
    event_reader: Option<BufReader<UnixStream>>,
}

impl Hyprland {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        let sig = std::env::var("HYPRLAND_INSTANCE_SIGNATURE")
            .map_err(|_| crate::HeliumError::Compositor("HYPRLAND_INSTANCE_SIGNATURE not set".into()))?;
        let runtime = std::env::var("XDG_RUNTIME_DIR")
            .unwrap_or_else(|_| "/run/user/1000".into());
        let base = PathBuf::from(runtime).join("hypr").join(&sig);
        let socket = base.join(".socket.sock");
        let event_socket = base.join(".socket2.sock");
        UnixStream::connect(&socket)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Hyprland: {e}")))?;
        let event_stream = UnixStream::connect(&event_socket)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Hyprland event socket: {e}")))?;
        let event_reader = Some(BufReader::new(event_stream));
        Ok(Hyprland { socket, event_socket, event_reader })
    }

    pub fn is_running() -> bool {
        std::env::var("HYPRLAND_INSTANCE_SIGNATURE").is_ok()
    }

    fn send_command(&self, cmd: &str) -> Option<String> {
        let mut stream = UnixStream::connect(&self.socket).ok()?;
        stream.write_all(cmd.as_bytes()).ok()?;
        stream.shutdown(std::net::Shutdown::Write).ok()?;
        let mut buf = Vec::new();
        let mut chunk = [0u8; 4096];
        loop {
            let n = stream.read(&mut chunk).ok()?;
            if n == 0 {
                break;
            }
            buf.extend_from_slice(&chunk[..n]);
        }
        String::from_utf8(buf).ok()
    }

    fn query_workspaces(&self) -> Option<Vec<HyprlandWorkspace>> {
        let resp = self.send_command("j/workspaces")?;
        serde_json::from_str(&resp).ok()
    }

    fn query_active_workspace(&self) -> Option<HyprlandWorkspace> {
        let resp = self.send_command("j/activeworkspace")?;
        serde_json::from_str(&resp).ok()
    }

    fn query_active_window(&self) -> Option<HyprlandClient> {
        let resp = self.send_command("j/activewindow")?;
        serde_json::from_str(&resp).ok()
    }

    fn query_monitors(&self) -> Option<Vec<HyprlandMonitor>> {
        let resp = self.send_command("j/monitors")?;
        serde_json::from_str(&resp).ok()
    }
}

impl Compositor for Hyprland {
    fn workspaces(&self) -> Vec<Workspace> {
        self.query_workspaces()
            .map(|list| {
                let active = self.query_active_workspace();
                list.into_iter()
                    .map(|w| Workspace {
                        id: w.id as u32,
                        name: w.name,
                        active: active.as_ref().is_some_and(|a| a.id == w.id),
                        occupied: w.windows > 0,
                        window_count: w.windows as u32,
                        monitor: w.monitor,
                    })
                    .collect()
            })
            .unwrap_or_default()
    }

    fn active_workspace(&self) -> Option<Workspace> {
        self.query_active_workspace().map(|w| Workspace {
            id: w.id as u32,
            name: w.name,
            active: true,
            occupied: w.windows > 0,
            window_count: w.windows as u32,
            monitor: w.monitor,
        })
    }

    fn monitors(&self) -> Vec<Monitor> {
        self.query_monitors()
            .map(|list| {
                list.into_iter()
                    .enumerate()
                    .map(|(i, m)| Monitor {
                        name: m.name,
                        width: m.width.unwrap_or(0),
                        height: m.height.unwrap_or(0),
                        scale: m.scale.unwrap_or(1.0).max(1.0),
                        primary: i == 0 || m.focused,
                    })
                    .collect()
            })
            .unwrap_or_default()
    }

    fn active_window(&self) -> Option<Window> {
        self.query_active_window().map(|c| {
            let ws_id = c.workspace.map(|w| w.id as u32).unwrap_or(0);
            Window {
                title: c.title.unwrap_or_default(),
                class: c.class.unwrap_or_default(),
                workspace_id: ws_id,
            }
        })
    }

    fn event_fd(&self) -> Option<RawFd> {
        self.event_reader.as_ref().map(|r| r.get_ref().as_raw_fd())
    }

    fn poll_event(&mut self) -> Option<CompositorEvent> {
        let reader = self.event_reader.as_mut()?;
        let mut line = String::new();
        reader.read_line(&mut line).ok()?;
        if line.is_empty() {
            return None;
        }
        let line = line.trim_end();
        let (event_type, data) = line.split_once(">>")?;
        match event_type {
            "workspace" | "workspacev2" => {
                let workspaces = self.workspaces();
                let active = workspaces.iter().find(|w| w.active).cloned();
                active.map(|ws| CompositorEvent::WorkspaceChanged {
                    focused_window: self.active_window(),
                    workspace: ws,
                })
            }
            "activewindow" => {
                let parts: Vec<&str> = data.splitn(2, ',').collect();
                let class = parts.first().unwrap_or(&"").to_string();
                let title = parts.get(1).unwrap_or(&"").to_string();
                let ws = self.active_workspace();
                let workspace_id = ws.as_ref().map(|w| w.id).unwrap_or(0);
                Some(CompositorEvent::WindowFocused(Window {
                    title,
                    class,
                    workspace_id,
                }))
            }
            "closewindow" => {
                Some(CompositorEvent::WindowClosed(Window {
                    title: String::new(),
                    class: data.to_string(),
                    workspace_id: 0,
                }))
            }
            "monitoradded" => {
                let name = data.to_string();
                let monitors = self.monitors();
                let mon = monitors.into_iter().find(|m| m.name == name);
                Some(CompositorEvent::MonitorAdded(
                    mon.unwrap_or(Monitor {
                        name,
                        width: 0,
                        height: 0,
                        scale: 1.0,
                        primary: false,
                    }),
                ))
            }
            "monitorremoved" => {
                Some(CompositorEvent::MonitorRemoved(data.to_string()))
            }
            _ => None,
        }
    }

    fn on_workspace_change(&mut self, _cb: Box<dyn Fn(Workspace) + Send>) {}

    fn on_window_focus(&mut self, _cb: Box<dyn Fn(Window) + Send>) {}
}

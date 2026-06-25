use std::io::{BufRead, BufReader, Write};
use std::os::unix::io::{AsRawFd, RawFd};
use std::os::unix::net::UnixStream;
use serde::Deserialize;
use super::compositor::{Compositor, CompositorEvent, Monitor, Window, Workspace};

pub struct Niri {
    socket_path: String,
    event_reader: Option<BufReader<UnixStream>>,
}

impl Niri {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        let path = std::env::var("NIRI_SOCKET")
            .map_err(|_| crate::HeliumError::Compositor("NIRI_SOCKET not set".into()))?;
        UnixStream::connect(&path)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Niri: {e}")))?;
        let event_stream = UnixStream::connect(&path)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot open Niri event stream: {e}")))?;
        let mut event_reader = BufReader::new(event_stream);
        event_reader.get_mut().write_all(b"{\"EventStream\":null}\n").ok();
        let mut ack = String::new();
        event_reader.read_line(&mut ack).ok();
        Ok(Niri { socket_path: path, event_reader: Some(event_reader) })
    }

    pub fn is_running() -> bool {
        std::env::var("NIRI_SOCKET").is_ok()
    }

    fn request(&self, req: &str) -> Option<serde_json::Value> {
        let mut stream = UnixStream::connect(&self.socket_path).ok()?;
        stream.write_all(req.as_bytes()).ok()?;
        stream.write_all(b"\n").ok()?;
        let mut reader = BufReader::new(stream);
        let mut line = String::new();
        reader.read_line(&mut line).ok()?;
        let val: serde_json::Value = serde_json::from_str(line.trim()).ok()?;
        val.get("Ok").cloned()
    }
}

impl Compositor for Niri {
    fn workspaces(&self) -> Vec<Workspace> {
        #[derive(Deserialize)]
        #[allow(dead_code)]
        struct NiriWorkspace {
            id: u64,
            name: Option<String>,
            output: Option<String>,
            is_focused: bool,
            active_window_id: Option<u64>,
        }
        #[derive(Deserialize)]
        struct NiriWindow {
            workspace_id: Option<u64>,
        }

        let ws_data = match self.request("{\"Workspaces\":null}") {
            Some(v) => v,
            None => return vec![],
        };
        let raw: Vec<NiriWorkspace> = serde_json::from_value(ws_data).unwrap_or_default();

        // get all windows to count per workspace
        let windows: Vec<NiriWindow> = self
            .request("{\"Windows\":null}")
            .and_then(|v| serde_json::from_value(v).ok())
            .unwrap_or_default();

        raw.into_iter()
            .map(|w| {
                let window_count = windows
                    .iter()
                    .filter(|win| win.workspace_id == Some(w.id))
                    .count() as u32;
                Workspace {
                    id: w.id as u32,
                    name: w.name.unwrap_or_else(|| w.id.to_string()),
                    active: w.is_focused,
                    occupied: window_count > 0,
                    window_count,
                    monitor: w.output.unwrap_or_default(),
                }
            })
            .collect()
    }

    fn active_workspace(&self) -> Option<Workspace> {
        self.workspaces().into_iter().find(|w| w.active)
    }

    fn monitors(&self) -> Vec<Monitor> {
        #[derive(Deserialize)]
        struct NiriMode { width: u32, height: u32 }
        #[derive(Deserialize)]
        struct NiriOutput {
            name: String,
            current_mode: Option<NiriMode>,
            #[serde(default = "default_scale")]
            scale: f64,
        }
        fn default_scale() -> f64 { 1.0 }

        let data = match self.request("{\"Outputs\":null}") {
            Some(v) => v,
            None => return vec![],
        };
        let map = match data.as_object() {
            Some(m) => m,
            None => return vec![],
        };
        map.values()
            .enumerate()
            .filter_map(|(i, v)| {
                let o: NiriOutput = serde_json::from_value(v.clone()).ok()?;
                let (w, h) = o.current_mode.map(|m| (m.width, m.height)).unwrap_or((0, 0));
                Some(Monitor {
                    name: o.name,
                    width: w,
                    height: h,
                    scale: if o.scale == 0.0 { 1.0 } else { o.scale },
                    primary: i == 0,
                })
            })
            .collect()
    }

    fn active_window(&self) -> Option<Window> {
        #[derive(Deserialize)]
        struct NiriWindow {
            title: Option<String>,
            app_id: Option<String>,
            workspace_id: Option<u64>,
        }
        let data = self.request("{\"FocusedWindow\":null}")?;
        let w: NiriWindow = serde_json::from_value(data).ok()?;
        Some(Window {
            title: w.title.unwrap_or_default(),
            class: w.app_id.unwrap_or_default(),
            workspace_id: w.workspace_id.unwrap_or(0) as u32,
        })
    }

    fn event_fd(&self) -> Option<RawFd> {
        self.event_reader.as_ref().map(|r| r.get_ref().as_raw_fd())
    }

    fn poll_event(&mut self) -> Option<CompositorEvent> {
        let reader = self.event_reader.as_mut()?;
        let mut line = String::new();
        reader.read_line(&mut line).ok()?;
        if line.trim().is_empty() {
            return None;
        }
        let val: serde_json::Value = serde_json::from_str(line.trim()).ok()?;
        let obj = val.as_object()?;
        let event_type = obj.keys().next()?.as_str();

        match event_type {
            "WorkspacesChanged" | "WorkspaceActivated" => {
                let workspaces = self.workspaces();
                let active = workspaces.iter().find(|w| w.active).cloned();
                match active {
                    Some(ws) => Some(CompositorEvent::WorkspaceChanged {
                        focused_window: self.active_window(),
                        workspace: ws,
                    }),
                    None => Some(CompositorEvent::WorkspacesUpdated(workspaces)),
                }
            }
            "WindowFocusChanged" => {
                self.active_window().map(CompositorEvent::WindowFocused)
            }
            "WindowOpenedOrChanged" => {
                Some(CompositorEvent::WorkspacesUpdated(self.workspaces()))
            }
            "WindowClosed" => {
                let id = obj["WindowClosed"]
                    .get("id")
                    .and_then(|v| v.as_u64())
                    .unwrap_or(0);
                Some(CompositorEvent::WindowClosed(Window {
                    title: String::new(),
                    class: id.to_string(),
                    workspace_id: 0,
                }))
            }
            "OutputConnected" => {
                let name = obj["OutputConnected"]
                    .get("output")
                    .and_then(|v| v.get("name"))
                    .and_then(|v| v.as_str())
                    .unwrap_or("")
                    .to_string();
                let mon = self.monitors().into_iter().find(|m| m.name == name);
                Some(CompositorEvent::MonitorAdded(mon.unwrap_or(Monitor {
                    name, width: 0, height: 0, scale: 1.0, primary: false,
                })))
            }
            "OutputDisconnected" => {
                let name = obj["OutputDisconnected"]
                    .get("output")
                    .and_then(|v| v.get("name"))
                    .and_then(|v| v.as_str())
                    .unwrap_or("")
                    .to_string();
                Some(CompositorEvent::MonitorRemoved(name))
            }
            _ => None,
        }
    }

    fn on_workspace_change(&mut self, _cb: Box<dyn Fn(Workspace) + Send>) {}
    fn on_window_focus(&mut self, _cb: Box<dyn Fn(Window) + Send>) {}
}

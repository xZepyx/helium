use std::cell::RefCell;
use std::collections::VecDeque;
use std::io::{BufRead, BufReader, ErrorKind, Write};
use std::os::unix::io::{AsRawFd, RawFd};
use std::os::unix::net::UnixStream;
use serde::Deserialize;
use super::compositor::{Compositor, CompositorEvent, Monitor, Window, WindowDiffusion, Workspace};

pub struct Niri {
    socket_path: String,
    event_reader: Option<BufReader<UnixStream>>,
    cache: RefCell<Option<Vec<(u64, Workspace)>>>,
    on_workspace_change_cb: Option<Box<dyn Fn(Workspace) + Send>>,
    on_window_focus_cb: Option<Box<dyn Fn(Window) + Send>>,
    previous_focus: Option<Window>,
    event_queue: VecDeque<CompositorEvent>,
}

impl Niri {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        let path = std::env::var("NIRI_SOCKET")
            .map_err(|_| crate::HeliumError::Compositor("NIRI_SOCKET not set".into()))?;
        UnixStream::connect(&path)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Niri: {e}")))?;
        let event_stream = UnixStream::connect(&path)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot open Niri event stream: {e}")))?;
        event_stream.set_nonblocking(true)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot set Niri event stream non-blocking: {e}")))?;
        let mut event_reader = BufReader::new(event_stream);
        event_reader.get_mut().write_all(b"{\"EventStream\":null}\n").ok();
        let mut ack = String::new();
        event_reader.read_line(&mut ack).ok();
        Ok(Niri { socket_path: path, event_reader: Some(event_reader), cache: RefCell::new(None), on_workspace_change_cb: None, on_window_focus_cb: None, previous_focus: None, event_queue: VecDeque::new() })
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

        let ok_val = val.get("Ok")?;
        if let Some(inner) = ok_val.as_object().and_then(|m| m.values().next()) {
            Some(inner.clone())
        } else {
            Some(ok_val.clone())
        }
    }

    fn fetch_workspaces(&self, with_windows: bool) -> Vec<Workspace> {
        let ws_data = match self.request("{\"Workspaces\":null}") {
            Some(v) => v,
            None => return vec![],
        };
        let mut raw: Vec<NiriWorkspace> = serde_json::from_value(ws_data).unwrap_or_default();
        raw.sort_by_key(|w| w.idx);

        let windows: Vec<NiriWindow> = if with_windows {
            self.request("{\"Windows\":null}")
                .and_then(|v| serde_json::from_value(v).ok())
                .unwrap_or_default()
        } else {
            vec![]
        };

        let pairs: Vec<(u64, Workspace)> = raw.into_iter().map(|w| {
            let niri_id = w.id;
            let pos = w.idx;
            let window_count = if with_windows {
                windows.iter()
                    .filter(|win| win.workspace_id == Some(niri_id))
                    .count() as u32
            } else {
                w.active_window_id.is_some() as u32
            };
            (niri_id, Workspace {
                id: pos as u32,
                name: w.name.unwrap_or_else(|| pos.to_string()),
                active: w.is_focused,
                occupied: window_count > 0,
                window_count,
                monitor: w.output.unwrap_or_default(),
            })
        }).collect();

        *self.cache.borrow_mut() = Some(pairs.clone());
        pairs.into_iter().map(|(_, w)| w).collect()
    }

    /// Fetch workspaces without querying windows separately (fast path).
    pub fn workspaces_inherent(&self) -> Vec<Workspace> {
        self.fetch_workspaces(false)
    }

    /// Fetch workspaces with full window count (slow path).
    pub fn workspaces_with_windows(&self) -> Vec<Workspace> {
        self.fetch_workspaces(true)
    }

    fn read_line_nonblocking(reader: &mut BufReader<UnixStream>) -> Option<String> {
        let mut line = String::new();
        match reader.read_line(&mut line) {
            Ok(0) => None,
            Ok(_) => {
                let trimmed = line.trim().to_string();
                if trimmed.is_empty() { None } else { Some(trimmed) }
            }
            Err(e) if e.kind() == ErrorKind::WouldBlock => None,
            Err(_) => None,
        }
    }
}

#[derive(Deserialize)]
struct NiriWorkspace {
    id: u64,
    idx: u64,
    name: Option<String>,
    output: Option<String>,
    is_focused: bool,
    active_window_id: Option<u64>,
}

#[derive(Deserialize)]
struct NiriWindow {
    workspace_id: Option<u64>,
}

impl Compositor for Niri {
    fn workspaces(&self) -> Vec<Workspace> {
        self.workspaces_inherent()
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
        struct FocusedWindow {
            title: Option<String>,
            app_id: Option<String>,
            workspace_id: Option<u64>,
        }
        let data = self.request("{\"FocusedWindow\":null}")?;
        let w: FocusedWindow = serde_json::from_value(data).ok()?;
        Some(Window {
            title: w.title.unwrap_or_default(),
            class: w.app_id.unwrap_or_default(),
            workspace_id: (w.workspace_id.unwrap_or(0) as u32).saturating_add(1),
        })
    }

    fn event_fd(&self) -> Option<RawFd> {
        self.event_reader.as_ref().map(|r| r.get_ref().as_raw_fd())
    }

    fn poll_event(&mut self) -> Option<CompositorEvent> {
        // Drain the event queue first
        if let Some(event) = self.event_queue.pop_front() {
            return Some(event);
        }

        let reader = self.event_reader.as_mut()?;
        let line = Self::read_line_nonblocking(reader)?;
        let val: serde_json::Value = serde_json::from_str(&line).ok()?;
        let obj = val.as_object()?;
        let event_type = obj.keys().next()?.as_str();

        match event_type {
            "WorkspacesChanged" => {
                // Event contains full workspace list — parse directly, no IPC
                let payload = &obj[event_type];
                let workspaces_val = payload.get("workspaces")?.clone();
                let mut raw: Vec<NiriWorkspace> = serde_json::from_value(workspaces_val).ok()?;
                raw.sort_by_key(|w| w.idx);
                let pairs: Vec<(u64, Workspace)> = raw.into_iter().map(|w| {
                    let pos = w.idx;
                    (w.id, Workspace {
                        id: pos as u32,
                        name: w.name.unwrap_or_else(|| pos.to_string()),
                        active: w.is_focused,
                        occupied: w.active_window_id.is_some(),
                        window_count: w.active_window_id.is_some() as u32,
                        monitor: w.output.unwrap_or_default(),
                    })
                }).collect();
                *self.cache.borrow_mut() = Some(pairs.clone());
                Some(CompositorEvent::WorkspacesUpdated(pairs.into_iter().map(|(_, w)| w).collect()))
            }
            "WorkspaceActivated" => {
                // Event has {id, focused} — update cache in-place, no IPC
                let payload = &obj[event_type];
                let niri_id = payload.get("id").and_then(|v| v.as_u64())?;
                let focused = payload.get("focused").and_then(|v| v.as_bool()).unwrap_or(false);
                let mut cache = self.cache.borrow_mut();
                let event = if let Some(ref mut list) = *cache {
                    for (_, ws) in list.iter_mut() {
                        ws.active = false;
                    }
                    if let Some((_, ws)) = list.iter_mut().find(|(id, _)| *id == niri_id) {
                        ws.active = focused;
                        if let Some(ref cb) = self.on_workspace_change_cb {
                            cb(ws.clone());
                        }
                    }
                    Some(CompositorEvent::WorkspacesUpdated(
                        list.iter().map(|(_, w)| w.clone()).collect()
                    ))
                } else {
                    drop(cache);
                    Some(CompositorEvent::WorkspacesUpdated(self.workspaces()))
                };
                event
            }
            "WindowFocusChanged" => {
                let window = self.active_window();
                let unfocused = self.previous_focus.take();
                if let Some(ref w) = window {
                    if let Some(ref cb) = self.on_window_focus_cb {
                        cb(w.clone());
                    }
                }
                let diffusion = WindowDiffusion { unfocused, focused: window.clone() };
                self.previous_focus = window.clone();
                self.event_queue.push_back(CompositorEvent::WindowDiffusion(diffusion));
                window.map(CompositorEvent::WindowFocused)
            }
            "WindowOpenedOrChanged" => {
                Some(CompositorEvent::WorkspacesUpdated(self.workspaces_with_windows()))
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

    fn on_workspace_change(&mut self, cb: Box<dyn Fn(Workspace) + Send>) {
        self.on_workspace_change_cb = Some(cb);
    }
    fn on_window_focus(&mut self, cb: Box<dyn Fn(Window) + Send>) {
        self.on_window_focus_cb = Some(cb);
    }
}

use std::io::{BufRead, BufReader, Write};
use std::os::unix::net::UnixStream;
use std::sync::{Arc, Mutex};

use serde::Deserialize;

use super::compositor::{Compositor, Monitor, Window, Workspace};

#[derive(Deserialize)]
struct MangoMonitorResponse {
    name: String,
    #[serde(default)]
    width: u32,
    #[serde(default)]
    height: u32,
    #[serde(default)]
    scale: f64,
}

#[derive(Deserialize)]
struct MangoClient {
    #[serde(default)]
    title: Option<String>,
    #[serde(default)]
    app_id: Option<String>,
    #[serde(default)]
    tags: Vec<u32>,
}

/// MangoWM compositor backend.
///
/// Communicates with the compositor over a Unix socket using the `mmsg`
/// protocol. The socket path is read from the `MANGO_INSTANCE_SIGNATURE`
/// environment variable.
pub struct MangoWM {
    socket_path: String,
    stream: Mutex<UnixStream>,
    workspace_callbacks: Arc<Mutex<Vec<Box<dyn Fn(Workspace) + Send>>>>,
    window_callbacks: Arc<Mutex<Vec<Box<dyn Fn(Window) + Send>>>>,
}

impl MangoWM {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        let path = std::env::var("MANGO_INSTANCE_SIGNATURE").map_err(|_| {
            crate::HeliumError::Compositor("MANGO_INSTANCE_SIGNATURE not set".into())
        })?;
        let stream =
            UnixStream::connect(&path).map_err(|e| {
                crate::HeliumError::Compositor(format!("cannot connect to MangoWM: {e}"))
            })?;
        let cb = Arc::new(Mutex::new(Vec::new()));
        let wb = Arc::new(Mutex::new(Vec::new()));

        Ok(MangoWM {
            socket_path: path,
            stream: Mutex::new(stream),
            workspace_callbacks: cb,
            window_callbacks: wb,
        })
    }

    pub fn is_running() -> bool {
        std::env::var("MANGO_INSTANCE_SIGNATURE").is_ok()
    }

    fn send_cmd(&self, cmd: &str) -> Result<String, crate::HeliumError> {
        let mut stream = self.stream.lock().unwrap();
        let full = format!("{cmd}\n");
        stream
            .write_all(full.as_bytes())
            .map_err(|e| crate::HeliumError::Compositor(format!("MangoWM send: {e}")))?;
        let mut reader = BufReader::new(&*stream);
        let mut line = String::new();
        reader
            .read_line(&mut line)
            .map_err(|e| crate::HeliumError::Compositor(format!("MangoWM recv: {e}")))?;
        Ok(line.trim().to_string())
    }
}

impl Compositor for MangoWM {
    fn workspaces(&self) -> Vec<Workspace> {
        let resp = match self.send_cmd("get all-tags") {
            Ok(r) => r,
            Err(_) => return vec![],
        };
        let parsed: Result<serde_json::Value, _> = serde_json::from_str(&resp);
        let val = match parsed {
            Ok(v) => v,
            Err(_) => return vec![],
        };
        let obj = match val.as_object() {
            Some(o) => o,
            None => return vec![],
        };
        let mut workspaces = Vec::new();
        for (monitor_name, tags_val) in obj {
            let tags = match tags_val.as_array() {
                Some(a) => a,
                None => continue,
            };
            for tag_val in tags {
                let index = tag_val.get("index").and_then(|v| v.as_u64()).unwrap_or(0);
                let focused = tag_val
                    .get("focused")
                    .and_then(|v| v.as_bool())
                    .unwrap_or(false);
                workspaces.push(Workspace {
                    id: index as u32,
                    name: tag_val
                        .get("name")
                        .and_then(|v| v.as_str())
                        .unwrap_or(&index.to_string())
                        .to_string(),
                    active: focused,
                    occupied: false,
                    monitor: monitor_name.clone(),
                });
            }
        }
        workspaces
    }

    fn active_workspace(&self) -> Option<Workspace> {
        let resp = match self.send_cmd("get focusing-client") {
            Ok(r) => r,
            Err(_) => return None,
        };
        let client: Result<MangoClient, _> = serde_json::from_str(&resp);
        let client = match client {
            Ok(c) => c,
            Err(_) => {
                let workspaces = self.workspaces();
                return workspaces.into_iter().find(|w| w.active);
            }
        };
        let active_tag = client.tags.first().copied().unwrap_or(1);
        let resp = match self.send_cmd("get all-tags") {
            Ok(r) => r,
            Err(_) => return None,
        };
        let parsed: Result<serde_json::Value, _> = serde_json::from_str(&resp);
        let val = match parsed {
            Ok(v) => v,
            Err(_) => return None,
        };
        let obj = match val.as_object() {
            Some(o) => o,
            None => return None,
        };
        for (monitor_name, tags_val) in obj {
            let tags = match tags_val.as_array() {
                Some(a) => a,
                None => continue,
            };
            for tag_val in tags {
                let index = tag_val.get("index").and_then(|v| v.as_u64()).unwrap_or(0);
                if index == active_tag as u64 {
                    return Some(Workspace {
                        id: index as u32,
                        name: tag_val
                            .get("name")
                            .and_then(|v| v.as_str())
                            .unwrap_or(&index.to_string())
                            .to_string(),
                        active: true,
                        occupied: false,
                        monitor: monitor_name.clone(),
                    });
                }
            }
        }
        None
    }

    fn monitors(&self) -> Vec<Monitor> {
        let resp = match self.send_cmd("get all-monitors") {
            Ok(r) => r,
            Err(_) => return vec![],
        };
        let monitors: Result<Vec<MangoMonitorResponse>, _> = serde_json::from_str(&resp);
        match monitors {
            Ok(list) => list
                .into_iter()
                .map(|m| Monitor {
                    name: m.name,
                    width: m.width,
                    height: m.height,
                    scale: m.scale.max(1.0),
                    primary: false,
                })
                .collect(),
            Err(_) => vec![],
        }
    }

    fn on_workspace_change(&mut self, cb: Box<dyn Fn(Workspace) + Send>) {
        self.workspace_callbacks.lock().unwrap().push(cb);
        if self.workspace_callbacks.lock().unwrap().len() != 1 {
            return;
        }
        let path = self.socket_path.clone();
        let cbs = self.workspace_callbacks.clone();
        std::thread::spawn(move || {
            let stream = match UnixStream::connect(&path) {
                Ok(s) => s,
                Err(_) => return,
            };
            let mut w = &stream;
            let _ = w.write_all(b"watch all-tags\n");
            let reader = BufReader::new(&stream);
            for line in reader.lines().map_while(Result::ok) {
                let parsed: Result<serde_json::Value, _> = serde_json::from_str(&line);
                let val = match parsed {
                    Ok(v) => v,
                    Err(_) => continue,
                };
                let obj = match val.as_object() {
                    Some(o) => o,
                    None => continue,
                };
                for (monitor_name, tags_val) in obj {
                    let tags = match tags_val.as_array() {
                        Some(a) => a,
                        None => continue,
                    };
                    for tag_val in tags {
                        let focused = tag_val
                            .get("focused")
                            .and_then(|v| v.as_bool())
                            .unwrap_or(false);
                        if !focused {
                            continue;
                        }
                        let index =
                            tag_val.get("index").and_then(|v| v.as_u64()).unwrap_or(0);
                        let ws = Workspace {
                            id: index as u32,
                            name: tag_val
                                .get("name")
                                .and_then(|v| v.as_str())
                                .unwrap_or(&index.to_string())
                                .to_string(),
                            active: true,
                            occupied: false,
                            monitor: monitor_name.clone(),
                        };
                        for cb in cbs.lock().unwrap().iter() {
                            cb(ws.clone());
                        }
                    }
                }
            }
        });
    }

    fn on_window_focus(&mut self, cb: Box<dyn Fn(Window) + Send>) {
        self.window_callbacks.lock().unwrap().push(cb);
        if self.window_callbacks.lock().unwrap().len() != 1 {
            return;
        }
        let path = self.socket_path.clone();
        let cbs = self.window_callbacks.clone();
        std::thread::spawn(move || {
            let stream = match UnixStream::connect(&path) {
                Ok(s) => s,
                Err(_) => return,
            };
            let _ = (&stream).write_all(b"watch focusing-client\n");
            let reader = BufReader::new(&stream);
            for line in reader.lines().map_while(Result::ok) {
                let client: Result<MangoClient, _> = serde_json::from_str(&line);
                let client = match client {
                    Ok(c) => c,
                    Err(_) => continue,
                };
                let win = Window {
                    title: client.title.unwrap_or_default(),
                    class: client.app_id.unwrap_or_default(),
                    workspace_id: client.tags.first().copied().unwrap_or(1),
                };
                for cb in cbs.lock().unwrap().iter() {
                    cb(win.clone());
                }
            }
        });
    }
}

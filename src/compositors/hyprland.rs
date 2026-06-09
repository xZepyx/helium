use std::io::{Read, Write};
use std::os::unix::net::UnixStream;
use std::path::PathBuf;

use serde::Deserialize;

use super::compositor::{Compositor, Monitor, Window, Workspace};

#[derive(Deserialize)]
struct HyprlandWorkspace {
    id: i64,
    name: String,
    monitor: String,
    windows: i64,
}

pub struct Hyprland {
    socket: PathBuf,
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
        // Verify the socket is reachable
        UnixStream::connect(&socket)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Hyprland: {e}")))?;
        Ok(Hyprland { socket })
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
            monitor: w.monitor,
        })
    }

    fn monitors(&self) -> Vec<Monitor> {
        vec![]
    }

    fn on_workspace_change(&mut self, _cb: Box<dyn Fn(Workspace) + Send>) {}

    fn on_window_focus(&mut self, _cb: Box<dyn Fn(Window) + Send>) {}
}

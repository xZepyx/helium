#![allow(dead_code)]

use std::io::{Read, Write};
use std::os::unix::net::UnixStream;

use super::compositor::{Compositor, Monitor, Window, Workspace};

struct IpcMessage {
    msg_type: u32,
    payload: Vec<u8>,
}

const MAGIC: &[u8; 6] = b"i3-ipc";
const GET_WORKSPACES: u32 = 1;
const SUBSCRIBE: u32 = 2;

fn send_ipc(stream: &mut UnixStream, msg_type: u32, payload: &[u8]) -> std::io::Result<IpcMessage> {
    let mut header = Vec::with_capacity(14);
    header.extend_from_slice(MAGIC);
    header.extend_from_slice(&(payload.len() as u32).to_le_bytes());
    header.extend_from_slice(&msg_type.to_le_bytes());
    stream.write_all(&header)?;
    stream.write_all(payload)?;

    let mut magic = [0u8; 6];
    stream.read_exact(&mut magic)?;
    let mut len_buf = [0u8; 4];
    stream.read_exact(&mut len_buf)?;
    let len = u32::from_le_bytes(len_buf) as usize;
    let mut type_buf = [0u8; 4];
    stream.read_exact(&mut type_buf)?;
    let response_type = u32::from_le_bytes(type_buf);
    let mut payload = vec![0u8; len];
    stream.read_exact(&mut payload)?;
    Ok(IpcMessage {
        msg_type: response_type,
        payload,
    })
}

pub struct Sway {
    stream: UnixStream,
}

impl Sway {
    pub fn connect() -> Result<Self, crate::HeliumError> {
        let path = std::env::var("SWAYSOCK")
            .map_err(|_| crate::HeliumError::Compositor("SWAYSOCK not set".into()))?;
        let stream = UnixStream::connect(&path)
            .map_err(|e| crate::HeliumError::Compositor(format!("cannot connect to Sway: {e}")))?;
        Ok(Sway { stream })
    }

    pub fn is_running() -> bool {
        std::env::var("SWAYSOCK").is_ok()
    }
}

impl Compositor for Sway {
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

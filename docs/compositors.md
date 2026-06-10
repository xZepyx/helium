# Compositors

Helium provides a unified `Compositor` trait and auto-detection for several
Wayland compositors. You write code against the trait, and Helium figures
out which IPC socket to poke.

## Auto-detection

```rust
use helium_wsl::compositors;

let mut compositor = compositors::detect()?;
let workspaces = compositor.workspaces();
```

Detection checks environment variables in order:

| Variable | Compositor | Feature flag |
|----------|-----------|--------------|
| `HYPRLAND_INSTANCE_SIGNATURE` | Hyprland | `compositor-hyprland` |
| `NIRI_SOCKET` | Niri | `compositor-niri` |
| `SWAYSOCK` | Sway / i3 | `compositor-sway` |
| — (always checked last) | MangoWM | `compositor-mangowm` |

If none of the env vars are present, `detect()` returns an error.

## The `Compositor` trait

```rust
pub trait Compositor: Send {
    fn workspaces(&self) -> Vec<Workspace>;
    fn active_workspace(&self) -> Option<Workspace>;
    fn monitors(&self) -> Vec<Monitor>;
    fn on_workspace_change(&mut self, cb: Box<dyn Fn(Workspace) + Send>);
    fn on_window_focus(&mut self, cb: Box<dyn Fn(Window) + Send>);
    fn active_window(&self) -> Option<Window> { None }
    fn event_fd(&self) -> Option<RawFd> { None }
    fn poll_event(&mut self) -> Option<CompositorEvent> { None }
}
```

### Data types

```rust
pub struct Workspace {
    pub id: u32,
    pub name: String,
    pub active: bool,
    pub occupied: bool,
    pub monitor: String,
}

pub struct Monitor {
    pub name: String,
    pub width: u32,
    pub height: u32,
    pub scale: f64,
    pub primary: bool,
}

pub struct Window {
    pub title: String,
    pub class: String,
    pub workspace_id: u32,
}

pub enum CompositorEvent {
    WorkspaceChanged(Workspace),
    WorkspacesUpdated(Vec<Workspace>),
    WindowFocused(Window),
    WindowClosed(Window),
    MonitorAdded(Monitor),
    MonitorRemoved(String),
}
```

## Struct-level constructors

| Struct | Constructor |
|--------|-------------|
| `Hyprland` | `Hyprland::connect()` |
| `Niri` | `Niri::connect()` |
| `Sway` | `Sway::connect()` |
| `MangoWM` | `MangoWM::connect()` |

All implement `Compositor` and can be used directly (bypassing `detect`
if you already know what compositor is running).

## Attaching to a shell

```rust
use helium_wsl::{Helium, compositors};

let compositor = compositors::detect()?;

let mut shell = Helium::from_file("bar.slint")
    .surface("main")
    .build()?;

shell.attach_compositor(compositor)
    .on_ready(|sh| {
        let ws = sh.compositors()[0].workspaces();
        sh.set("main", "num_workspaces", ws.len() as i32);
    })
    .run()?;
```

### Push-based event handling

If the compositor supports an event socket (Hyprland does), use
`on_compositor_event` to receive events via the event fd instead of
polling:

```rust
let compositor = compositors::detect()?;

shell.on_compositor_event(compositor, |event, ctx| {
    match event {
        CompositorEvent::WorkspaceChanged(ws) => {
            ctx.set("main", "active_ws", ws.id as i32);
        }
        _ => {}
    }
})?;
```

The compositor is consumed and kept alive inside the event loop. The
callback receives a `CompositorEvent` and a `TickContext` for updating
surface properties.

## Implemented backends

### Hyprland

Connects via `$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket.sock`
for commands and `.socket2.sock` for events. Uses Hyprland's JSON IPC.
Supports `event_fd()` and `poll_event()` for push-based workspace,
window, and monitor events.

### Niri

Connects via `$NIRI_SOCKET`. Requires `compositor-niri` feature.

### Sway

Connects via `$SWAYSOCK`. Requires `compositor-sway` feature.

### MangoWM

Stub — no public IPC documentation available.

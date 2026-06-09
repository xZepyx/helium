# Compositors

Helium provides a unified `Compositor` trait and auto-detection for several
Wayland compositors. You write code against the trait, and Helium figures
out which IPC socket to poke.

---

## Auto-detection

```rust
use helium::compositors;

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

You can also enable individual backends by feature flag instead of the
full `compositors` group — useful if you only target one DE and want to
shave a few seconds off your compile.

---

## The Compositor trait

```rust
pub trait Compositor: Send {
    fn workspaces(&self) -> Vec<Workspace>;
    fn active_workspace(&self) -> Option<Workspace>;
    fn monitors(&self) -> Vec<Monitor>;
    fn on_workspace_change(&mut self, cb: Box<dyn Fn(Workspace) + Send>);
    fn on_window_focus(&mut self, cb: Box<dyn Fn(Window) + Send>);
}
```

### Data types

```rust
pub struct Workspace {
    pub id: u32,
    pub name: String,
    pub active: bool,
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
```

## Compositor types returned

| Struct | Constructor |
|--------|-------------|
| `Hyprland` | `Hyprland::connect()` |
| `Niri` | `Niri::connect()` |
| `Sway` | `Sway::connect()` |
| `MangoWM` | `MangoWM::connect()` |

All implement `Compositor` and can be used directly (bypassing `detect`
if you already know what compositor is running).

---

## Attaching to HeliumRuntime

Once you have a compositor, attach it to the runtime to keep it alive for
the duration of the event loop:

```rust
let mut runtime = Helium::from_file("bar.slint")
    .surface("Main")
    .build()?;

let compositor = compositors::detect()?;

runtime.attach_compositor(compositor)
    .on_ready(|rt| {
        let ws = rt.compositors()[0].workspaces();
        rt.set("Main", "num_workspaces", ws.len() as i32);
    })
    .run()?;
```

### Polling on a timer

If your compositor does not support async event subscriptions (or you just
want to poll), use `on_compositor_event`:

```rust
runtime.on_compositor_event(compositor, Duration::from_millis(200), |ctx, comp| {
    if let Some(ws) = comp.active_workspace() {
        ctx.set("Main", "active_ws_id", ws.id as i32);
    }
})?;
```

The compositor is consumed and kept alive inside the event loop.

---

## Implemented backends

### Hyprland

Connects via `$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket.sock`.
Uses Hyprland's JSON IPC.

### Niri

Connects via `$NIRI_SOCKET`. Uses JSON IPC.

### Sway

Connects via `$SWAYSOCK`. Uses the i3/Sway binary IPC protocol
(magic string `"i3-ipc"`, length-prefixed messages with a 32-bit
payload length and type).

### MangoWM

Currently stubbed. MangoWM does not have public IPC documentation.
If you know how it works, pull requests are very welcome — the
protocol is probably simple once someone decodes it.

# Compositors

Helium provides a unified `Compositor` trait and auto-detection for several
Wayland compositors.

## Auto-detection

```rust
use helium::compositors;

let mut compositor = compositors::detect()?;
let workspaces = compositor.workspaces();
```

Detection checks env vars in order:

| Variable | Compositor |
|---|---|
| `HYPRLAND_INSTANCE_SIGNATURE` | Hyprland |
| `NIRI_SOCKET` | Niri |
| `SWAYSOCK` | Sway |

If none are set, `detect()` returns an error.

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

## Implemented backends

### Hyprland

Connects via `$XDG_RUNTIME_DIR/hypr/$HYPRLAND_INSTANCE_SIGNATURE/.socket.sock`.
Uses Hyprland's JSON IPC.

### Niri

Connects via `$NIRI_SOCKET`. Uses JSON IPC.

### Sway

Connects via `$SWAYSOCK`. Uses the i3/Sway binary IPC protocol (magic string
`"i3-ipc"`, length-prefixed messages).

### MangoWM

Stubbed with `todo!()`. MangoWM doesn't have public IPC documentation yet.
If you know the protocol, contributions are welcome.

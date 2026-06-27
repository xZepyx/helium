# Shell API

Helium's public API covers anchors, the shell builder, the runtime handle
(`ShellInstance`), macros, context handles, and a raw escape hatch to
`layer-shika`.

## Building a shell

Start with `Helium::from_file(path)`, configure surfaces via
`SurfaceInitializer`, then `.build()` to get a `ShellInstance`.

```rust
use helium_wsl::{Helium, AnchorEdge, Layer, MonitorPolicy};

let mut shell = Helium::from_file("ui/bar.slint")
    .surface("main")
        .height(42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .exclusive()
        .monitors(MonitorPolicy::Primary)
    .build()?;
```

Alternative entry points:

| Method | Description |
|--------|-------------|
| `Helium::from_file(path)` | Load a `.slint` file from disk |
| `Helium::from_source(code)` | Parse a Slint source string |
| `Helium::from_compilation(compilation)` | Use a pre-compiled `CompilationResult` |

### SurfaceInitializer methods

| Method | Description |
|--------|-------------|
| `.surface(name)` | Start configuring a new surface (or switch to the next one) |
| `.width(w)` | Set surface width in pixels |
| `.height(h)` | Set surface height in pixels |
| `.size(w, h)` | Set both width and height |
| `.anchor(edges)` | Set anchor edges (tuple of `AnchorEdge` values) |
| `.exclusive()` | Infer exclusive zone from dimensions |
| `.exclusive_zone(z)` | Set exclusive zone manually |
| `.namespace(ns)` | Set the app id / namespace |
| `.monitors(policy)` | Set `MonitorPolicy` (All, Primary, Named) |
| `.layer(layer)` | Set the `Layer` (Background, Bottom, Top, Overlay) |
| `.margin(t, r, b, l)` | Set margins in pixels |
| `.keyboard(mode)` | Set keyboard interactivity (stubbed) |
| `.interactivity(b)` | Toggle click-through (stubbed) |
| `.build()` | Build and return `ShellInstance` |
| `.run()` | Build and run the event loop (blocks) |

### MonitorPolicy

```rust
pub enum MonitorPolicy {
    All,
    Primary,
    Named(String),
}
```

### KeyboardMode

```rust
pub enum KeyboardMode {
    None,
    OnDemand,
    Exclusive,
}
```

## ShellInstance

The runtime handle for an active shell. Wraps a `layer_shika::Shell`
and adds convenience methods.

### `set` — write a Slint property

```rust
pub fn set(&mut self, surface: &str, prop: &str, value: impl IntoSlintValue)
```

Sets a property on a named surface. The value is converted automatically
via the `IntoSlintValue` trait.

```rust
shell.set("main", "title", "Hello, World!");
shell.set("main", "count", 42);
shell.set("main", "visible", true);
```

### `get` — read a Slint property

```rust
pub fn get(&self, surface: &str, prop: &str) -> Option<slint_interpreter::Value>
```

Returns `None` if the surface or property does not exist.

```rust
if let Some(val) = shell.get("main", "title") {
    println!("title = {val:?}");
}
```

### `run` — start the event loop

Blocks the current thread. If an `on_ready` callback was registered, it
fires just before the loop starts.

```rust
shell.run()?;
```

### `on_ready` — one-shot setup callback

Fires once just before the event loop starts. Receives `&mut ShellInstance`.

```rust
shell.on_ready(|sh| {
    sh.set("main", "initialized", true);
});
```

### `on_tick` — recurring timer

```rust
pub fn on_tick(
    &mut self,
    interval: Duration,
    cb: impl FnMut(&mut TickContext) + 'static,
) -> Result<(), HeliumError>
```

Registers a repeating timer. The callback receives a `TickContext`.

```rust
shell.on_tick(Duration::from_millis(1000), |ctx| {
    ctx.set("main", "time", "14:30");
})?;
```

### `on_ipc` — file descriptor event source

```rust
pub fn on_ipc(
    &mut self,
    fd: RawFd,
    cb: impl FnMut(&mut IpcContext) + 'static,
) -> Result<(), HeliumError>
```

Watches a raw file descriptor for readability.

```rust
use std::os::unix::net::UnixStream;
use std::os::unix::io::IntoRawFd;

let stream = UnixStream::connect("/tmp/my-ipc.sock")?;
shell.on_ipc(stream.into_raw_fd(), |ctx| {
    ctx.set("main", "ipc_ready", true);
})?;
```

### `attach_compositor` / `compositors` / `compositors_mut`

```rust
pub fn attach_compositor(&mut self, compositor: Box<dyn Compositor>) -> &mut Self
pub fn compositors(&self) -> &[Box<dyn Compositor>]
pub fn compositors_mut(&mut self) -> &mut [Box<dyn Compositor>]
```

Attach a compositor handle that stays alive for the shell's lifetime.

```rust
let compositor = helium_wsl::compositors::detect()?;
shell.attach_compositor(compositor)
    .on_ready(|sh| {
        let ws = sh.compositors()[0].workspaces();
        sh.set("main", "workspace_count", ws.len() as i32);
    });
```

### `on_compositor_event` — push-based compositor events

```rust
pub fn on_compositor_event(
    &mut self,
    compositor: Box<dyn Compositor>,
    cb: impl FnMut(CompositorEvent, &mut TickContext) + 'static,
) -> Result<(), HeliumError>
```

Connects to the compositor's event socket (via `event_fd()`) and fires
the callback when events arrive. The compositor is consumed.

```rust
let compositor = compositors::detect()?;
shell.on_compositor_event(compositor, |event, ctx| {
    if let CompositorEvent::WorkspaceChanged { workspace, .. } = event {
        ctx.set("main", "active_ws", workspace.id as i32);
    }
})?;
```

### `attach_adapters` — wire an adapter registry into the event loop

```rust
pub fn attach_adapters(
    &mut self,
    registry: AdapterRegistry,
    tick_interval: Duration,
    surface: &str,
) -> Result<(), HeliumError>
```

Registers a repeating timer that ticks all adapters and applies their
property changes to the named surface.

```rust
use std::time::Duration;
use helium_wsl::adapters::{AdapterRegistry, ClockAdapter, WorkspacesAdapter};

let registry = AdapterRegistry::new(adapters! {
    "clock" => ClockAdapter { format: "%H:%M".into(), ..Default::default() },
    "workspaces" => WorkspacesAdapter { max: 9 },
});
shell.attach_adapters(registry, Duration::from_secs(1), "main")?;
```

### `surface_names` — list registered surfaces

```rust
pub fn surface_names(&self) -> Vec<String>
```

### `with_all_surfaces` — iterate surfaces

```rust
shell.with_all_surfaces(|name| {
    println!("surface: {name}");
});
```

### `update` — bulk property update

```rust
shell.update("main", |batch| {
    batch.set("title", "Hello");
    batch.set("count", 42);
});
```

### `on_property_change` — react to Slint property changes

```rust
shell.on_property_change("main", "counter", |val| {
    println!("counter changed to {val:?}");
});
```

Note: callbacks are stored but currently not wired to the component
instance (waiting on slint-interpreter API).

### `on_signal` — react to Slint signals

```rust
shell.on_signal("main", "clicked", || {
    println!("button clicked!");
});
```

Signals are wired to the Slint component via `set_callback()`.
Property change callbacks are stored but not wired (waiting on
slint-interpreter property notification hooks).

### `on_surface_ready` — per-surface ready callback

```rust
shell.on_surface_ready("main", |sh| {
    sh.set("main", "ready", true);
});
```

### `on_surface_created` / `on_surface_destroyed`

```rust
shell.on_surface_created(|name| {
    println!("surface {name} created");
});
shell.on_surface_destroyed(|name| {
    println!("surface {name} destroyed");
});
```

### `on_event` / `emit` — event bus

```rust
shell.on_event("config-changed", |val| {
    println!("config changed: {val:?}");
});

shell.emit("config-changed", "reloaded");
```

### `on_key` — keyboard input

```rust
shell.on_key("main", Key::Escape, |event| {
    if event.pressed {
        println!("Escape pressed");
    }
});
```

Note: callbacks are stored but currently not wired (waiting on
layer-shika keyboard API).

### `hide` / `show`

```rust
shell.hide("main");
shell.show("main");
```

Currently stubbed (waiting on layer-shika visibility API).

### `reload_ui`

```rust
shell.reload_ui("main", "new-bar.slint");
```

Currently stubbed (waiting on layer-shika component replacement API).

### `pause` / `resume`

```rust
shell.pause();
shell.resume();
```

Suspend or resume rendering and update callbacks.

### `set_on` / `surface_monitor`

```rust
shell.set_on("main", "DP-1", "title", "Hello");
if let Some(monitor) = shell.surface_monitor("main") {
    println!("main is on {monitor}");
}
```

`set_on` currently falls back to `set` (per-monitor targeting pending).

### `into_inner` — raw access

```rust
let inner: layer_shika::Shell = shell.into_inner();
```

Consumes the runtime and returns the underlying `layer_shika::Shell`.

## TickContext and IpcContext

Two lightweight handles that wrap `&mut AppState` and expose `set` and `get`
for surface properties.

| Handle | When you get one |
|--------|-----------------|
| `TickContext` | Inside `on_tick` callbacks |
| `IpcContext` | Inside `on_ipc` callbacks |

```rust
impl TickContext<'_> {
    pub fn set(&mut self, surface: &str, prop: &str, value: impl IntoSlintValue);
    pub fn get(&self, surface: &str, prop: &str) -> Option<slint_interpreter::Value>;
}

impl IpcContext<'_> {
    pub fn set(&mut self, surface: &str, prop: &str, value: impl IntoSlintValue);
    pub fn get(&self, surface: &str, prop: &str) -> Option<slint_interpreter::Value>;
}
```

## PropertyBatch

Obtained via `ShellInstance::update`. Properties are applied when the
closure returns.

```rust
impl PropertyBatch<'_> {
    pub fn set(&mut self, prop: &str, value: impl IntoSlintValue);
    pub fn get(&self, prop: &str) -> Option<slint_interpreter::Value>;
}
```

## IntoSlintValue

Trait for automatic conversion of Rust values into `slint_interpreter::Value`.

| Rust type | Slint variant |
|-----------|--------------|
| `f64`, `f32`, `u8`–`u64`, `i32`, `i64` | `Value::Number(f64)` |
| `bool` | `Value::Bool(bool)` |
| `String`, `&str` | `Value::String(SharedString)` |
| `slint_interpreter::Value` | identity (pass through) |

## Key and KeyEvent

```rust
pub enum Key {
    Escape, Return, Space, Tab, Backspace,
    Up, Down, Left, Right,
    Home, End, PageUp, PageDown, Insert, Delete,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Raw(u32),
}

pub struct Modifiers {
    pub ctrl: bool,
    pub alt: bool,
    pub shift: bool,
    pub super_key: bool,
}

pub struct KeyEvent {
    pub key: Key,
    pub modifiers: Modifiers,
    pub pressed: bool,
}
```

## Raw access

The full `layer-shika` API lives at `helium_wsl::raw`:

```rust
use helium_wsl::raw::layer_shika;
```

## Re-exports

The crate re-exports `chrono`, `slint`, `slint_interpreter`, and `Layer`
for convenience.

## Prelude

```rust
use helium_wsl::prelude::*;
```

Brings into scope: `AnchorEdge`, `CompositorEvent`, `Helium`,
`IntoSlintValue`, `IpcContext`, `Key`, `KeyboardMode`, `KeyEvent`,
`Layer`, `Modifiers`, `MonitorPolicy`, `PropertyBatch`,
`ShellInitializer`, `ShellInstance`, `SurfaceInitializer`, `TickContext`.

## Macros

### `helium_config!`

Declare typed config structs with defaults. See [Config](config.md).

### `helium_struct!`

Generate a plain struct with `Debug`, `Clone`, `PartialEq`, public fields,
and a `new()` constructor.

```rust
helium_struct! {
    Foo {
        name: String,
        count: i32,
    }
}
```

### `helium_model!`

Generate a Slint-compatible model wrapper around `Vec<T>`.

```rust
helium_model! { MyModel(String) }

let model = MyModel::from_vec(vec!["a".into(), "b".into()]);
let value: slint_interpreter::Value = model.into();
```

The generated type wraps `slint::VecModel<T>` and exposes `push`, `clear`,
`from_vec`, `set_vec`, and `row_count`. It implements
`Into<slint_interpreter::Value>`.

### `adapters!`

Concise adapter registration. See [Adapters](adapters.md).

## HeliumError

```rust
pub enum HeliumError {
    Compositor(String),
    Shell(String),
    Service(String),
    Io(std::io::Error),
    Config(config::ConfigError),
}
```

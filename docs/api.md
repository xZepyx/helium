# API

Helium's public API covers anchors, the shell builder, the runtime handle
(`HeliumRuntime`), macros, and a raw escape hatch to `layer-shika`.

---

## Anchors

Helium replaces `layer-shika`'s bitflag-based anchors with a variadic tuple
API. Fewer ways to accidentally `OR` the wrong bits together.

### AnchorEdge

```rust
pub enum AnchorEdge {
    Top,
    Bottom,
    Left,
    Right,
}
```

### Passing anchors to a surface

Use tuples of 1–4 `AnchorEdge` values with the `.anchor()` method:

```rust
use helium::{AnchorEdge, SurfaceBuilder};

surface.anchor((AnchorEdge::Top,));
surface.anchor((AnchorEdge::Top, AnchorEdge::Left));
surface.anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right));
surface.anchor((AnchorEdge::Top, AnchorEdge::Bottom, AnchorEdge::Left, AnchorEdge::Right));
```

### How it works

`IntoAnchorEdges` is a sealed trait implemented for tuples of 1 through 4
`AnchorEdge` values. Under the hood, each tuple is converted to a
`layer_shika::AnchorEdges`. The sealed pattern means you cannot accidentally
pass a random 4-tuple of `AnchorEdge` and have it do something unexpected.

---

## HeliumBuilder / HeliumSurfaceBuilder

Entry point for constructing a shell. Start with `Helium::from_file(path)`,
configure surfaces, then `.build()` to get a `HeliumRuntime`.

```rust
use helium::{Helium, AnchorEdge, MonitorPolicy};

Helium::from_file("ui/bar.slint")
    .surface("Main")
        .height(42)
        .anchor((AnchorEdge::Top, AnchorEdge::Left, AnchorEdge::Right))
        .exclusive()
        .monitors(MonitorPolicy::Primary)
    .build()?
    .run()?;
```

### MonitorPolicy

Controls which outputs a surface appears on:

```rust
pub enum MonitorPolicy {
    All,        // Every connected monitor
    Primary,    // The monitor marked as primary
    Named(String), // A specific monitor by name
}
```

---

## HeliumRuntime

The runtime handle is what you interact with during the event loop.
It wraps a `layer_shika::Shell` and adds convenience methods.

### `set` — write a Slint property

```rust
pub fn set(&mut self, surface: &str, prop: &str, value: impl IntoSlintValue)
```

Sets a property on a named surface. The value is converted automatically
via the `IntoSlintValue` trait — no manual `Value::Number(...)` wrapping.

```rust
runtime.set("Main", "title", "Hello, World!");
runtime.set("Main", "count", 42);
runtime.set("Main", "visible", true);
```

### `get` — read a Slint property

```rust
pub fn get(&self, surface: &str, prop: &str) -> Option<slint_interpreter::Value>
```

Returns `None` if the surface or property does not exist.

```rust
if let Some(val) = runtime.get("Main", "title") {
    println!("title = {val:?}");
}
```

### `on_ready` — one-shot setup callback

Fires once just before the event loop starts. Receives `&mut HeliumRuntime`,
so you can call `set`/`get` to initialise properties before the first frame.

```rust
runtime.on_ready(|rt| {
    rt.set("Main", "initialized", true);
});
```

Use this instead of setting properties on the builder before `.run()` — the
surface components may not exist yet at build time.

### `on_tick` — recurring timer

```rust
pub fn on_tick(
    &mut self,
    interval: Duration,
    cb: impl FnMut(&mut TickContext) + 'static,
) -> Result<(), HeliumError>
```

Registers a repeating timer. The callback receives a `TickContext` with
`set` and `get` for modifying surface properties.

```rust
runtime.on_tick(Duration::from_millis(1000), |ctx| {
    ctx.set("Main", "time", chrono::Local::now().format("%H:%M").to_string());
})?;
```

### `on_ipc` — file descriptor event source

```rust
pub fn on_ipc(
    &mut self,
    fd: impl AsFd + 'static,
    cb: impl FnMut(&mut IpcContext) + 'static,
) -> Result<(), HeliumError>
```

Watches a file descriptor for readability and fires the callback when
data arrives. Use this to wire in compositor sockets, D-Bus connections,
or any other fd-based IPC without touching the calloop event loop directly.

```rust
let stream = UnixStream::connect("/tmp/my-ipc.sock")?;
runtime.on_ipc(stream, |ctx| {
    ctx.set("Main", "ipc_ready", true);
})?;
```

### `attach_compositor` / `compositors` — persistent compositor handle

```rust
pub fn attach_compositor(&mut self, compositor: Box<dyn Compositor>) -> &mut Self
pub fn compositors(&self) -> &[Box<dyn Compositor>]
pub fn compositors_mut(&mut self) -> &mut [Box<dyn Compositor>]
```

Attach a compositor handle that stays alive for the runtime's lifetime.
Access it during `on_ready` to set up initial state:

```rust
let compositor = helium::compositors::detect()?;
runtime.attach_compositor(compositor)
    .on_ready(|rt| {
        let ws = rt.compositors()[0].workspaces();
        rt.set("Main", "workspace_count", ws.len() as i32);
    });
```

### `on_compositor_event` — poll a compositor on a timer

```rust
pub fn on_compositor_event(
    &mut self,
    compositor: Box<dyn Compositor>,
    interval: Duration,
    cb: impl FnMut(&mut IpcContext, &mut dyn Compositor) + 'static,
) -> Result<(), HeliumError>
```

Takes ownership of a compositor and polls it on a timer. The callback
receives an `IpcContext` for setting properties and a mutable compositor
reference for querying state.

```rust
runtime.on_compositor_event(compositor, Duration::from_millis(200), |ctx, comp| {
    let ws = comp.active_workspace();
    if let Some(ws) = ws {
        ctx.set("Main", "active_ws", ws.id as i32);
    }
})?;
```

### `into_inner` — raw access

```rust
pub fn into_inner(self) -> Shell
```

Consumes the runtime and returns the underlying `layer_shika::Shell` when
you need something the wrapper doesn't expose.

---

## TickContext and IpcContext

Two lightweight handles that wrap `&mut AppState` and expose `set` and `get`
for surface properties. They are structurally identical but semantically
distinct:

| Handle | When you get one |
|--------|-----------------|
| `TickContext` | Inside `on_tick` callbacks (timer-based) |
| `IpcContext` | Inside `on_ipc` and `on_compositor_event` callbacks (external events) |

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

---

## IntoSlintValue

Trait for automatic conversion of Rust values into `slint_interpreter::Value`.
Implemented for all the usual suspects:

| Rust type | Slint variant |
|-----------|--------------|
| `f64`, `f32`, `u8`–`u64`, `i32`, `i64` | `Value::Number(f64)` |
| `bool` | `Value::Bool(bool)` |
| `String`, `&str` | `Value::String(SharedString)` |
| `slint_interpreter::Value` | identity (pass through) |

If the type you need isn't listed, implement it yourself — it is a single
method returning a `Value`.

---

## Raw access

The full `layer-shika` API lives at `helium::raw`:

```rust
use helium::raw::AnchorEdges;

// Direct access when Helium's wrapper is not enough
let edges = AnchorEdges::top_bar();
```

---

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

No serde, no loading — just a struct. Good for lightweight data types.

### `helium_model!`

Generate a Slint-compatible model wrapper around `Vec<T>`.

```rust
helium_model! { MyModel(String) }

let model = MyModel::from_vec(vec!["a".into(), "b".into()]);
let value: slint_interpreter::Value = model.into();
```

The generated type wraps `slint::VecModel<T>` and exposes `push`, `clear`,
`from_vec`, `set_vec`, and `row_count`. It also implements
`Into<slint_interpreter::Value>` via `ModelRc<T>` conversion.

---

## Prelude

For convenience, `helium::prelude::*` re-exports the most common types:

```rust
use helium::prelude::*;

// Now in scope:
//   Helium, HeliumRuntime, IntoSlintValue,
//   IpcContext, TickContext, MonitorPolicy,
//   AnchorEdge
```

# Adapters

Adapters are the bridge between your config file and what appears on screen.
Each adapter reads some config fields and writes to surface properties —
periodically on a tick, or in response to a shell event.

---

## The Adapter trait

```rust
pub trait Adapter: Send {
    fn init(&mut self, ctx: &mut AdapterCtx) {}
    fn tick(&mut self, ctx: &mut AdapterCtx) {}
    fn on_event(&mut self, event: &ShellEvent, ctx: &mut AdapterCtx) {}
}
```

| Method | When it is called |
|--------|-------------------|
| `init` | Once, when the adapter is registered |
| `tick` | Every frame (or tick interval) |
| `on_event` | When a shell event fires — workspace change, window focus, etc. |

All three methods are optional (default no-ops). Only override what you
actually need.

---

## AdapterCtx

```rust
ctx.set("property_name", "value");
let val: Option<String> = ctx.get("property_name");
```

`set` writes a string value to a named property on the surface. `get`
reads it back (returns `None` if the property has not been set yet).

In a real integration these map to Slint properties via the component
instance. The current implementation uses an in-memory hashmap — the
interface is the same regardless of the backing store.

---

## ShellEvent

```rust
#[derive(Debug, Clone)]
pub enum ShellEvent {
    Tick,
    WorkspaceChange,
    WindowFocus,
}
```

---

## Built-in adapters

### ClockAdapter

Sets `"clock_text"` to the current time, formatted with a strftime string.

```rust
let adapter = ClockAdapter {
    format: "%H:%M".into(),
    interval_ms: 1000,
};
```

Configuration fields:

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `format` | `String` | `"%H:%M"` | strftime format string |
| `interval_ms` | `u64` | `1000` | How often to update (milliseconds) |

### WorkspacesAdapter

Sets `"workspaces"` (JSON array of workspace numbers) and
`"active_workspace"` (current workspace index).

```rust
let adapter = WorkspacesAdapter { max: 5 };
```

Configuration fields:

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `max` | `u8` | (required) | Maximum number of workspaces to track |

---

## Registration

Use the `adapters!` macro for a concise registration:

```rust
use helium::adapters;

let mut registry = adapters! {
    "clock" => ClockAdapter { format: "%H:%M".into(), ..Default::default() },
    "workspaces" => WorkspacesAdapter { max: 9 },
};
```

Or build an `AdapterRegistry` manually:

```rust
use helium::adapters::{AdapterRegistry, ClockAdapter};

let mut registry = AdapterRegistry::new(vec![
    ("clock", Box::new(ClockAdapter::default())),
]);
```

The `AdapterRegistry` can then be wired to a tick source or event source
in your application loop.

---

## Writing a custom adapter

```rust
use helium::adapters::{Adapter, AdapterCtx, ShellEvent};

struct WeatherAdapter {
    api_key: String,
}

impl Adapter for WeatherAdapter {
    fn tick(&mut self, ctx: &mut AdapterCtx) {
        // Fetch weather data, set surface properties
        ctx.set("weather_temp", "22°C");
        ctx.set("weather_icon", "☀️");
    }

    fn on_event(&mut self, event: &ShellEvent, _ctx: &mut AdapterCtx) {
        if matches!(event, ShellEvent::WorkspaceChange) {
            // Refresh weather when the user switches workspaces
        }
    }
}
```

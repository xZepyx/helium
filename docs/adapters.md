# Adapters

Adapters are the bridge between your config file and what appears on screen.
Each adapter reads some config fields and writes to surface properties —
periodically on a tick, or in response to a shell event.

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
| `on_event` | When a shell event fires |

All three methods are optional (default no-ops).

## AdapterCtx

```rust
ctx.set("property_name", "value");
let val: Option<String> = ctx.get("property_name");
```

`set` writes a string value to a named property. `get` reads it back.
The current implementation uses an in-memory hashmap as a stand-in
until the Slint integration is wired.

## ShellEvent

```rust
#[derive(Debug, Clone)]
pub enum ShellEvent {
    Tick,
    WorkspaceChange,
    WindowFocus,
}
```

## Built-in adapters

### ClockAdapter

Sets `"clock_text"` to the current time, formatted with a strftime string.

```rust
let adapter = ClockAdapter {
    format: "%H:%M".into(),
    interval_ms: 1000,
};
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `format` | `String` | `"%H:%M"` | strftime format string |
| `interval_ms` | `u64` | `1000` | How often to update (milliseconds) |

### WorkspacesAdapter

Sets `"workspaces"` (JSON array) and `"active_workspace"` (current index).

```rust
let adapter = WorkspacesAdapter { max: 5 };
```

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `max` | `u8` | `9` | Maximum number of workspaces to track |

## Registration

Use the `adapters!` macro for a concise registration:

```rust
use helium_wsl::adapters;

let mut registry = adapters! {
    "clock" => ClockAdapter { format: "%H:%M".into(), ..Default::default() },
    "workspaces" => WorkspacesAdapter { max: 9 },
};
```

Or build an `AdapterRegistry` manually:

```rust
use helium_wsl::adapters::{AdapterRegistry, ClockAdapter};

let mut registry = AdapterRegistry::new(vec![
    ("clock", Box::new(ClockAdapter::default())),
]);
```

The `AdapterRegistry` exposes `init_all`, `tick_all`, `event_all`, and
`names`.

## Writing a custom adapter

```rust
use helium_wsl::adapters::{Adapter, AdapterCtx, ShellEvent};

struct WeatherAdapter {
    api_key: String,
}

impl Adapter for WeatherAdapter {
    fn tick(&mut self, ctx: &mut AdapterCtx) {
        ctx.set("weather_temp", "22°C");
        ctx.set("weather_icon", "clear");
    }

    fn on_event(&mut self, event: &ShellEvent, _ctx: &mut AdapterCtx) {
        if matches!(event, ShellEvent::WorkspaceChange) {
            // refresh weather
        }
    }
}
```

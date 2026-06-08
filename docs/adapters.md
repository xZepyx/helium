# Adapters

Adapters wire config modules to surface properties. They are the bridge between
your config file and what appears on screen.

## The Adapter trait

```rust
pub trait Adapter: Send {
    fn init(&mut self, ctx: &mut AdapterCtx) {}
    fn tick(&mut self, ctx: &mut AdapterCtx) {}
    fn on_event(&mut self, event: &ShellEvent, ctx: &mut AdapterCtx) {}
}
```

- `init` — called once when the adapter is registered
- `tick` — called every frame (or tick interval)
- `on_event` — called on workspace changes, window focus, etc.

## AdapterCtx

`AdapterCtx` provides `set` and `get` for surface properties:

```rust
ctx.set("clock_text", "14:30");
let val: Option<String> = ctx.get("clock_text");
```

In a full implementation, `set` writes to a Slint property on the surface.
Right now it operates on an in-memory property bag.

## Built-in adapters

### ClockAdapter

Sets the `"clock_text"` property to the current time.

```rust
let adapter = ClockAdapter {
    format: "%H:%M".into(),
    interval_ms: 1000,
};
```

### WorkspacesAdapter

Sets `"workspaces"` (JSON array of workspace numbers) and `"active_workspace"`
(current workspace).

```rust
let adapter = WorkspacesAdapter { max: 5 };
```

## Registration

Use the `adapters!` macro:

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

## Writing a custom adapter

```rust
use helium::adapters::{Adapter, AdapterCtx, ShellEvent};

struct WeatherAdapter {
    api_key: String,
}

impl Adapter for WeatherAdapter {
    fn tick(&mut self, ctx: &mut AdapterCtx) {
        // Fetch weather, set properties
        ctx.set("weather_temp", "22°C");
    }

    fn on_event(&mut self, event: &ShellEvent, _ctx: &mut AdapterCtx) {
        if matches!(event, ShellEvent::Tick) {
            // Periodic refresh
        }
    }
}
```

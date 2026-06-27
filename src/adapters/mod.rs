mod clock;
mod workspaces;

pub use clock::ClockAdapter;
pub use workspaces::WorkspacesAdapter;

use std::collections::HashMap;
use std::sync::{Arc, Mutex};

/// Shell-level event that adapters can react to.
#[derive(Debug, Clone)]
pub enum ShellEvent {
    Tick,
    WorkspaceChange,
    WindowFocus,
}

/// A context handle provided to adapter methods.
///
/// Adapters use this to read and write surface properties. In a real
/// integration these would bridge to Slint properties; for now they
/// operate on an in-memory property bag.
pub struct AdapterCtx {
    properties: Arc<Mutex<HashMap<String, String>>>,
}

impl AdapterCtx {
    pub fn _new() -> Self {
        AdapterCtx {
            properties: Arc::new(Mutex::new(HashMap::new())),
        }
    }

    /// Set a surface property to a string value.
    pub fn set(&self, prop: &str, val: impl ToString) {
        if let Ok(mut map) = self.properties.lock() {
            map.insert(prop.to_string(), val.to_string());
        }
    }

    /// Get a surface property value.
    pub fn get(&self, prop: &str) -> Option<String> {
        self.properties.lock().ok().and_then(|map| map.get(prop).cloned())
    }

    /// Drain all accumulated property changes.
    ///
    /// Call this after `tick_all` / `event_all` to collect the properties
    /// that adapters set and apply them to actual surface components.
    pub fn take_changes(&self) -> Vec<(String, String)> {
        self.properties
            .lock()
            .ok()
            .map(|mut map| map.drain().collect())
            .unwrap_or_default()
    }
}

/// An adapter wires a config module to shell surface properties.
///
/// Adapters receive ticks, events, and can set properties on the surface
/// through the provided [`AdapterCtx`].
pub trait Adapter: Send {
    /// Called once when the adapter is registered.
    fn init(&mut self, _ctx: &mut AdapterCtx) {}
    /// Called on every tick (frame).
    fn tick(&mut self, _ctx: &mut AdapterCtx) {}
    /// Called when a shell event occurs.
    fn on_event(&mut self, _event: &ShellEvent, _ctx: &mut AdapterCtx) {}
}

/// Register a set of named adapters.
///
/// # Example
///
/// ```ignore
/// adapters! {
///     "clock" => ClockAdapter { format: "%H:%M".into(), ..Default::default() },
///     "workspaces" => WorkspacesAdapter { max: 9 },
/// }
/// ```
#[macro_export]
macro_rules! adapters {
    ($($key:expr => $adapter:expr),* $(,)?) => {
        {
            let mut registry: Vec<(&str, Box<dyn $crate::adapters::Adapter>)> = Vec::new();
            $(
                registry.push(($key, Box::new($adapter) as Box<dyn $crate::adapters::Adapter>));
            )*
            registry
        }
    };
}

/// A registry of named adapters.
pub struct AdapterRegistry {
    adapters: Vec<(String, Box<dyn Adapter>)>,
}

impl AdapterRegistry {
    /// Create a registry from a list of (name, adapter) pairs.
    pub fn new(pairs: Vec<(&str, Box<dyn Adapter>)>) -> Self {
        AdapterRegistry {
            adapters: pairs
                .into_iter()
                .map(|(name, adapter)| (name.to_string(), adapter))
                .collect(),
        }
    }

    /// Initialize all adapters.
    pub fn init_all(&mut self, ctx: &mut AdapterCtx) {
        for (_, adapter) in &mut self.adapters {
            adapter.init(ctx);
        }
    }

    /// Tick all adapters.
    pub fn tick_all(&mut self, ctx: &mut AdapterCtx) {
        for (_, adapter) in &mut self.adapters {
            adapter.tick(ctx);
        }
    }

    /// Send an event to all adapters.
    pub fn event_all(&mut self, event: &ShellEvent, ctx: &mut AdapterCtx) {
        for (_, adapter) in &mut self.adapters {
            adapter.on_event(event, ctx);
        }
    }

    /// Iterate over adapter names.
    pub fn names(&self) -> impl Iterator<Item = &str> {
        self.adapters.iter().map(|(n, _)| n.as_str())
    }
}

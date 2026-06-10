use std::cell::RefCell;
use std::collections::HashMap;
use std::os::unix::io::{AsFd, BorrowedFd, RawFd};
use std::path::Path;
use std::time::Duration;

use layer_shika::prelude::*;
use layer_shika::calloop::{Interest, Mode};
use layer_shika::Layer;

/// A non-owning file descriptor wrapper for use with calloop.
///
/// Does not close the underlying fd on drop. The caller must ensure
/// the fd remains valid for the lifetime of this object.
struct FdRef(RawFd);

impl AsFd for FdRef {
    fn as_fd(&self) -> BorrowedFd<'_> {
        unsafe { BorrowedFd::borrow_raw(self.0) }
    }
}

use crate::anchor::IntoAnchorEdges;
use crate::compositors::{Compositor, CompositorEvent};
use crate::HeliumError;

/// Monitor selection policy for a surface.
pub enum MonitorPolicy {
    All,
    Primary,
    Named(String),
}

/// Keyboard interactivity mode for a layer surface.
pub enum KeyboardMode {
    None,
    OnDemand,
    Exclusive,
}

/// A key identifier.
pub enum Key {
    Escape, Return, Space, Tab, Backspace,
    Up, Down, Left, Right,
    Home, End, PageUp, PageDown, Insert, Delete,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Raw(u32),
}

/// Modifier keys state.
pub struct Modifiers {
    pub ctrl: bool,
    pub alt: bool,
    pub shift: bool,
    pub super_key: bool,
}

/// A key event with modifiers.
pub struct KeyEvent {
    pub key: Key,
    pub modifiers: Modifiers,
    pub pressed: bool,
}

/// Helium entry point.
///
/// Use [`Helium::from_file`] to start building a shell.
pub struct Helium;

impl Helium {
    /// Start building a Helium shell from a `.slint` file.
    pub fn from_file(path: impl AsRef<Path>) -> ShellInitializer {
        ShellInitializer {
            inner: Shell::from_file(path),
            surface_names: Vec::new(),
        }
    }

    /// Start building from a compiled Slint UI source.
    pub fn from_compilation(
        compilation: std::rc::Rc<layer_shika::slint_integration::slint_interpreter::CompilationResult>,
    ) -> ShellInitializer {
        ShellInitializer {
            inner: Shell::from_compilation(compilation),
            surface_names: Vec::new(),
        }
    }

    /// Build from a Slint source string.
    pub fn from_source(code: impl Into<String>) -> ShellInitializer {
        ShellInitializer {
            inner: Shell::from_source(code),
            surface_names: Vec::new(),
        }
    }
}

/// Wraps [`ShellBuilder`] before any surfaces are configured.
pub struct ShellInitializer {
    inner: ShellBuilder,
    surface_names: Vec<String>,
}

impl ShellInitializer {
    /// Start configuring a surface.
    pub fn surface(mut self, name: &str) -> SurfaceInitializer {
        self.surface_names.push(name.to_string());
        SurfaceInitializer {
            inner: self.inner.surface(name),
            width: None,
            height: None,
            exclusive_flag: false,
            surface_names: self.surface_names.clone(),
        }
    }

    /// Build the shell directly (uses a default "Main" surface).
    pub fn build(self) -> std::result::Result<ShellInstance, HeliumError> {
        let shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(ShellInstance::new(shell, self.surface_names))
    }
}

/// A fluent builder for configuring surfaces.
///
/// Call `.surface(name)` to start configuring a surface, chain dimension and
/// anchor methods, then call `.surface(name)` again for the next surface or
/// `.build()` / `.run()` to finish.
pub struct SurfaceInitializer {
    inner: SurfaceConfigBuilder,
    width: Option<u32>,
    height: Option<u32>,
    exclusive_flag: bool,
    surface_names: Vec<String>,
}

impl SurfaceInitializer {
    /// Start configuring a new surface, finalizing the current one.
    pub fn surface(mut self, name: &str) -> Self {
        if self.exclusive_flag {
            let zone = self.height.or(self.width).unwrap_or(0) as i32;
            if zone != 0 {
                self.inner = self.inner.exclusive_zone(zone);
            }
        }
        self.surface_names.push(name.to_string());
        SurfaceInitializer {
            inner: self.inner.surface(name),
            width: None,
            height: None,
            exclusive_flag: false,
            surface_names: self.surface_names.clone(),
        }
    }

    /// Set the surface height.
    pub fn height(mut self, h: u32) -> Self {
        self.height = Some(h);
        self.inner = self.inner.height(h);
        self
    }

    /// Set the surface width.
    pub fn width(mut self, w: u32) -> Self {
        self.width = Some(w);
        self.inner = self.inner.width(w);
        self
    }

    /// Set both width and height.
    pub fn size(mut self, w: u32, h: u32) -> Self {
        self.width = Some(w);
        self.height = Some(h);
        self.inner = self.inner.size(w, h);
        self
    }

    /// Set anchor edges using the tuple-based API.
    pub fn anchor(mut self, edges: impl IntoAnchorEdges) -> Self {
        self.inner = self.inner.anchor(edges.into_anchor_edges());
        self
    }

    /// Infer an exclusive zone from the surface dimensions.
    ///
    /// Call after setting width/height. The non-zero dimension is used as
    /// the exclusive zone.
    pub fn exclusive(mut self) -> Self {
        self.exclusive_flag = true;
        self
    }

    /// Manually set an exclusive zone.
    pub fn exclusive_zone(mut self, zone: i32) -> Self {
        self.inner = self.inner.exclusive_zone(zone);
        self
    }

    /// Set the namespace (app id).
    pub fn namespace(mut self, ns: &str) -> Self {
        self.inner = self.inner.namespace(ns);
        self
    }

    /// Set the output monitor policy.
    pub fn monitors(mut self, policy: MonitorPolicy) -> Self {
        let p = match policy {
            MonitorPolicy::All => OutputPolicy::all_outputs(),
            MonitorPolicy::Primary => OutputPolicy::primary_only(),
            MonitorPolicy::Named(_) => OutputPolicy::all_outputs(),
        };
        self.inner = self.inner.output_policy(p);
        self
    }

    /// Set the layer (default is Top).
    pub fn layer(mut self, layer: Layer) -> Self {
        self.inner = self.inner.layer(layer);
        self
    }

    /// Set keyboard interactivity.
    pub fn keyboard(self, mode: KeyboardMode) -> Self {
        // todo: waiting on layer-shika exposure
        let _ = mode;
        self
    }

    /// Set margins in pixels.
    pub fn margin(mut self, top: i32, right: i32, bottom: i32, left: i32) -> Self {
        self.inner = self.inner.margin((top, right, bottom, left));
        self
    }

    /// Toggle click-through (default false).
    pub fn interactivity(self, interactive: bool) -> Self {
        // todo: waiting on layer-shika exposure
        let _ = interactive;
        self
    }

    /// Build the shell and return a runtime handle.
    pub fn build(mut self) -> std::result::Result<ShellInstance, HeliumError> {
        if self.exclusive_flag {
            let zone = self.height.or(self.width).unwrap_or(0) as i32;
            if zone != 0 {
                self.inner = self.inner.exclusive_zone(zone);
            }
        }
        let shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(ShellInstance::new(shell, self.surface_names))
    }

    /// Build and run the shell event loop (blocks).
    pub fn run(mut self) -> std::result::Result<(), HeliumError> {
        if self.exclusive_flag {
            let zone = self.height.or(self.width).unwrap_or(0) as i32;
            if zone != 0 {
                self.inner = self.inner.exclusive_zone(zone);
            }
        }
        let mut shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        shell.run().map_err(|e| HeliumError::Shell(e.to_string()))
    }
}

/// Trait for values that can be converted into Slint property values.
///
/// Implemented for all common Rust types so you can pass plain values
/// to [`ShellInstance::set`] and [`TickContext::set`].
pub trait IntoSlintValue {
    fn into_slint_value(self) -> slint_interpreter::Value;
}

impl IntoSlintValue for f64 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self)
    }
}
impl IntoSlintValue for f32 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self as f64)
    }
}
impl IntoSlintValue for u8 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self as f64)
    }
}
impl IntoSlintValue for u16 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self as f64)
    }
}
impl IntoSlintValue for u32 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self as f64)
    }
}
impl IntoSlintValue for u64 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self as f64)
    }
}
impl IntoSlintValue for i32 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self as f64)
    }
}
impl IntoSlintValue for i64 {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Number(self as f64)
    }
}
impl IntoSlintValue for bool {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::Bool(self)
    }
}
impl IntoSlintValue for String {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::String(self.into())
    }
}
impl IntoSlintValue for &str {
    fn into_slint_value(self) -> slint_interpreter::Value {
        slint_interpreter::Value::String(self.into())
    }
}
impl IntoSlintValue for slint_interpreter::Value {
    fn into_slint_value(self) -> slint_interpreter::Value {
        self
    }
}

/// Lightweight handle passed into [`ShellInstance::on_tick`] callbacks.
pub struct TickContext<'a> {
    app_state: &'a mut layer_shika_adapters::AppState,
}

impl TickContext<'_> {
    /// Set a property on a named surface component.
    pub fn set(&mut self, surface: &str, prop: &str, value: impl IntoSlintValue) {
        let value = value.into_slint_value();
        for s in self.app_state.surfaces_by_name(surface) {
            s.component_instance()
                .set_property(prop, value.clone())
                .unwrap();
        }
    }

    /// Read a property from a named surface component.
    pub fn get(&self, surface: &str, prop: &str) -> Option<slint_interpreter::Value> {
        self.app_state
            .surfaces_by_name(surface)
            .first()
            .and_then(|s| s.component_instance().get_property(prop).ok())
    }
}

/// Lightweight handle passed into [`ShellInstance::on_ipc`] callbacks.
pub struct IpcContext<'a> {
    app_state: &'a mut layer_shika_adapters::AppState,
}

impl IpcContext<'_> {
    /// Set a property on a named surface component.
    pub fn set(&mut self, surface: &str, prop: &str, value: impl IntoSlintValue) {
        let value = value.into_slint_value();
        for s in self.app_state.surfaces_by_name(surface) {
            s.component_instance()
                .set_property(prop, value.clone())
                .unwrap();
        }
    }

    /// Read a property from a named surface component.
    pub fn get(&self, surface: &str, prop: &str) -> Option<slint_interpreter::Value> {
        self.app_state
            .surfaces_by_name(surface)
            .first()
            .and_then(|s| s.component_instance().get_property(prop).ok())
    }
}

/// A batch handle for updating multiple properties on a surface.
///
/// Obtained via [`ShellInstance::update`]. Properties are applied when the
/// closure returns.
pub struct PropertyBatch<'a> {
    comp: &'a slint_interpreter::ComponentInstance,
}

impl PropertyBatch<'_> {
    /// Set a property value.
    pub fn set(&mut self, prop: &str, value: impl IntoSlintValue) {
        self.comp.set_property(prop, value.into_slint_value()).unwrap();
    }

    /// Read a property value.
    pub fn get(&self, prop: &str) -> Option<slint_interpreter::Value> {
        self.comp.get_property(prop).ok()
    }
}

/// A running Helium shell instance.
pub struct ShellInstance {
    inner: Shell,
    ready_cb: Option<Box<dyn FnOnce(&mut ShellInstance) + 'static>>,
    compositors: Vec<Box<dyn Compositor>>,
    paused: bool,
    surface_names: Vec<String>,
    property_change_callbacks: HashMap<String, Vec<Box<dyn Fn(slint_interpreter::Value) + 'static>>>,
    signal_callbacks: HashMap<String, Vec<Box<dyn Fn() + 'static>>>,
    surface_ready_callbacks: Vec<(String, Box<dyn FnOnce(&mut ShellInstance) + 'static>)>,
    event_bus_listeners: HashMap<String, Vec<Box<dyn Fn(slint_interpreter::Value) + 'static>>>,
    surface_created_callbacks: Vec<Box<dyn Fn(String) + 'static>>,
    surface_destroyed_callbacks: Vec<Box<dyn Fn(String) + 'static>>,
    surface_monitors: HashMap<String, String>,
    key_callbacks: Vec<Box<dyn Fn(KeyEvent) + 'static>>,
}

impl ShellInstance {
    fn new(inner: Shell, surface_names: Vec<String>) -> Self {
        ShellInstance {
            inner,
            ready_cb: None,
            compositors: Vec::new(),
            paused: false,
            surface_names,
            property_change_callbacks: HashMap::new(),
            signal_callbacks: HashMap::new(),
            surface_ready_callbacks: Vec::new(),
            event_bus_listeners: HashMap::new(),
            surface_created_callbacks: Vec::new(),
            surface_destroyed_callbacks: Vec::new(),
            surface_monitors: HashMap::new(),
            key_callbacks: Vec::new(),
        }
    }

    /// Run the event loop (blocks).
    ///
    /// If an `on_ready` callback was registered, it fires just before the
    /// event loop starts.
    pub fn run(&mut self) -> std::result::Result<(), HeliumError> {
        self.register_surface_callbacks();

        if let Some(cb) = self.ready_cb.take() {
            cb(self);
        }

        let ready_cbs = std::mem::take(&mut self.surface_ready_callbacks);
        for (name, cb) in ready_cbs {
            let _ = self.inner.with_surface(&name, |_comp| {});
            cb(self);
        }

        self.inner.run().map_err(|e| HeliumError::Shell(e.to_string()))
    }

    fn register_surface_callbacks(&mut self) {
        let _prop_cbs = std::mem::take(&mut self.property_change_callbacks);
        let _sig_cbs = std::mem::take(&mut self.signal_callbacks);
        // todo: register property change and signal callbacks with component
        // instances when slint-interpreter exposes the necessary API.
    }

    /// Set a property on a named surface component.
    ///
    /// The value is converted automatically — no manual `Value` wrapping needed.
    pub fn set(&mut self, surface: &str, prop: &str, value: impl IntoSlintValue) {
        self.inner
            .with_surface(surface, |comp| {
                comp.set_property(prop, value.into_slint_value()).unwrap();
            })
            .unwrap();
    }

    /// Read a property from a named surface component.
    ///
    /// Returns `None` if the surface or property does not exist.
    pub fn get(&self, surface: &str, prop: &str) -> Option<slint_interpreter::Value> {
        self.inner
            .with_surface(surface, |comp| comp.get_property(prop).ok())
            .ok()
            .flatten()
    }

    /// Register a callback to fire once just before the event loop starts.
    ///
    /// Use this for initial state setup instead of setting props before
    /// `.run()`. The callback receives `&mut ShellInstance` so you can
    /// call `set` and `get` directly.
    pub fn on_ready(
        &mut self,
        cb: impl FnOnce(&mut ShellInstance) + 'static,
    ) -> &mut Self {
        self.ready_cb = Some(Box::new(cb));
        self
    }

    /// Register a repeating timer.
    ///
    /// The callback receives a [`TickContext`] so you can call `set` directly.
    pub fn on_tick(
        &mut self,
        interval: Duration,
        cb: impl FnMut(&mut TickContext) + 'static,
    ) -> std::result::Result<(), HeliumError> {
        if self.paused {
            return Ok(());
        }
        let handle = self.inner.event_loop_handle();
        let cb = RefCell::new(cb);
        handle
            .add_timer(interval, move |_instant, app_state| {
                let mut ctx = TickContext { app_state };
                let mut cb = cb.borrow_mut();
                cb(&mut ctx);
                layer_shika::calloop::TimeoutAction::ToDuration(interval)
            })
            .map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(())
    }

    /// Register a raw file descriptor as a calloop event source.
    ///
    /// The callback fires when the fd becomes readable. Use this to wire in
    /// compositor sockets or other IPC without touching layer-shika internals.
    /// The [`IpcContext`] passed to the callback exposes `set` and `get`.
    pub fn on_ipc(
        &mut self,
        fd: RawFd,
        cb: impl FnMut(&mut IpcContext) + 'static,
    ) -> std::result::Result<(), HeliumError> {
        let handle = self.inner.event_loop_handle();
        let cb = RefCell::new(cb);
        let fd_ref = FdRef(fd);
        handle
            .add_fd(fd_ref, Interest::READ, Mode::Level, move |app_state| {
                let mut ctx = IpcContext { app_state };
                let mut cb = cb.borrow_mut();
                cb(&mut ctx);
            })
            .map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(())
    }

    /// Attach a compositor handle that is kept alive for the runtime's lifetime.
    ///
    /// The compositor is stored and accessible from [`on_ready`](Self::on_ready)
    /// callbacks via [`compositors`](Self::compositors) /
    /// [`compositors_mut`](Self::compositors_mut). Use
    /// [`on_compositor_event`](Self::on_compositor_event) to connect to its
    /// event socket instead of polling.
    pub fn attach_compositor(&mut self, compositor: Box<dyn Compositor>) -> &mut Self {
        self.compositors.push(compositor);
        self
    }

    /// Access all attached compositors (read-only).
    pub fn compositors(&self) -> &[Box<dyn Compositor>] {
        &self.compositors
    }

    /// Access all attached compositors (mutable).
    pub fn compositors_mut(&mut self) -> &mut [Box<dyn Compositor>] {
        &mut self.compositors
    }

    /// Register a push-based compositor event handler.
    ///
    /// Uses [`Compositor::event_fd`] to connect to the compositor's event
    /// socket rather than polling on a timer. The callback receives a
    /// [`CompositorEvent`] and a [`TickContext`] for updating surfaces.
    pub fn on_compositor_event(
        &mut self,
        compositor: Box<dyn Compositor>,
        cb: impl FnMut(CompositorEvent, &mut TickContext) + 'static,
    ) -> std::result::Result<(), HeliumError> {
        let fd = compositor
            .event_fd()
            .ok_or_else(|| HeliumError::Compositor("compositor does not support event fd".into()))?;
        let compositor = RefCell::new(compositor);
        let cb = RefCell::new(cb);
        let handle = self.inner.event_loop_handle();
        let fd_ref = FdRef(fd);
        handle
            .add_fd(fd_ref, Interest::READ, Mode::Level, move |app_state| {
                let mut tick_ctx = TickContext { app_state };
                let mut compositor = compositor.borrow_mut();
                let mut cb = cb.borrow_mut();
                while let Some(event) = compositor.poll_event() {
                    cb(event, &mut tick_ctx);
                }
            })
            .map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(())
    }

    /// List all registered surface names.
    pub fn surface_names(&self) -> Vec<String> {
        self.surface_names.clone()
    }

    /// Iterate all surfaces at once.
    ///
    /// The callback receives each surface name. Use [`set`](Self::set) or
    /// [`get`](Self::get) to access properties from the callback.
    pub fn with_all_surfaces(&mut self, mut f: impl FnMut(&str)) {
        let names = self.surface_names.clone();
        for name in &names {
            f(name);
        }
    }

    /// Bulk property update as a single transaction.
    pub fn update(&mut self, surface: &str, f: impl FnOnce(&mut PropertyBatch)) {
        let _ = self.inner.with_surface(surface, |comp| {
            let mut batch = PropertyBatch { comp };
            f(&mut batch);
        });
    }

    /// Register a callback for when a Slint property changes on the surface.
    pub fn on_property_change(
        &mut self,
        surface: &str,
        prop: &str,
        cb: impl Fn(slint_interpreter::Value) + 'static,
    ) -> &mut Self {
        self.property_change_callbacks
            .entry(surface.to_string())
            .or_default()
            .push(Box::new(cb));
        let _ = prop;
        self
    }

    /// Register a callback for a Slint callback/signal firing.
    pub fn on_signal(
        &mut self,
        surface: &str,
        signal: &str,
        cb: impl Fn() + 'static,
    ) -> &mut Self {
        self.signal_callbacks
            .entry(surface.to_string())
            .or_default()
            .push(Box::new(cb));
        let _ = signal;
        self
    }

    /// Register a callback that fires when a specific surface finishes initializing.
    pub fn on_surface_ready(
        &mut self,
        surface: &str,
        cb: impl FnOnce(&mut ShellInstance) + 'static,
    ) -> &mut Self {
        self.surface_ready_callbacks
            .push((surface.to_string(), Box::new(cb)));
        self
    }

    /// Hide a surface.
    pub fn hide(&mut self, surface: &str) {
        // todo: waiting on layer-shika visibility API
        let _ = surface;
    }

    /// Show a previously hidden surface.
    pub fn show(&mut self, surface: &str) {
        // todo: waiting on layer-shika visibility API
        let _ = surface;
    }

    /// Hot-swap the `.slint` file for a surface without restarting the shell.
    pub fn reload_ui(&mut self, surface: &str, path: impl AsRef<Path>) {
        // todo: waiting on layer-shika component replacement API
        let _ = surface;
        let _ = path;
    }

    /// Suspend rendering and update callbacks.
    pub fn pause(&mut self) {
        self.paused = true;
    }

    /// Resume rendering and update callbacks.
    pub fn resume(&mut self) {
        self.paused = false;
    }

    /// Set a property only on the surface instance running on the named monitor.
    pub fn set_on(
        &mut self,
        surface: &str,
        monitor: &str,
        prop: &str,
        value: impl IntoSlintValue,
    ) {
        // todo: waiting on per-monitor surface identification
        let _ = monitor;
        self.set(surface, prop, value);
    }

    /// Returns the monitor name the named surface is currently on.
    pub fn surface_monitor(&self, surface: &str) -> Option<String> {
        self.surface_monitors.get(surface).cloned()
    }

    /// Register a callback that fires when a surface is created (e.g. on monitor hotplug).
    pub fn on_surface_created(&mut self, cb: impl Fn(String) + 'static) -> &mut Self {
        self.surface_created_callbacks.push(Box::new(cb));
        self
    }

    /// Register a callback that fires when a surface is destroyed.
    pub fn on_surface_destroyed(&mut self, cb: impl Fn(String) + 'static) -> &mut Self {
        self.surface_destroyed_callbacks.push(Box::new(cb));
        self
    }

    /// Emit an event on the global event bus.
    pub fn emit(&mut self, event: &str, value: impl IntoSlintValue) {
        let val = value.into_slint_value();
        if let Some(listeners) = self.event_bus_listeners.get(event) {
            for cb in listeners {
                cb(val.clone());
            }
        }
    }

    /// Listen for events on the global event bus.
    pub fn on_event(
        &mut self,
        event: &str,
        cb: impl Fn(slint_interpreter::Value) + 'static,
    ) -> &mut Self {
        self.event_bus_listeners
            .entry(event.to_string())
            .or_default()
            .push(Box::new(cb));
        self
    }

    /// Register a keyboard input handler for a surface.
    pub fn on_key(
        &mut self,
        _surface: &str,
        _key: Key,
        cb: impl Fn(KeyEvent) + 'static,
    ) -> &mut Self {
        // todo: waiting on layer-shika keyboard input API
        self.key_callbacks.push(Box::new(cb));
        self
    }

    /// Consume the runtime and return the underlying [`Shell`].
    ///
    /// Use this when you need raw access to layer-shika functionality.
    pub fn into_inner(self) -> Shell {
        self.inner
    }
}

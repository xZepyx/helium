use std::cell::RefCell;
use std::os::unix::io::AsFd;
use std::path::Path;
use std::time::Duration;

use layer_shika::prelude::*;
use layer_shika::calloop::{Interest, Mode};

use crate::anchor::IntoAnchorEdges;
use crate::compositors::Compositor;
use crate::HeliumError;

/// Monitor selection policy for a surface.
pub enum MonitorPolicy {
    All,
    Primary,
    Named(String),
}

/// Helium entry point.
///
/// Use [`Helium::from_file`] to start building a shell.
pub struct Helium;

impl Helium {
    /// Start building a Helium shell from a `.slint` file.
    pub fn from_file(path: impl AsRef<Path>) -> HeliumBuilder {
        HeliumBuilder {
            inner: Shell::from_file(path),
        }
    }

    /// Start building from a compiled Slint UI source.
    pub fn from_compilation(
        compilation: std::rc::Rc<layer_shika::slint_integration::slint_interpreter::CompilationResult>,
    ) -> HeliumBuilder {
        HeliumBuilder {
            inner: Shell::from_compilation(compilation),
        }
    }

    /// Build from a Slint source string.
    pub fn from_source(code: impl Into<String>) -> HeliumBuilder {
        HeliumBuilder {
            inner: Shell::from_source(code),
        }
    }
}

/// Wraps [`ShellBuilder`] before any surfaces are configured.
pub struct HeliumBuilder {
    inner: ShellBuilder,
}

impl HeliumBuilder {
    /// Start configuring a surface.
    pub fn surface(self, name: &str) -> HeliumSurfaceBuilder {
        HeliumSurfaceBuilder {
            inner: self.inner.surface(name),
            width: None,
            height: None,
            exclusive_flag: false,
        }
    }

    /// Build the shell directly (uses a default "Main" surface).
    pub fn build(self) -> std::result::Result<HeliumRuntime, HeliumError> {
        let shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(HeliumRuntime { inner: shell, ready_cb: None, compositors: Vec::new() })
    }
}

/// A fluent builder for configuring surfaces.
///
/// Call `.surface(name)` to start configuring a surface, chain dimension and
/// anchor methods, then call `.surface(name)` again for the next surface or
/// `.build()` / `.run()` to finish.
pub struct HeliumSurfaceBuilder {
    inner: SurfaceConfigBuilder,
    width: Option<u32>,
    height: Option<u32>,
    exclusive_flag: bool,
}

impl HeliumSurfaceBuilder {
    /// Start configuring a new surface, finalizing the current one.
    pub fn surface(mut self, name: &str) -> Self {
        if self.exclusive_flag {
            let zone = self.height.or(self.width).unwrap_or(0) as i32;
            if zone != 0 {
                self.inner = self.inner.exclusive_zone(zone);
            }
        }
        HeliumSurfaceBuilder {
            inner: self.inner.surface(name),
            width: None,
            height: None,
            exclusive_flag: false,
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

    /// Build the shell and return a runtime handle.
    pub fn build(mut self) -> std::result::Result<HeliumRuntime, HeliumError> {
        if self.exclusive_flag {
            let zone = self.height.or(self.width).unwrap_or(0) as i32;
            if zone != 0 {
                self.inner = self.inner.exclusive_zone(zone);
            }
        }
        let shell = self.inner.build().map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(HeliumRuntime { inner: shell, ready_cb: None, compositors: Vec::new() })
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

/// A running Helium shell instance.
pub struct HeliumRuntime {
    inner: Shell,
    ready_cb: Option<Box<dyn FnOnce(&mut HeliumRuntime) + 'static>>,
    compositors: Vec<Box<dyn Compositor>>,
}

/// Trait for values that can be converted into Slint property values.
///
/// Implemented for all common Rust types so you can pass plain values
/// to [`HeliumRuntime::set`] and [`TickContext::set`].
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

/// Lightweight handle passed into [`HeliumRuntime::on_tick`] callbacks.
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

/// Lightweight handle passed into [`HeliumRuntime::on_ipc`] callbacks.
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

impl HeliumRuntime {
    /// Run the event loop (blocks).
    ///
    /// If an `on_ready` callback was registered, it fires just before the
    /// event loop starts.
    pub fn run(&mut self) -> std::result::Result<(), HeliumError> {
        if let Some(cb) = self.ready_cb.take() {
            cb(self);
        }
        self.inner.run().map_err(|e| HeliumError::Shell(e.to_string()))
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
    /// `.run()`. The callback receives `&mut HeliumRuntime` so you can
    /// call `set` and `get` directly.
    pub fn on_ready(
        &mut self,
        cb: impl FnOnce(&mut HeliumRuntime) + 'static,
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
    ///
    /// The event loop takes ownership of the fd.
    pub fn on_ipc(
        &mut self,
        fd: impl AsFd + 'static,
        cb: impl FnMut(&mut IpcContext) + 'static,
    ) -> std::result::Result<(), HeliumError> {
        let handle = self.inner.event_loop_handle();
        let cb = RefCell::new(cb);
        handle
            .add_fd(fd, Interest::READ, Mode::Level, move |app_state| {
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
    /// [`on_compositor_event`](Self::on_compositor_event) to poll it on a timer.
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

    /// Register a recurring timer that polls a compositor.
    ///
    /// The callback receives an [`IpcContext`] for updating UI properties and
    /// a mutable reference to the compositor for querying state. The compositor
    /// is kept alive for the duration of the event loop.
    pub fn on_compositor_event(
        &mut self,
        compositor: Box<dyn Compositor>,
        interval: Duration,
        cb: impl FnMut(&mut IpcContext, &mut dyn Compositor) + 'static,
    ) -> std::result::Result<(), HeliumError> {
        let handle = self.inner.event_loop_handle();
        let compositor = RefCell::new(compositor);
        let cb = RefCell::new(cb);
        handle
            .add_timer(interval, move |_instant, app_state| {
                let mut ctx = IpcContext { app_state };
                let mut compositor = compositor.borrow_mut();
                let mut cb = cb.borrow_mut();
                cb(&mut ctx, &mut **compositor);
                layer_shika::calloop::TimeoutAction::ToDuration(interval)
            })
            .map_err(|e| HeliumError::Shell(e.to_string()))?;
        Ok(())
    }

    /// Consume the runtime and return the underlying [`Shell`].
    ///
    /// Use this when you need raw access to layer-shika functionality.
    pub fn into_inner(self) -> Shell {
        self.inner
    }
}

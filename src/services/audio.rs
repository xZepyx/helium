/// Current audio state.
#[derive(Debug, Clone)]
pub struct AudioState {
    pub volume: f32,
    pub muted: bool,
}

fn backend() -> Result<(), String> {
    Err("no audio backend available — enable the \"services\" feature or add pipewire manually".into())
}

/// Get the current system volume as a fraction 0.0–1.0.
pub fn volume() -> Result<f32, String> {
    backend()?;
    Ok(0.0)
}

/// Set the system volume.
pub fn set_volume(_v: f32) -> Result<(), String> {
    backend()?;
    Ok(())
}

/// Check whether the system audio is muted.
pub fn muted() -> Result<bool, String> {
    backend()?;
    Ok(false)
}

/// Toggle system audio mute.
pub fn toggle_mute() -> Result<(), String> {
    backend()?;
    Ok(())
}

/// Register a callback for audio state changes.
///
/// This requires a running event loop. The callback is invoked on the
/// audio backend's thread – you may need to dispatch to your UI thread.
///
/// # Note
///
/// PipeWire/D-Bus integration is not yet implemented when the
/// `service-audio` feature is enabled. This function returns an error
/// until a real backend is wired.
pub fn on_change(_cb: impl Fn(AudioState) + Send + 'static) -> Result<(), String> {
    backend()?;
    Err("audio on_change: PipeWire/D-Bus monitoring not yet implemented".into())
}

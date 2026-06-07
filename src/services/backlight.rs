use std::fs;
use std::path::PathBuf;

/// Backlight error.
#[derive(Debug, thiserror::Error)]
pub enum BacklightError {
    #[error("no backlight device found")]
    NoDevice,
    #[error("I/O error: {0}")]
    Io(#[from] std::io::Error),
    #[error("parse error: {0}")]
    Parse(String),
}

fn find_device() -> Result<PathBuf, BacklightError> {
    let sysfs = PathBuf::from("/sys/class/backlight");
    let mut entries = fs::read_dir(&sysfs)
        .map_err(|_| BacklightError::NoDevice)?;
    entries
        .next()
        .ok_or(BacklightError::NoDevice)?
        .map(|e| e.path())
        .map_err(|_| BacklightError::NoDevice)
}

fn read_u32(path: &std::path::Path, file: &str) -> Result<u32, BacklightError> {
    let s = fs::read_to_string(path.join(file))?;
    s.trim()
        .parse::<u32>()
        .map_err(|e| BacklightError::Parse(e.to_string()))
}

/// Get the current brightness as a normalized 0.0–1.0 fraction.
pub fn brightness() -> Result<f32, BacklightError> {
    let dev = find_device()?;
    let actual = read_u32(&dev, "actual_brightness")?;
    let max = read_u32(&dev, "max_brightness")?;
    if max == 0 {
        return Ok(0.0);
    }
    Ok(actual as f32 / max as f32)
}

/// Set the backlight brightness.
///
/// `v` is clamped to 0.0–1.0.
pub fn set_brightness(v: f32) -> Result<(), BacklightError> {
    let dev = find_device()?;
    let max = read_u32(&dev, "max_brightness")?;
    let clamped = v.clamp(0.0, 1.0);
    let value = (clamped * max as f32).round() as u32;
    fs::write(dev.join("brightness"), value.to_string())?;
    Ok(())
}

/// Register a callback for brightness changes.
///
/// Monitors the `actual_brightness` sysfs file with inotify. The callback
/// receives the new 0.0–1.0 brightness value.
pub fn on_change(_cb: impl Fn(f32) + Send + 'static) -> Result<(), BacklightError> {
    // Requires the `inotify` crate – currently stubbed.
    // Implementation: watch `/sys/class/backlight/<dev>/actual_brightness`
    // with inotify and call `cb` when the file changes.
    Err(BacklightError::Io(std::io::Error::new(
        std::io::ErrorKind::Unsupported,
        "inotify backlight monitoring not yet wired",
    )))
}

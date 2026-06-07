use chrono::Local;

/// Get the current local time.
pub fn now() -> chrono::DateTime<Local> {
    chrono::Local::now()
}

/// Format the current local time with a [`strftime`](chrono::format::strftime) string.
///
/// # Example
///
/// ```ignore
/// formatted("%H:%M") // → "14:30"
/// ```
pub fn formatted(fmt: &str) -> String {
    now().format(fmt).to_string()
}

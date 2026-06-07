use serde::Deserialize;

use super::{Adapter, AdapterCtx};

/// Clock adapter config.
#[derive(Debug, Clone, Deserialize)]
pub struct ClockAdapter {
    pub format: String,
    #[serde(default = "default_interval")]
    pub interval_ms: u64,
}

fn default_interval() -> u64 {
    1000
}

impl Default for ClockAdapter {
    fn default() -> Self {
        ClockAdapter {
            format: "%H:%M".into(),
            interval_ms: 1000,
        }
    }
}

impl Adapter for ClockAdapter {
    fn tick(&mut self, ctx: &mut AdapterCtx) {
        ctx.set("clock_text", crate::services::time::formatted(&self.format));
    }
}

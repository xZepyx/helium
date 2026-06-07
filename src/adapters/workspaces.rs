use serde::Deserialize;

use super::{Adapter, AdapterCtx, ShellEvent};

/// Workspaces adapter config.
#[derive(Debug, Clone, Deserialize)]
pub struct WorkspacesAdapter {
    #[serde(default = "default_max")]
    pub max: u8,
}

fn default_max() -> u8 {
    9
}

impl Default for WorkspacesAdapter {
    fn default() -> Self {
        WorkspacesAdapter { max: 9 }
    }
}

impl Adapter for WorkspacesAdapter {
    fn init(&mut self, ctx: &mut AdapterCtx) {
        // In a real implementation, query the compositor for initial workspace state
        ctx.set("workspaces", "[1, 2, 3, 4, 5, 6, 7, 8, 9]");
        ctx.set("active_workspace", "1");
    }

    fn on_event(&mut self, event: &ShellEvent, _ctx: &mut AdapterCtx) {
        if let ShellEvent::WorkspaceChange = event {
            // Update which workspace is active
            // ctx.set("active_workspace", compositor.active_workspace());
        }
    }
}

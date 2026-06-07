/// Connection type.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionType {
    Wifi,
    Ethernet,
    None,
}

/// Current network status.
#[derive(Debug, Clone)]
pub struct NetworkStatus {
    pub connected: bool,
    pub ssid: Option<String>,
    pub signal_strength: Option<u8>,
    pub connection_type: ConnectionType,
}

/// Get the current network status from NetworkManager over D-Bus.
pub fn status() -> Result<NetworkStatus, String> {
    Err("NetworkManager not available — enable the \"services\" feature".into())
}

/// Register a callback for network status changes.
pub fn on_change(_cb: impl Fn(NetworkStatus) + Send + 'static) -> Result<(), String> {
    Err("NetworkManager not available — enable the \"services\" feature".into())
}

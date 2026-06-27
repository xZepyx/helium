/// A Bluetooth device.
#[derive(Debug, Clone)]
pub struct BtDevice {
    pub name: String,
    pub address: String,
    pub connected: bool,
    pub battery: Option<u8>,
}

/// Overall Bluetooth state.
#[derive(Debug, Clone)]
pub struct BluetoothState {
    pub enabled: bool,
    pub devices: Vec<BtDevice>,
}

/// Check whether Bluetooth is enabled.
pub fn enabled() -> Result<bool, String> {
    Err("BlueZ not available — enable the \"services\" feature".into())
}

/// Get the list of known Bluetooth devices.
pub fn devices() -> Result<Vec<BtDevice>, String> {
    Err("BlueZ not available — enable the \"services\" feature".into())
}

/// Register a callback for Bluetooth state changes.
///
/// BlueZ/D-Bus monitoring is not yet implemented. Returns an error until
/// a real backend is wired.
pub fn on_change(_cb: impl Fn(BluetoothState) + Send + 'static) -> Result<(), String> {
    Err("Bluetooth on_change: BlueZ/D-Bus monitoring not yet implemented".into())
}

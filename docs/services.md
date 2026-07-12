# Services

Helium bundles free-function wrappers for common system services in
`helium_wsl::services`. Each module is self-contained — no global state,
connections are created on demand.

## Time

Always available. Uses `chrono` internally — no IPC, no sockets, just the
system clock.

```rust
use helium_wsl::services::time;

let now = time::now();
let formatted = time::formatted("%H:%M:%S");
```

## Backlight

Always available. Reads and writes `/sys/class/backlight/<device>/`.

```rust
use helium_wsl::services::backlight;

let b = backlight::brightness()?;
backlight::set_brightness(0.8)?;
```

`on_change` requires the `inotify` feature to monitor brightness changes
via inotify. Without it, the function returns an error at runtime.

## Audio

Requires `service-audio` feature. Currently stubbed — all functions
return errors at runtime.

```rust
use helium_wsl::services::audio;

let vol = audio::volume()?;
audio::set_volume(0.5)?;
audio::toggle_mute()?;
audio::on_change(|state| {
    println!("volume: {}, muted: {}", state.volume, state.muted);
})?;
```

## Bluetooth

Requires `service-bluetooth` feature (included in `services`). Uses
BlueZ D-Bus via zbus. Fully implemented.

```rust
use helium_wsl::services::bluetooth;

let state = bluetooth::state()?;
println!("{} is {}", state.name, if state.enabled { "on" } else { "off" });

let devices = bluetooth::devices()?;
for d in &devices {
    println!("{} ({}) rssi={:?}", d.name, d.address, d.rssi);
}

bluetooth::set_enabled(true)?;     // power on
bluetooth::set_enabled(false)?;    // power off
bluetooth::scan()?;                // scan for 5s, return found devices
bluetooth::start_discovery()?;     // start scanning
bluetooth::stop_discovery()?;      // stop scanning

bluetooth::connect("AA:BB:CC:DD:EE:FF")?;
bluetooth::disconnect("AA:BB:CC:DD:EE:FF")?;
bluetooth::pair("AA:BB:CC:DD:EE:FF")?;
bluetooth::unpair("AA:BB:CC:DD:EE:FF")?;
bluetooth::trust("AA:BB:CC:DD:EE:FF", true)?;
bluetooth::block("AA:BB:CC:DD:EE:FF", true)?;
```

### Data types

```rust
pub struct BluetoothState {
    pub enabled: bool,
    pub discovering: bool,
    pub discoverable: bool,
    pub pairable: bool,
    pub address: String,
    pub name: String,
    pub devices: Vec<BtDevice>,
    pub adapter_path: Option<String>,
}

pub struct BtDevice {
    pub name: String,
    pub address: String,
    pub connected: bool,
    pub paired: bool,
    pub battery: Option<u8>,
    pub rssi: Option<i16>,
    pub trusted: bool,
    pub blocked: bool,
    pub icon: Option<String>,
    pub path: String,
}
```

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `enabled` | `() -> Result<bool, String>` | Check if Bluetooth adapter is powered on |
| `devices` | `() -> Result<Vec<BtDevice>, String>` | List all known devices |
| `state` | `() -> Result<BluetoothState, String>` | Full adapter state including devices |
| `scan` | `() -> Result<Vec<BtDevice>, String>` | Start discovery for 5s, return visible devices |
| `start_discovery` | `() -> Result<(), String>` | Start continuous scanning |
| `stop_discovery` | `() -> Result<(), String>` | Stop continuous scanning |
| `set_enabled` | `(on: bool) -> Result<(), String>` | Power the adapter on/off |
| `set_discoverable` | `(on: bool) -> Result<(), String>` | Make adapter discoverable to other devices |
| `set_pairable` | `(on: bool) -> Result<(), String>` | Make adapter pairable |
| `connect` | `(address: &str) -> Result<(), String>` | Connect to a device by MAC address |
| `disconnect` | `(address: &str) -> Result<(), String>` | Disconnect from a device |
| `pair` | `(address: &str) -> Result<(), String>` | Pair with a device |
| `unpair` | `(address: &str) -> Result<(), String>` | Remove pairing for a device |
| `trust` | `(address: &str, trusted: bool) -> Result<(), String>` | Set device trust status |
| `block` | `(address: &str, blocked: bool) -> Result<(), String>` | Block or unblock a device |
| `on_change` | `(cb: impl Fn(BluetoothState) + Send + 'static) -> Result<(), String>` | Stubbed — not yet implemented |

`on_change` is currently stubbed.

## Network

Requires `service-network` feature (included in `services`). Uses
NetworkManager D-Bus via zbus. Fully implemented.

```rust
use helium_wsl::services::network;

let status = network::status()?;
println!("connected: {}, ssid: {:?}, ip: {:?}",
    status.connected, status.ssid, status.ip_address);

let aps = network::scan()?;        // request scan, wait 1.5s, return APs
for ap in &aps {
    println!("{} ({}%) freq={}", ap.ssid, ap.strength, ap.frequency);
}

network::connect("MyWiFi", Some("password"))?;
network::disconnect()?;

let saved = network::saved_connections()?;
network::forget("MyWiFi")?;
```

### Data types

```rust
pub struct NetworkStatus {
    pub connected: bool,
    pub ssid: Option<String>,
    pub signal_strength: Option<u8>,
    pub connection_type: ConnectionType,
    pub connectivity: Connectivity,
    pub ip_address: Option<String>,
}

pub enum ConnectionType {
    Wifi,
    Ethernet,
    None,
}

pub enum Connectivity {
    Unknown,
    NoConnection,
    Connecting,
    ConnectedLocal,
    ConnectedSite,
    ConnectedGlobal,
}

pub struct WifiAccessPoint {
    pub ssid: String,
    pub bssid: String,
    pub strength: u8,
    pub frequency: u32,
    pub protected: bool,
}

pub struct SavedConnection {
    pub id: String,
    pub uuid: String,
    pub ssid: Option<String>,
    pub path: String,
}
```

### Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `status` | `() -> Result<NetworkStatus, String>` | Get current network state |
| `scan` | `() -> Result<Vec<WifiAccessPoint>, String>` | Request scan and return visible APs |
| `connect` | `(ssid: &str, password: Option<&str>) -> Result<(), String>` | Connect to a Wi-Fi network |
| `disconnect` | `() -> Result<(), String>` | Disconnect all Wi-Fi devices |
| `saved_connections` | `() -> Result<Vec<SavedConnection>, String>` | List saved Wi-Fi connections |
| `forget` | `(ssid_or_uuid: &str) -> Result<(), String>` | Delete a saved connection |
| `on_change` | `(cb: impl Fn(NetworkStatus) + Send + 'static) -> Result<(), String>` | Stubbed — not yet implemented |

`on_change` is currently stubbed.

## Power

Requires `service-power` feature. Currently stubbed — all functions
return errors at runtime.

```rust
use helium_wsl::services::power;

for battery in power::batteries()? {
    println!("{:?}%", battery.percentage);
}
power::on_change(|batteries| {})?;
```

## Power Profiles

Requires `service-powerprofiles` feature. Currently stubbed — all
functions return errors at runtime.

```rust
use helium_wsl::services::powerprofiles;

let profile = powerprofiles::active()?;
powerprofiles::set(powerprofiles::Profile::PowerSaver)?;
```

## Feature flags reference

| Module | Feature | External dep | Status |
|--------|---------|-------------|--------|
| `time` | always | `chrono` | always available |
| `backlight` | always | (sysfs) | always available; `on_change` requires `inotify` feature |
| `audio` | `service-audio` | `zbus` | stubbed |
| `bluetooth` | `service-bluetooth` | `zbus` | fully implemented |
| `network` | `service-network` | `zbus` | fully implemented |
| `power` | `service-power` | `zbus` | stubbed |
| `powerprofiles` | `service-powerprofiles` | `zbus` | stubbed |

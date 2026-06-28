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

`on_change` is currently stubbed (requires `inotify`).

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
BlueZ D-Bus via zbus.

```rust
use helium_wsl::services::bluetooth;

let state = bluetooth::state()?;
println!("{} is {}", state.name, if state.enabled { "on" } else { "off" });

let devices = bluetooth::devices()?;
for d in &devices {
    println!("{} ({})", d.name, d.address);
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

`on_change` is currently stubbed.

## Network

Requires `service-network` feature (included in `services`). Uses
NetworkManager D-Bus via zbus.

```rust
use helium_wsl::services::network;

let status = network::status()?;
println!("connected: {}, ssid: {:?}, ip: {:?}",
    status.connected, status.ssid, status.ip_address);

let aps = network::scan()?;        // request scan, wait 1.5s, return APs
for ap in &aps {
    println!("{} ({}%)", ap.ssid, ap.strength);
}

network::connect("MyWiFi", Some("password"))?;
network::disconnect()?;

let saved = network::saved_connections()?;
network::forget("MyWiFi")?;
```

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

| Module | Feature | External dep |
|--------|---------|-------------|
| `time` | always | `chrono` |
| `backlight` | always | (sysfs, no dep) |
| `audio` | `service-audio` | `zbus` (stubbed) |
| `bluetooth` | `service-bluetooth` | `zbus` (bluez D-Bus) |
| `network` | `service-network` | `zbus` (NM D-Bus) |
| `power` | `service-power` | `zbus` (stubbed) |
| `powerprofiles` | `service-powerprofiles` | `zbus` (stubbed) |

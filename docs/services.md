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

Requires `service-bluetooth` feature. Currently stubbed — all functions
return errors at runtime.

```rust
use helium_wsl::services::bluetooth;

let enabled = bluetooth::enabled()?;
let devices = bluetooth::devices()?;
```

## Network

Requires `service-network` feature. Currently stubbed — all functions
return errors at runtime.

```rust
use helium_wsl::services::network;

let status = network::status()?;
println!("connected: {}, ssid: {:?}", status.connected, status.ssid);
```

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
| `bluetooth` | `service-bluetooth` | `zbus` (stubbed) |
| `network` | `service-network` | `zbus` (stubbed) |
| `power` | `service-power` | `zbus` (stubbed) |
| `powerprofiles` | `service-powerprofiles` | `zbus` (stubbed) |

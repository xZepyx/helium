# Services

Helium bundles free-function wrappers for common system services in
`helium::services`. Each module is self-contained — no global state,
connections are created on demand.

---

## Time

Always available. Uses `chrono` internally — no IPC, no sockets, just the
system clock.

```rust
use helium::services::time;

let now = time::now();
let formatted = time::formatted("%H:%M:%S");
```

## Backlight

Always available. Reads and writes `/sys/class/backlight/<device>/`.

```rust
use helium::services::backlight;

let b = backlight::brightness()?;
backlight::set_brightness(0.8)?;
```

---

## Audio

Requires `service-audio` feature. Backend: PipeWire / PulseAudio via D-Bus
(planned — currently stubbed).

```rust
use helium::services::audio;

let vol = audio::volume()?;
audio::set_volume(0.5)?;
audio::toggle_mute()?;
audio::on_change(|state| {
    println!("volume: {}, muted: {}", state.volume, state.muted);
})?;
```

## Bluetooth

Requires `service-bluetooth` feature. Backend: BlueZ via D-Bus.

```rust
use helium::services::bluetooth;

let enabled = bluetooth::enabled()?;
let devices = bluetooth::devices()?;
```

## Network

Requires `service-network` feature. Backend: NetworkManager via D-Bus.

```rust
use helium::services::network;

let status = network::status()?;
println!("connected: {}, ssid: {:?}", status.connected, status.ssid);
```

## Power

Requires `service-power` feature. Backend: UPower via D-Bus.

```rust
use helium::services::power;

for battery in power::batteries()? {
    println!("{:?}%", battery.percentage);
}
power::on_change(|batteries| {
    // react to changes
})?;
```

## Power Profiles

Requires `service-powerprofiles` feature. Backend: power-profiles-daemon
via D-Bus.

```rust
use helium::services::powerprofiles;

let profile = powerprofiles::active()?;
powerprofiles::set(powerprofiles::Profile::PowerSaver)?;
```

---

## Feature flags reference

| Module | Feature | External dep |
|--------|---------|-------------|
| `time` | always | `chrono` |
| `backlight` | always | (sysfs, no dep) |
| `audio` | `service-audio` | `zbus` |
| `bluetooth` | `service-bluetooth` | `zbus` |
| `network` | `service-network` | `zbus` |
| `power` | `service-power` | `zbus` |
| `powerprofiles` | `service-powerprofiles` | `zbus` |

All zbus-using modules share the same `zbus` dependency — enabling any one
of them compiles zbus once and shares it.

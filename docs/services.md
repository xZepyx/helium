# Services

Helium provides a set of system service wrappers in `helium::services`. Each
module exposes free functions — no global state, create connections on demand.

All services require the `services` feature (enabled by default).

## Audio

Backend: PipeWire (planned), PulseAudio (planned). Currently stubbed.

```rust
use helium::services::audio;

let vol = audio::volume()?;
audio::set_volume(0.5)?;
let muted = audio::muted()?;
audio::toggle_mute()?;
audio::on_change(|state| {
    println!("volume: {}, muted: {}", state.volume, state.muted);
})?;
```

## Time

Uses `chrono`, no external backend needed.

```rust
use helium::services::time;

let now = time::now();
let formatted = time::formatted("%H:%M:%S");
```

## Backlight

Reads/writes `/sys/class/backlight/<device>/`.

```rust
use helium::services::backlight;

let b = backlight::brightness()?;
backlight::set_brightness(0.8)?;
backlight::on_change(|new_val| {
    println!("brightness: {}", new_val);
})?;
```

## Power (UPower via D-Bus)

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

```rust
use helium::services::powerprofiles;

let profile = powerprofiles::active()?;
powerprofiles::set(powerprofiles::Profile::PowerSaver)?;
```

## Network (NetworkManager via D-Bus)

```rust
use helium::services::network;

let status = network::status()?;
println!("connected: {}, ssid: {:?}", status.connected, status.ssid);
```

## Bluetooth (BlueZ via D-Bus)

```rust
use helium::services::bluetooth;

let enabled = bluetooth::enabled()?;
let devices = bluetooth::devices()?;
```

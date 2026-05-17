# BluetoothService

Bluetooth device management via `bluetoothctl`.

```python
bt = None
try:
    bt = helium.services.BluetoothService.get_default()
except Exception:
    pass

if bt:
    if bt.is_bluetooth_on():
        devices = bt.get_devices()
        for d in devices:
            print(d.name, d.connected)

    bt.set_bluetooth_on(False)
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `connect_signal(signal, cb)` | Registers a signal handler |
| `is_bluetooth_on()` | Checks if Bluetooth is powered |
| `set_bluetooth_on(v)` | Powers Bluetooth on/off |
| `get_devices()` | Returns known devices |

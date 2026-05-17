# NetworkService

WiFi and network status via `nmcli`.

```python
net = None
try:
    net = helium.services.NetworkService.get_default()
except Exception:
    pass

if net:
    ssid = net.get_ssid()
    strength = net.get_strength()    # 0-100
    connected = net.is_connected()
    icon = net.get_icon_name()
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `connect_signal(signal, cb)` | Registers a signal handler |
| `get_ssid()` | Returns current SSID |
| `get_strength()` | Returns signal strength (0-100) |
| `is_connected()` | Returns connection state |
| `get_icon_name()` | Returns icon name for strength |

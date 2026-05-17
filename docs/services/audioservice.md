# AudioService

Volume and mute control via PipeWire or PulseAudio.

```python
audio = None
try:
    audio = helium.services.AudioService.get_default()
    audio.start_polling()
except Exception:
    pass

if audio:
    vol = audio.get_volume()       # 0-100
    audio.set_volume(75)

    muted = audio.get_muted()
    audio.set_muted(True)
    audio.toggle_muted()

    name = audio.get_default_sink_name()
    desc = audio.get_default_sink_description()
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `start_polling(ms=1000)` | Starts polling for changes |
| `stop_polling()` | Stops polling |
| `connect_signal(signal, cb)` | Registers a signal handler |
| `get_volume()` | Returns volume (0-100) |
| `set_volume(percent)` | Sets volume |
| `get_muted()` | Returns mute state |
| `set_muted(v)` | Sets mute state |
| `toggle_muted()` | Toggles mute |
| `get_default_sink_name()` | Returns default sink name |
| `get_default_sink_description()` | Returns default sink description |

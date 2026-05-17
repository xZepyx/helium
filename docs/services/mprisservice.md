# MprisService

Media player control via `playerctl`. Get what's playing and control playback.

```python
mpris = None
try:
    mpris = helium.services.MprisService.get_default()
    mpris.start_polling()
except Exception:
    pass

if mpris:
    players = mpris.get_players()
    if players:
        player = players[0]
        artist = mpris.get_artist(player)
        title = mpris.get_title(player)
        status = mpris.get_status(player)  # "Playing", "Paused", "Stopped"
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `connect_signal(signal, cb)` | Registers a signal handler |
| `get_players()` | Lists active MPRIS players |
| `get_artist(player="")` | Returns track artist |
| `get_title(player="")` | Returns track title |
| `get_status(player="")` | Returns playback status |
| `start_polling(ms=2000)` | Starts polling |
| `stop_polling()` | Stops polling |

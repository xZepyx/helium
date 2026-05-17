# PowerProfilesService

Power profile switching via `powerprofilesctl`.

```python
power = None
try:
    power = helium.services.PowerProfilesService.get_default()
except Exception:
    pass

if power:
    current = power.get_profile()
    profiles = power.get_profiles()
    power.set_profile("balanced")
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `get_profile()` | Returns active profile |
| `set_profile(profile)` | Sets active profile |
| `get_profiles()` | Returns available profiles |

# NotificationService

Desktop notifications via DBus (org.freedesktop.Notifications).

```python
notifications = None
try:
    notifications = helium.services.NotificationService.get_default()
except Exception:
    pass

if notifications:
    notifs = notifications.get_notifications()
    for n in notifs:
        print(n.app_name, n.summary)

    nid = notifications.add_notification(
        "my_app",
        "Hello!",
        body="This is a notification",
        icon="info",
        urgency=1,
    )

    dnd = notifications.get_dnd()
    notifications.set_dnd(True)
    count = notifications.get_count()
```

## Methods

| Method | What it does |
|---|---|
| `get_default()` | Returns the singleton |
| `connect_signal(signal, cb)` | Registers a signal handler |
| `get_dnd()` | Returns Do Not Disturb state |
| `set_dnd(v)` | Sets Do Not Disturb |
| `get_count()` | Number of notifications |
| `get_notifications()` | Returns all notifications |
| `add_notification(app, summary, body="", icon="", urgency=0, category="")` | Sends a notification; returns ID |

# Calendar

A date picker widget.

```python
from helium.types import Calendar

cal = Calendar()
cal.select_day(2026, 5, 17)
cal.mark_day(15)
cal.unmark_day(15)
cal.clear_marks()

cal.set_show_week_numbers(True)
cal.set_show_heading(True)
cal.set_show_day_names(True)

cal.on_day_selected(lambda: print("selected"))
cal.on_month_changed(lambda: print("month changed"))
```

## Constructor

`Calendar()`

## Methods

| Method | What it does |
|---|---|
| `select_day(year, month, day)` | Selects a specific date |
| `select_month(year, month)` | Selects a month |
| `mark_day(day)` | Marks a day |
| `unmark_day(day)` | Unmarks a day |
| `clear_marks()` | Clears all marks |
| `set_show_week_numbers(v)` | Shows week numbers |
| `set_show_heading(v)` | Shows month/year heading |
| `set_show_day_names(v)` | Shows day name headers |
| `on_day_selected(callback)` | Fires on day selection |
| `on_month_changed(callback)` | Fires on month change |

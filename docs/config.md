# Config

Config in Helium uses a proc macro (`helium_config!`) that turns a DSL
into nested serde-deserializable structs with defaults, a `load` method,
a `save` method, and a `reload` method.

---

## The `helium_config!` macro

```rust
use helium::helium_config;

helium_config! {
    HeliumConfig {
        bar: {
            height: u32 = 42,
            position: String = "top",
            modules: {
                clock: {
                    format: String = "%H:%M",
                    interval_ms: u64 = 1000,
                },
                workspaces: {
                    max: u8 = 9,
                },
            },
        },
    }
}
```

### What this generates

| Generated type | Fields |
|----------------|--------|
| `HeliumConfig` | `bar: HeliumConfigBar` |
| `HeliumConfigBar` | `height: u32`, `position: String`, `modules: HeliumConfigBarModules` |
| `HeliumConfigBarModules` | `clock: HeliumConfigBarModulesClock`, `workspaces: HeliumConfigBarModulesWorkspaces` |
| `HeliumConfigBarModulesClock` | `format: String`, `interval_ms: u64` |
| `HeliumConfigBarModulesWorkspaces` | `max: u8` |

Each struct derives `Debug`, `Clone`, `serde::Deserialize`, and `Default`.
Values after `=` are used as defaults when a field is missing from JSON.

### Naming rules for nested types

Struct names are `ParentName` + `PascalCase(field_name)`:

| Parent | Field | Generated type |
|--------|-------|----------------|
| `HeliumConfig` | `bar` | `HeliumConfigBar` |
| `HeliumConfigBar` | `modules` | `HeliumConfigBarModules` |

### Optional fields

Any field type that wraps `Option<T>` works out of the box — missing JSON
fields default to `None`, explicit `null` also produces `None`, and a
present value produces `Some(value)`.

```rust
helium_config! {
    Config {
        label: String = "default",
        tooltip: Option<String>,   // None if missing or null
        volume: Option<f64>,       // None if missing or null
    }
}
```

---

## Loading

Every root struct gets a `load` method:

```rust
let config = HeliumConfig::load("~/.config/helium/bar.json")?;
```

Behaviour:

- **File exists and is valid JSON** — deserialises and returns it.
- **File exists but is garbage** — returns a `helium::ConfigError::Parse`
  with the serde error message (line number, expected token, etc.).
- **File does not exist** — creates parent directories, writes the default
  config as pretty-printed JSON, and returns the default.
- **File cannot be read** — returns `helium::ConfigError::Io` wrapping the
  `std::io::Error`.

---

## Reloading

Re-read the config from a file and replace the current values in place:

```rust
config.reload("config.json")?;
```

This is a shorthand for reading the file and overwriting `self`. It does
not modify the file if loading fails.

---

## Saving

Write the current config back to disk:

```rust
config.save("config.json")?;
```

Creates parent directories if they don't exist. Returns a
`helium::ConfigError` on failure.

---

## Using with Helium

Pass a config to `Helium::from_config` (if that method exists on your
version), or just access the fields directly:

```rust
let config = HeliumConfig::load("config.json")?;
let height = config.bar.height;
```

# Config

Config in Helium uses a proc macro (`helium_config!`) that turns a DSL
into nested serde-deserializable structs with defaults, a `load` method,
a `save` method, and a `reload` method.

## The `helium_config!` macro

```rust
use helium_wsl::helium_config;

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

Each struct derives `Debug`, `Clone`, `serde::Deserialize`, `serde::Serialize`, and `Default`.
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
        tooltip: Option<String>,
        volume: Option<f64>,
    }
}
```

## Loading

Every root struct gets a `load` method:

```rust
let config = HeliumConfig::load("~/.config/helium/bar.json")?;
```

Behaviour:

- **File exists and is valid JSON** — deserialises and returns it.
- **File exists but is garbage** — returns a `helium_wsl::config::ConfigError::Parse`
  with the serde error message.
- **File does not exist** — creates parent directories, writes the default
  config as pretty-printed JSON, and returns the default.
- **File cannot be read** — returns `helium_wsl::config::ConfigError::Io`
  wrapping the `std::io::Error`.

## Reloading

Re-read the config from a file and replace the current values in place:

```rust
config.reload("config.json")?;
```

This does not modify the file if loading fails.

## Saving

Write the current config back to disk:

```rust
config.save("config.json")?;
```

Creates parent directories if they don't exist.

## Using the config

Access the fields directly — there is no `Helium::from_config()` method:

```rust
let config = HeliumConfig::load("config.json")?;
let height = config.bar.height;
```

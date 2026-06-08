# Config

Helium provides a proc macro for declaring nested configuration structs with
serde deserialization and default values.

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

This expands to struct types with `Debug`, `Clone`, and `serde::Deserialize`
derived, plus `Default` (using the values after `=`):

- `HeliumConfig`
- `HeliumConfigBar`
- `HeliumConfigBarModules`
- `HeliumConfigBarModulesClock`
- `HeliumConfigBarModulesWorkspaces`

Each struct has a `pub` field for every entry in its block.

## Naming rules

Nested struct names are formed by concatenating the parent struct name with the
field name, converted to PascalCase. Examples:

| Parent | Field | Generated type |
|---|---|---|
| `HeliumConfig` | `bar` | `HeliumConfigBar` |
| `HeliumConfigBar` | `modules` | `HeliumConfigBarModules` |
| `HeliumConfigBarModules` | `clock` | `HeliumConfigBarModulesClock` |

## Loading

Every root struct gets a `load` method:

```rust
let config = HeliumConfig::load("~/.config/helium/bar.json")?;
```

Fields not present in the JSON file will use their default values. The loader
uses `serde_json` and propagates errors via `helium::ConfigError`.

## Using with Helium

Pass a config to `Helium::from_config`:

```rust
let config = HeliumConfig::load("config.json")?;
let helium = Helium::from_config(config)?;
```

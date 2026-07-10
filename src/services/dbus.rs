use std::sync::{Mutex, OnceLock};
use tokio::runtime::Runtime;
use zvariant::{DynamicType, OwnedValue, Value};

pub(crate) fn rt() -> &'static Runtime {
    static RT: OnceLock<Runtime> = OnceLock::new();
    RT.get_or_init(|| {
        tokio::runtime::Builder::new_multi_thread()
            .worker_threads(1)
            .enable_all()
            .build()
            .expect("failed to create tokio runtime for D-Bus")
    })
}

fn raw_connection() -> Result<zbus::Connection, String> {
    rt().block_on(zbus::Connection::system())
        .map_err(|e| format!("D-Bus connection failed: {e}"))
}

pub fn connection() -> Result<zbus::Connection, String> {
    static CONN: OnceLock<Mutex<Option<zbus::Connection>>> = OnceLock::new();
    let lock = CONN.get_or_init(|| Mutex::new(None));
    let mut guard = lock.lock().map_err(|e| e.to_string())?;
    if let Some(ref conn) = *guard {
        return Ok(conn.clone());
    }
    let conn = raw_connection()?;
    *guard = Some(conn.clone());
    Ok(conn)
}

pub fn call_method<B: serde::Serialize + DynamicType>(
    destination: &str,
    path: &str,
    interface: Option<&str>,
    method: &str,
    body: &B,
) -> Result<zbus::Message, String> {
    let conn = connection()?;
    rt().block_on(
        conn.call_method(Some(destination), path, interface, method, body),
    )
    .map_err(|e| format!("D-Bus call {method} failed: {e}"))
}

fn unwrap_variant(val: &OwnedValue) -> OwnedValue {
    let v: &Value<'static> = val;
    match v {
        Value::Value(boxed) => {
            let inner: Result<OwnedValue, _> = boxed.as_ref().try_into();
            inner.unwrap_or_else(|_| OwnedValue::try_from(v).unwrap())
        }
        _ => OwnedValue::try_from(v).unwrap(),
    }
}

pub fn get_property_raw(
    destination: &str,
    path: &str,
    interface: &str,
    property: &str,
) -> Result<OwnedValue, String> {
    let reply = call_method(
        destination,
        path,
        Some("org.freedesktop.DBus.Properties"),
        "Get",
        &(interface, property),
    )?;
    let val: OwnedValue = reply
        .body()
        .deserialize()
        .map_err(|e| format!("failed to deserialize property {property}: {e}"))?;
    Ok(unwrap_variant(&val))
}

macro_rules! prop_getter {
    ($name:ident, $ty:ty) => {
        pub fn $name(
            dest: &str, path: &str, iface: &str, prop: &str,
        ) -> Result<$ty, String> {
            let val = get_property_raw(dest, path, iface, prop)?;
            <$ty>::try_from(&val).map_err(|e| format!("expected {} for {prop}: {e}", stringify!($ty)))
        }
    };
}

prop_getter!(get_property_u32, u32);
prop_getter!(get_property_bool, bool);
#[allow(dead_code)]
prop_getter!(get_property_i16, i16);

pub fn get_property_string(
    dest: &str, path: &str, iface: &str, prop: &str,
) -> Result<String, String> {
    let val = get_property_raw(dest, path, iface, prop)?;
    let v: &Value<'static> = &val;
    match v {
        Value::Str(s) => Ok(s.to_string()),
        _ => Err(format!("expected string for {prop}")),
    }
}

pub fn get_property_bytes(
    dest: &str, path: &str, iface: &str, prop: &str,
) -> Result<Vec<u8>, String> {
    let val = get_property_raw(dest, path, iface, prop)?;
    let v: &Value<'static> = &val;
    match v {
        Value::Array(arr) => {
            let mut bytes = Vec::new();
            for item in arr.inner() {
                match item {
                    Value::U8(b) => bytes.push(*b),
                    _ => return Err(format!("expected byte array for {prop}")),
                }
            }
            Ok(bytes)
        }
        other => Err(format!("expected array for {prop}, got {other:?}")),
    }
}

pub fn set_property(
    destination: &str,
    path: &str,
    interface: &str,
    property: &str,
    value: &(impl serde::Serialize + DynamicType + zvariant::Type),
) -> Result<(), String> {
    call_method(
        destination,
        path,
        Some("org.freedesktop.DBus.Properties"),
        "Set",
        &(interface, property, value),
    )?;
    Ok(())
}

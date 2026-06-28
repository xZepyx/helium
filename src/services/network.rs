use std::collections::HashMap;
use zvariant::{Array, OwnedValue, Str, Value};

use super::dbus;

const NM: &str = "org.freedesktop.NetworkManager";
const NM_PATH: &str = "/org/freedesktop/NetworkManager";
const NM_IF: &str = "org.freedesktop.NetworkManager";
const DEV_IF: &str = "org.freedesktop.NetworkManager.Device";
const WIFI_IF: &str = "org.freedesktop.NetworkManager.Device.Wireless";
const AP_IF: &str = "org.freedesktop.NetworkManager.AccessPoint";
const SETTINGS_IF: &str = "org.freedesktop.NetworkManager.Settings";
const SCONN_IF: &str = "org.freedesktop.NetworkManager.Settings.Connection";

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionType {
    Wifi,
    Ethernet,
    None,
}

#[derive(Debug, Clone)]
pub struct NetworkStatus {
    pub connected: bool,
    pub ssid: Option<String>,
    pub signal_strength: Option<u8>,
    pub connection_type: ConnectionType,
    pub connectivity: Connectivity,
    pub ip_address: Option<String>,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Connectivity {
    Unknown,
    NoConnection,
    Connecting,
    ConnectedLocal,
    ConnectedSite,
    ConnectedGlobal,
}

#[derive(Debug, Clone)]
pub struct WifiAccessPoint {
    pub ssid: String,
    pub bssid: String,
    pub strength: u8,
    pub frequency: u32,
    pub protected: bool,
}

#[derive(Debug, Clone)]
pub struct SavedConnection {
    pub id: String,
    pub uuid: String,
    pub ssid: Option<String>,
    pub path: String,
}

fn find_wifi_devices() -> Result<Vec<String>, String> {
    let reply = dbus::call_method(NM, NM_PATH, Some(NM_IF), "GetDevices", &())?;
    let paths: Vec<OwnedValue> = reply.body().deserialize()
        .map_err(|e| format!("failed to list devices: {e}"))?;

    let mut wifi = Vec::new();
    for val in paths {
        if let Value::ObjectPath(p) = &*val {
            let path = p.to_string();
            if dbus::get_property_u32(NM, &path, DEV_IF, "DeviceType")? == 2 {
                wifi.push(path);
            }
        }
    }
    Ok(wifi)
}

fn decode_ssid(bytes: &[u8]) -> String {
    String::from_utf8_lossy(bytes).trim_matches('\0').to_string()
}

fn nm_state_to_connectivity(state: u32) -> Connectivity {
    match state {
        70 => Connectivity::ConnectedGlobal,
        60 => Connectivity::ConnectedSite,
        50 => Connectivity::ConnectedLocal,
        40 => Connectivity::Connecting,
        20 | 30 => Connectivity::NoConnection,
        _ => Connectivity::Unknown,
    }
}

fn get_ip_addresses(dev_path: &str) -> Vec<String> {
    let raw: OwnedValue = match dbus::get_property_raw(NM, dev_path, DEV_IF, "Ip4Config") {
        Ok(v) => v,
        Err(_) => return vec![],
    };
    let ip4_path = match &*raw {
        Value::ObjectPath(p) if !p.as_str().is_empty() && p.as_str() != "/" => p.as_str(),
        _ => return vec![],
    };

    let addresses: Vec<Vec<u32>> = match
        dbus::call_method(NM, ip4_path, Some("org.freedesktop.DBus.Properties"), "Get", &("org.freedesktop.NetworkManager.IP4Config", "Addresses"))
            .and_then(|r| r.body().deserialize().map_err(|e| e.to_string()))
    {
        Ok(addrs) => addrs,
        Err(_) => return vec![],
    };

    addresses
        .iter()
        .filter_map(|triplet| {
            let ip = triplet.first()?;
            Some(format!(
                "{}.{}.{}.{}",
                (ip >> 24) & 0xff,
                (ip >> 16) & 0xff,
                (ip >> 8) & 0xff,
                ip & 0xff
            ))
        })
        .collect()
}

pub fn status() -> Result<NetworkStatus, String> {
    let wifi_devs = find_wifi_devices()?;
    let conn_state: u32 = dbus::get_property_u32(NM, NM_PATH, NM_IF, "State")?;
    let connectivity = nm_state_to_connectivity(conn_state);
    let connected = matches!(connectivity, Connectivity::ConnectedGlobal | Connectivity::ConnectedSite | Connectivity::ConnectedLocal);

    let (ssid, signal_strength) = if let Some(dev_path) = wifi_devs.first() {
        match dbus::get_property_raw(NM, dev_path, WIFI_IF, "ActiveAccessPoint") {
            Ok(raw) => match &*raw {
                Value::ObjectPath(p) if !p.as_str().is_empty() && p.as_str() != "/" => {
                    let ap_path = p.as_str().to_string();
                    let raw_ssid = dbus::get_property_bytes(NM, &ap_path, AP_IF, "Ssid").unwrap_or_default();
                    let strength = dbus::get_property_u32(NM, &ap_path, AP_IF, "Strength").unwrap_or(0);
                    (Some(decode_ssid(&raw_ssid)), Some(strength as u8))
                }
                _ => (None, None),
            },
            Err(_) => (None, None),
        }
    } else {
        (None, None)
    };

    let ip_address = if connected {
        wifi_devs.first().map(|d| get_ip_addresses(d).join(", "))
    } else {
        None
    };

    Ok(NetworkStatus {
        connected,
        ssid,
        signal_strength,
        connection_type: if !wifi_devs.is_empty() { ConnectionType::Wifi } else { ConnectionType::None },
        connectivity,
        ip_address,
    })
}

pub fn scan() -> Result<Vec<WifiAccessPoint>, String> {
    let wifi_devs = find_wifi_devices()?;
    if wifi_devs.is_empty() {
        return Err("no Wi-Fi device found".into());
    }

    let dev = &wifi_devs[0];
    let _ = dbus::call_method(NM, dev, Some(WIFI_IF), "RequestScan", &HashMap::<String, OwnedValue>::new());
    std::thread::sleep(std::time::Duration::from_millis(1500));

    let ap_paths: Vec<String> = dbus::get_property_raw(NM, dev, WIFI_IF, "AccessPoints").and_then(|v| {
        match &*v {
            Value::Array(arr) => {
                let items: Vec<String> = arr.inner().iter().filter_map(|item| {
                    if let Value::ObjectPath(p) = item {
                        Some(p.to_string())
                    } else {
                        None
                    }
                }).collect();
                Ok(items)
            }
            _ => Ok(vec![]),
        }
    }).unwrap_or_default();

    let mut aps = Vec::new();
    for path in ap_paths {
        let raw_ssid = match dbus::get_property_bytes(NM, &path, AP_IF, "Ssid") {
            Ok(s) if !s.is_empty() => s,
            _ => continue,
        };

        let ssid = decode_ssid(&raw_ssid);
        let bssid = dbus::get_property_string(NM, &path, AP_IF, "HwAddress").unwrap_or_default();
        let strength = dbus::get_property_u32(NM, &path, AP_IF, "Strength").unwrap_or(0) as u8;
        let frequency = dbus::get_property_u32(NM, &path, AP_IF, "Frequency").unwrap_or(0);
        let rsn = dbus::get_property_u32(NM, &path, AP_IF, "RsnFlags").unwrap_or(0);
        let wpa = dbus::get_property_u32(NM, &path, AP_IF, "WpaFlags").unwrap_or(0);

        aps.push(WifiAccessPoint {
            ssid,
            bssid,
            strength,
            frequency,
            protected: rsn > 0 || wpa > 0,
        });
    }

    aps.sort_by(|a, b| b.strength.cmp(&a.strength));
    aps.dedup_by(|a, b| a.ssid == b.ssid);
    Ok(aps)
}

pub fn connect(ssid: &str, password: Option<&str>) -> Result<(), String> {
    let wifi_devs = find_wifi_devices()?;
    if wifi_devs.is_empty() {
        return Err("no Wi-Fi device found".into());
    }

    let dev_path = &wifi_devs[0];
    let interface = dbus::get_property_string(NM, dev_path, DEV_IF, "Interface")?;
    let uuid = uuid_v4();

    let settings = build_connection_settings(ssid, password, &interface, &uuid);

    let reply = dbus::call_method(NM, "/org/freedesktop/NetworkManager/Settings", Some(SETTINGS_IF), "AddConnection", &settings)?;
    let _conn_path: OwnedValue = reply.body().deserialize()
        .map_err(|e| format!("AddConnection failed: {e}"))?;

    Ok(())
}

fn build_connection_settings(
    ssid: &str,
    password: Option<&str>,
    interface: &str,
    uuid: &str,
) -> HashMap<String, HashMap<String, OwnedValue>> {
    let mut conn = HashMap::new();
    conn.insert("type".to_string(), OwnedValue::from(Str::from("802-11-wireless")));
    conn.insert("uuid".to_string(), OwnedValue::from(Str::from(uuid)));
    conn.insert("id".to_string(), OwnedValue::from(Str::from(ssid)));
    conn.insert("interface-name".to_string(), OwnedValue::from(Str::from(interface)));

    let mut wifi = HashMap::new();
    let ssid_arr: Array<'_> = ssid.as_bytes().to_vec().into();
    let ssid_owned = OwnedValue::try_from(Value::Array(ssid_arr)).unwrap();
    wifi.insert("ssid".to_string(), ssid_owned);
    wifi.insert("mode".to_string(), OwnedValue::from(Str::from("infrastructure")));

    let mut settings: HashMap<String, HashMap<String, OwnedValue>> = HashMap::new();
    settings.insert("connection".to_string(), conn);
    settings.insert("802-11-wireless".to_string(), wifi);

    if let Some(pwd) = password {
        let mut sec = HashMap::new();
        sec.insert("key-mgmt".to_string(), OwnedValue::from(Str::from("wpa-psk")));
        sec.insert("psk".to_string(), OwnedValue::from(Str::from(pwd)));
        settings.insert("802-11-wireless-security".to_string(), sec);
    }

    {
        let mut ip4 = HashMap::new();
        ip4.insert("method".to_string(), OwnedValue::from(Str::from("auto")));
        settings.insert("ipv4".to_string(), ip4);
    }

    {
        let mut ip6 = HashMap::new();
        ip6.insert("method".to_string(), OwnedValue::from(Str::from("auto")));
        settings.insert("ipv6".to_string(), ip6);
    }

    settings
}

pub fn disconnect() -> Result<(), String> {
    let wifi_devs = find_wifi_devices()?;
    if wifi_devs.is_empty() {
        return Err("no Wi-Fi device found".into());
    }
    for dev in &wifi_devs {
        let _ = dbus::call_method(NM, dev, Some(DEV_IF), "Disconnect", &());
    }
    Ok(())
}

pub fn saved_connections() -> Result<Vec<SavedConnection>, String> {
    let reply = dbus::call_method(NM, "/org/freedesktop/NetworkManager/Settings", Some(SETTINGS_IF), "ListConnections", &())?;
    let conn_paths: Vec<OwnedValue> = reply.body().deserialize()
        .map_err(|e| format!("ListConnections failed: {e}"))?;

    let mut results = Vec::new();
    for val in conn_paths {
        let path = match &*val {
            Value::ObjectPath(p) => p.to_string(),
            _ => continue,
        };

        let sreply = match dbus::call_method(NM, &path, Some(SCONN_IF), "GetSettings", &()) {
            Ok(r) => r,
            Err(_) => continue,
        };
        let ssettings: HashMap<String, HashMap<String, OwnedValue>> =
            match sreply.body().deserialize() {
                Ok(s) => s,
                Err(_) => continue,
            };

        let conn_section = match ssettings.get("connection") {
            Some(c) => c,
            None => continue,
        };

        let is_wifi = conn_section.get("type")
            .and_then(|v| match &**v { Value::Str(t) if t.as_str() == "802-11-wireless" => Some(()), _ => None })
            .is_some();
        if !is_wifi {
            continue;
        }

        let id = conn_section.get("id")
            .and_then(|v| match &**v { Value::Str(s) => Some(s.to_string()), _ => None })
            .unwrap_or_default();
        let uuid = conn_section.get("uuid")
            .and_then(|v| match &**v { Value::Str(s) => Some(s.to_string()), _ => None })
            .unwrap_or_default();
        let ssid = ssettings.get("802-11-wireless")
            .and_then(|w| w.get("ssid"))
            .and_then(|v| {
                match &**v {
                    Value::Array(arr) => {
                        let bytes: Vec<u8> = arr.inner().iter().filter_map(|x| {
                            if let Value::U8(b) = x { Some(*b) } else { None }
                        }).collect();
                        Some(decode_ssid(&bytes))
                    }
                    _ => None
                }
            });

        results.push(SavedConnection { id, uuid, ssid, path });
    }

    Ok(results)
}

pub fn forget(ssid_or_uuid: &str) -> Result<(), String> {
    for conn in saved_connections()? {
        if conn.id == ssid_or_uuid || conn.uuid == ssid_or_uuid || conn.ssid.as_deref() == Some(ssid_or_uuid) {
            dbus::call_method(NM, &conn.path, Some(SCONN_IF), "Delete", &())?;
            return Ok(());
        }
    }
    Err(format!("no saved connection matching '{ssid_or_uuid}'"))
}

pub fn on_change(_cb: impl Fn(NetworkStatus) + Send + 'static) -> Result<(), String> {
    Err("NetworkManager D-Bus signal monitoring not yet implemented in blocking API".into())
}

fn uuid_v4() -> String {
    use std::time::{SystemTime, UNIX_EPOCH};
    let now = SystemTime::now().duration_since(UNIX_EPOCH).unwrap_or_default();
    let ns = now.as_nanos();
    format!(
        "{:08x}-{:04x}-4{:03x}-a{:03x}-{:012x}",
        (ns >> 32) as u32,
        (ns >> 16) as u16 & 0xffff,
        (ns >> 4) as u16 & 0xfff,
        (ns >> 0) as u16 & 0xfff,
        ns & 0xffffffffffff
    )
}

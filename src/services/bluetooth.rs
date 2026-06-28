use std::collections::HashMap;
use zvariant::{OwnedValue, Value};

use super::dbus;

const BLUEZ: &str = "org.bluez";
const ADAPTER_IF: &str = "org.bluez.Adapter1";
const DEVICE_IF: &str = "org.bluez.Device1";
const OM_IF: &str = "org.freedesktop.DBus.ObjectManager";

#[derive(Debug, Clone)]
pub struct BtDevice {
    pub name: String,
    pub address: String,
    pub connected: bool,
    pub paired: bool,
    pub battery: Option<u8>,
    pub rssi: Option<i16>,
    pub trusted: bool,
    pub blocked: bool,
    pub icon: Option<String>,
    pub path: String,
}

#[derive(Debug, Clone)]
pub struct BluetoothState {
    pub enabled: bool,
    pub discovering: bool,
    pub discoverable: bool,
    pub pairable: bool,
    pub address: String,
    pub name: String,
    pub devices: Vec<BtDevice>,
    pub adapter_path: Option<String>,
}

fn find_adapter() -> Result<String, String> {
    let objects = get_managed_objects()?;
    for (path, ifaces) in &objects {
        if ifaces.contains_key(ADAPTER_IF) {
            return Ok(path.clone());
        }
    }
    Err("no Bluetooth adapter found".into())
}

fn get_managed_objects() -> Result<HashMap<String, HashMap<String, HashMap<String, OwnedValue>>>, String> {
    let reply = dbus::call_method(BLUEZ, "/", Some(OM_IF), "GetManagedObjects", &())?;
    reply.body().deserialize()
        .map_err(|e| format!("failed to get BlueZ objects: {e}"))
}

fn get_all_devices() -> Result<Vec<BtDevice>, String> {
    let objects = get_managed_objects()?;
    let mut devices = Vec::new();

    for (path, ifaces) in &objects {
        let props = match ifaces.get(DEVICE_IF) {
            Some(p) => p,
            None => continue,
        };

        let name = props.get("Name")
            .and_then(|v| match &**v { Value::Str(s) => Some(s.to_string()), _ => None })
            .unwrap_or_default();
        let address = props.get("Address")
            .and_then(|v| match &**v { Value::Str(s) => Some(s.to_string()), _ => None })
            .unwrap_or_default();
        let connected = props.get("Connected")
            .and_then(|v| match &**v { Value::Bool(b) => Some(*b), _ => None })
            .unwrap_or(false);
        let paired = props.get("Paired")
            .and_then(|v| match &**v { Value::Bool(b) => Some(*b), _ => None })
            .unwrap_or(false);
        let trusted = props.get("Trusted")
            .and_then(|v| match &**v { Value::Bool(b) => Some(*b), _ => None })
            .unwrap_or(false);
        let blocked = props.get("Blocked")
            .and_then(|v| match &**v { Value::Bool(b) => Some(*b), _ => None })
            .unwrap_or(false);
        let battery = props.get("BatteryPercentage")
            .and_then(|v| match &**v { Value::U8(b) => Some(*b), _ => None });
        let rssi = props.get("RSSI")
            .and_then(|v| match &**v { Value::I16(r) => Some(*r), _ => None });
        let icon = props.get("Icon")
            .and_then(|v| match &**v { Value::Str(s) => Some(s.to_string()), _ => None });

        devices.push(BtDevice {
            name,
            address,
            connected,
            paired,
            battery,
            rssi,
            trusted,
            blocked,
            icon,
            path: path.clone(),
        });
    }

    devices.sort_by(|a, b| b.rssi.unwrap_or(-200).cmp(&a.rssi.unwrap_or(-200)));
    Ok(devices)
}

pub fn enabled() -> Result<bool, String> {
    let adapter = find_adapter()?;
    dbus::get_property_bool(BLUEZ, &adapter, ADAPTER_IF, "Powered")
}

pub fn devices() -> Result<Vec<BtDevice>, String> {
    get_all_devices()
}

pub fn state() -> Result<BluetoothState, String> {
    let adapter_path = find_adapter()?;
    let enabled = dbus::get_property_bool(BLUEZ, &adapter_path, ADAPTER_IF, "Powered")?;
    let discovering = dbus::get_property_bool(BLUEZ, &adapter_path, ADAPTER_IF, "Discovering")?;
    let discoverable = dbus::get_property_bool(BLUEZ, &adapter_path, ADAPTER_IF, "Discoverable")?;
    let pairable = dbus::get_property_bool(BLUEZ, &adapter_path, ADAPTER_IF, "Pairable")?;
    let address = dbus::get_property_string(BLUEZ, &adapter_path, ADAPTER_IF, "Address")?;
    let name = dbus::get_property_string(BLUEZ, &adapter_path, ADAPTER_IF, "Name")?;
    let devices = get_all_devices()?;

    Ok(BluetoothState {
        enabled, discovering, discoverable, pairable, address, name, devices,
        adapter_path: Some(adapter_path),
    })
}

pub fn scan() -> Result<Vec<BtDevice>, String> {
    let adapter = find_adapter()?;

    let discovering = dbus::get_property_bool(BLUEZ, &adapter, ADAPTER_IF, "Discovering")?;
    if !discovering {
        dbus::call_method(BLUEZ, &adapter, Some(ADAPTER_IF), "StartDiscovery", &())?;
    }

    std::thread::sleep(std::time::Duration::from_secs(5));

    let mut devices = get_all_devices()?;
    let _ = dbus::call_method(BLUEZ, &adapter, Some(ADAPTER_IF), "StopDiscovery", &());

    devices.retain(|d| d.rssi.is_some());
    devices.sort_by(|a, b| b.rssi.unwrap_or(-200).cmp(&a.rssi.unwrap_or(-200)));
    Ok(devices)
}

pub fn start_discovery() -> Result<(), String> {
    let adapter = find_adapter()?;
    dbus::call_method(BLUEZ, &adapter, Some(ADAPTER_IF), "StartDiscovery", &())?;
    Ok(())
}

pub fn stop_discovery() -> Result<(), String> {
    let adapter = find_adapter()?;
    dbus::call_method(BLUEZ, &adapter, Some(ADAPTER_IF), "StopDiscovery", &())?;
    Ok(())
}

pub fn set_enabled(on: bool) -> Result<(), String> {
    let adapter = find_adapter()?;
    dbus::set_property(BLUEZ, &adapter, ADAPTER_IF, "Powered", &on)?;
    Ok(())
}

pub fn set_discoverable(on: bool) -> Result<(), String> {
    let adapter = find_adapter()?;
    dbus::set_property(BLUEZ, &adapter, ADAPTER_IF, "Discoverable", &on)?;
    Ok(())
}

pub fn set_pairable(on: bool) -> Result<(), String> {
    let adapter = find_adapter()?;
    dbus::set_property(BLUEZ, &adapter, ADAPTER_IF, "Pairable", &on)?;
    Ok(())
}

fn device_path(adapter: &str, address: &str) -> String {
    format!("{}/dev_{}", adapter, address.replace(':', "_"))
}

pub fn connect(device_address: &str) -> Result<(), String> {
    let adapter = find_adapter()?;
    let path = device_path(&adapter, device_address);

    dbus::call_method(BLUEZ, &path, Some(DEVICE_IF), "Connect", &())?;

    for _ in 0..30 {
        if dbus::get_property_bool(BLUEZ, &path, DEVICE_IF, "Connected")? {
            return Ok(());
        }
        std::thread::sleep(std::time::Duration::from_millis(200));
    }
    Err(format!("timed out connecting to {device_address}"))
}

pub fn disconnect(device_address: &str) -> Result<(), String> {
    let adapter = find_adapter()?;
    let path = device_path(&adapter, device_address);
    dbus::call_method(BLUEZ, &path, Some(DEVICE_IF), "Disconnect", &())?;
    Ok(())
}

pub fn pair(device_address: &str) -> Result<(), String> {
    let adapter = find_adapter()?;
    let path = device_path(&adapter, device_address);

    dbus::call_method(BLUEZ, &path, Some(DEVICE_IF), "Pair", &())?;

    for _ in 0..30 {
        if dbus::get_property_bool(BLUEZ, &path, DEVICE_IF, "Paired")? {
            return Ok(());
        }
        std::thread::sleep(std::time::Duration::from_millis(200));
    }
    Err(format!("timed out pairing with {device_address}"))
}

pub fn unpair(device_address: &str) -> Result<(), String> {
    let adapter = find_adapter()?;
    let dev_path = device_path(&adapter, device_address);
    let op = zvariant::ObjectPath::from_str_unchecked(&dev_path);
    dbus::call_method(
        BLUEZ, &adapter, Some(ADAPTER_IF), "RemoveDevice",
        &Value::ObjectPath(op),
    )?;
    Ok(())
}

pub fn trust(device_address: &str, trusted: bool) -> Result<(), String> {
    let adapter = find_adapter()?;
    let path = device_path(&adapter, device_address);
    dbus::set_property(BLUEZ, &path, DEVICE_IF, "Trusted", &trusted)?;
    Ok(())
}

pub fn block(device_address: &str, blocked: bool) -> Result<(), String> {
    let adapter = find_adapter()?;
    let path = device_path(&adapter, device_address);
    dbus::set_property(BLUEZ, &path, DEVICE_IF, "Blocked", &blocked)?;
    Ok(())
}

pub fn on_change(_cb: impl Fn(BluetoothState) + Send + 'static) -> Result<(), String> {
    Err("BlueZ D-Bus signal monitoring not yet implemented in blocking API".into())
}

use clap::{Parser, Subcommand};

#[derive(Parser)]
#[command(name = "helium", about = "Helium WiFi, Bluetooth, and Window CLI")]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Wi-Fi management
    Wifi {
        #[command(subcommand)]
        action: WifiAction,
    },
    /// Bluetooth management
    Bluetooth {
        #[command(subcommand)]
        action: BluetoothAction,
    },
    /// Window focus tracking (WindowDiffusion)
    Window {
        #[command(subcommand)]
        action: WindowAction,
    },
}

#[derive(Subcommand)]
enum WifiAction {
    /// Show current connection status
    Status,
    /// Scan for available access points
    Scan,
    /// Connect to a Wi-Fi network
    Connect {
        /// SSID of the network
        ssid: String,
        /// Password (omit for open networks)
        password: Option<String>,
    },
    /// Disconnect current Wi-Fi
    Disconnect,
    /// List saved connections
    Saved,
    /// Forget a saved connection by SSID or UUID
    Forget {
        /// SSID or UUID to forget
        target: String,
    },
}

#[derive(Subcommand)]
enum BluetoothAction {
    /// Show full Bluetooth adapter and device state
    Status,
    /// List known/paired devices
    Devices,
    /// Scan for nearby Bluetooth devices
    Scan,
    /// Start discovery scan
    ScanStart,
    /// Stop discovery scan
    ScanStop,
    /// Connect to a device by address
    Connect {
        /// Device address (XX:XX:XX:XX:XX:XX)
        address: String,
    },
    /// Disconnect a device by address
    Disconnect {
        /// Device address
        address: String,
    },
    /// Pair with a device
    Pair {
        /// Device address
        address: String,
    },
    /// Unpair/remove a device
    Unpair {
        /// Device address
        address: String,
    },
    /// Enable Bluetooth adapter
    On,
    /// Disable Bluetooth adapter
    Off,
    /// Trust a device
    Trust {
        address: String,
    },
    /// Block a device
    Block {
        address: String,
    },
}

#[derive(Subcommand)]
enum WindowAction {
    /// Show currently focused window
    Focus,
    /// Monitor window focus changes in real time
    Watch,
}

fn main() {
    let cli = Cli::parse();
    match cli.command {
        Commands::Wifi { action } => handle_wifi(action),
        Commands::Bluetooth { action } => handle_bluetooth(action),
        Commands::Window { action } => handle_window(action),
    }
}

fn handle_wifi(action: WifiAction) {
    match action {
        WifiAction::Status => {
            match helium_wsl::services::network::status() {
                Ok(s) => {
                    println!("Connected:      {}", if s.connected { "yes" } else { "no" });
                    println!("Type:           {:?}", s.connection_type);
                    println!("Connectivity:   {:?}", s.connectivity);
                    if let Some(ref ssid) = s.ssid {
                        println!("SSID:           {}", ssid);
                    }
                    if let Some(ref sig) = s.signal_strength {
                        println!("Signal:         {}%", sig);
                    }
                    if let Some(ref ip) = s.ip_address {
                        println!("IP Address:     {}", ip);
                    }
                }
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        WifiAction::Scan => {
            match helium_wsl::services::network::scan() {
                Ok(aps) => {
                    if aps.is_empty() {
                        println!("No access points found.");
                        return;
                    }
                    println!("{:<4} {:<30} {:<18} {:>4} {:>8}  {}", "#", "SSID", "BSSID", "Sig", "Freq", "Sec");
                    for (i, ap) in aps.iter().enumerate() {
                        let sec = if ap.protected { "***" } else { "open" };
                        println!("{:<4} {:<30} {:<18} {:>3}% {:>8}  {}", i + 1, ap.ssid, ap.bssid, ap.strength, ap.frequency, sec);
                    }
                }
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        WifiAction::Connect { ssid, password } => {
            match helium_wsl::services::network::connect(&ssid, password.as_deref()) {
                Ok(()) => println!("Connected to '{ssid}'"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        WifiAction::Disconnect => {
            match helium_wsl::services::network::disconnect() {
                Ok(()) => println!("Disconnected"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        WifiAction::Saved => {
            match helium_wsl::services::network::saved_connections() {
                Ok(conns) => {
                    if conns.is_empty() {
                        println!("No saved connections.");
                        return;
                    }
                    for conn in &conns {
                        if let Some(ref ssid) = conn.ssid {
                            println!("  {} ({}) [{}]", conn.id, ssid, conn.uuid);
                        } else {
                            println!("  {} [{}]", conn.id, conn.uuid);
                        }
                    }
                }
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        WifiAction::Forget { target } => {
            match helium_wsl::services::network::forget(&target) {
                Ok(()) => println!("Forgot '{target}'"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
    }
}

fn handle_bluetooth(action: BluetoothAction) {
    match action {
        BluetoothAction::Status => {
            match helium_wsl::services::bluetooth::state() {
                Ok(s) => {
                    println!("Adapter:        {}", s.name);
                    println!("Address:        {}", s.address);
                    println!("Powered:        {}", if s.enabled { "on" } else { "off" });
                    println!("Discovering:    {}", if s.discovering { "yes" } else { "no" });
                    println!("Discoverable:   {}", if s.discoverable { "yes" } else { "no" });
                    println!("Pairable:       {}", if s.pairable { "yes" } else { "no" });
                    println!("Devices:        {}", s.devices.len());
                    for d in &s.devices {
                        let conn = if d.connected { "connected" } else { "disconnected" };
                        let bat = d.battery.map(|b| format!(" {b}%")).unwrap_or_default();
                        println!("  {} {} {} {}", d.name, d.address, conn, bat);
                    }
                }
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Devices => {
            match helium_wsl::services::bluetooth::devices() {
                Ok(devs) => {
                    if devs.is_empty() {
                        println!("No devices found.");
                        return;
                    }
                    for d in &devs {
                        let conn = if d.connected { "(C)" } else { "" };
                        let paired = if d.paired { "(P)" } else { "" };
                        let rssi = d.rssi.map(|r| format!(" {}dBm", r)).unwrap_or_default();
                        println!("  {} {} {}{}{}", d.name, d.address, paired, conn, rssi);
                    }
                }
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Scan => {
            println!("Scanning for 5 seconds...");
            match helium_wsl::services::bluetooth::scan() {
                Ok(devs) => {
                    if devs.is_empty() {
                        println!("No devices found during scan.");
                        return;
                    }
                    println!("Found {} device(s):", devs.len());
                    for d in &devs {
                        let rssi = d.rssi.map(|r| format!(" {}dBm", r)).unwrap_or_default();
                        println!("  {} {} {}", d.name, d.address, rssi);
                    }
                }
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::ScanStart => {
            match helium_wsl::services::bluetooth::start_discovery() {
                Ok(()) => println!("Discovery started"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::ScanStop => {
            match helium_wsl::services::bluetooth::stop_discovery() {
                Ok(()) => println!("Discovery stopped"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Connect { address } => {
            match helium_wsl::services::bluetooth::connect(&address) {
                Ok(()) => println!("Connected to {address}"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Disconnect { address } => {
            match helium_wsl::services::bluetooth::disconnect(&address) {
                Ok(()) => println!("Disconnected {address}"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Pair { address } => {
            match helium_wsl::services::bluetooth::pair(&address) {
                Ok(()) => println!("Paired with {address}"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Unpair { address } => {
            match helium_wsl::services::bluetooth::unpair(&address) {
                Ok(()) => println!("Unpaired {address}"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::On => {
            match helium_wsl::services::bluetooth::set_enabled(true) {
                Ok(()) => println!("Bluetooth enabled"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Off => {
            match helium_wsl::services::bluetooth::set_enabled(false) {
                Ok(()) => println!("Bluetooth disabled"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Trust { address } => {
            match helium_wsl::services::bluetooth::trust(&address, true) {
                Ok(()) => println!("Trusted {address}"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
        BluetoothAction::Block { address } => {
            match helium_wsl::services::bluetooth::block(&address, true) {
                Ok(()) => println!("Blocked {address}"),
                Err(e) => eprintln!("Error: {e}"),
            }
        }
    }
}

fn handle_window(action: WindowAction) {
    match action {
        WindowAction::Focus => {
            #[cfg(not(feature = "compositors"))]
            {
                eprintln!("Compositor support not enabled");
                return;
            }
            #[cfg(feature = "compositors")]
            {
                match helium_wsl::compositors::detect() {
                    Ok(mut comp) => {
                        match comp.active_window() {
                            Some(w) => {
                                println!("Focused window:");
                                println!("  Title:  {}", w.title);
                                println!("  Class:  {}", w.class);
                                println!("  WS ID:  {}", w.workspace_id);
                            }
                            None => println!("No window focused"),
                        }
                    }
                    Err(e) => eprintln!("Error: {e}"),
                }
            }
        }
        WindowAction::Watch => {
            #[cfg(not(feature = "compositors"))]
            {
                eprintln!("Compositor support not enabled");
                return;
            }
            #[cfg(feature = "compositors")]
            {
                match helium_wsl::compositors::detect() {
                    Ok(mut comp) => {
                        println!("Watching for window focus changes (Ctrl+C to stop)...");
                        if let Some(_fd) = comp.event_fd() {
                                            let mut _last_focus: Option<helium_wsl::compositors::Window> = None;
                            loop {
                                if let Some(event) = comp.poll_event() {
                                    match event {
                                        helium_wsl::CompositorEvent::WindowFocused(w) => {
                                            println!("Focused:  {} ({})", w.title, w.class);
                                            _last_focus = Some(w);
                                        }
                                        helium_wsl::CompositorEvent::WindowDiffusion(d) => {
                                            let unfocused = d.unfocused.map(|w| format!("{} ({})", w.title, w.class)).unwrap_or("none".into());
                                            let focused = d.focused.map(|w| format!("{} ({})", w.title, w.class)).unwrap_or("none".into());
                                            println!("Diffusion: {unfocused} → {focused}");
                                        }
                                        _ => {}
                                    }
                                } else {
                                    std::thread::sleep(std::time::Duration::from_millis(50));
                                }
                            }
                        } else {
                            eprintln!("Compositor does not support event fd");
                        }
                    }
                    Err(e) => eprintln!("Error: {e}"),
                }
            }
        }
    }
}

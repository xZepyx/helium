import helium

helium.init()
helium.load_css("gradle/style.css")

# 1. Create the Main Panel
panel = helium.Panel(
    namespace="my-shell",
    anchor=["top", "left", "right"],
    height=40
)

# 2. Create a Container (Box)
# This will hold multiple widgets in a row
content_box = helium.Box(
    orientation="horizontal",
    spacing=12
)

# 3. Create some "stuff"
main_label = helium.Label("Helium Shell")

btn_power = helium.Button("Power")
btn_apps = helium.Button("Apps")

# 4. Pack everything together
content_box.add(main_label)
content_box.add(btn_apps)
content_box.add(btn_power)

# 5. Set the box as the panel's child and show
panel.set_child(content_box)
panel.show()

# Add your CSS class for styling
panel.add_css_class("panel")

helium.run()
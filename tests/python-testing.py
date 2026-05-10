import helium

helium.init()

panel = helium.Panel(
    anchor=[
        "top",
        "left",
        "right"
    ],
    height = 40
)

label = helium.Label(
    "hello from python"
)

panel.set_child(label)
panel.show()

panel.add_css_class(
    "panel"
)

helium.run()
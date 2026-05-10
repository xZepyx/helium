import helium

helium.init()

props = helium.PanelProperties()

props.namespace_ = "example"

props.anchor = [
    "top",
    "left",
    "right"
]

props.height = 40

panel = helium.Panel(props)

label = helium.Label(
    "hello from python"
)

panel.set_child(label)

panel.show()

helium.run()
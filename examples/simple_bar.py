import helium
from helium.types import Panel, Label

helium.init()

panel = Panel(
    namespace="hello",
    anchor=["left", "top", "right"],
)
panel.set_child(Label("hello world"))
panel.show()

helium.run()

from helium.types import Panel, Label, Box
from helium.functions import Poll
import helium
import datetime

helium.init()
helium.load_css("gradle/style.css")

class Bar(Panel):
    def __init__(self):
        super().__init__(
            namespace="my-shell", 
            anchor=["top", "left", "right"], 
            height=40
        )
        
        # 1. Left Section: Apps
        self.left_box = Box(halign="start")
        self.left_box.add(Label("Apps"))
        self.left_box.add_css_class("leftbox")

        # 2. Center Section: No 1-10
        self.center_box = Box(halign="center")
        self.center_box.add(Label("Workspaces"))
        self.center_box.add_css_class("centerbox")

        # 3. Right Section: Clock
        self.right_box = Box(halign="end")
        self.clock_label = Label("Loading...")
        self.right_box.add(self.clock_label)
        self.right_box.add_css_class("rightbox")

        # 4. Main Container (The "cbox")
        # We put all three inside a horizontal box that fills the panel
        self.main_container = Box(
            orientation="horizontal",
            children=[self.left_box, self.center_box, self.right_box],
            halign="fill"
        )
        
        # Ensure the sub-boxes expand to fill the width so halign works
        self.left_box.set_hexpand(True)
        self.center_box.set_hexpand(True)
        self.right_box.set_hexpand(True)

        self.set_child(self.main_container)
        self.add_css_class("panel")
        self.show()
        
        Poll(1000, self.update_clock)

    def update_clock(self):
        now = datetime.datetime.now().strftime("%H:%M:%S")
        self.clock_label.set_label(now)
        return True 

Bar()
helium.run()
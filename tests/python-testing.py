import json
import helium
from helium.types import Panel, Box, CenterBox, Label, Button
from helium.compositor.hyprland import hyprland_dispatch, get_active_workspace

# 1. Initialize the library
helium.init()

# 2. Load CSS (Ensure this path is correct for your Arch setup)
helium.load_css("tests/style.css")

class WorkspaceIndicator(Box):
    def __init__(self):
        # halign="center" ensures the pill stays compact in the middle
        super().__init__(orientation="horizontal", spacing=2, halign="center")
        self.add_css_class("workspace-pill")
        
        # 3. Create the buttons ONCE and store them in a list
        self.buttons = []
        for i in range(1, 9):
            btn = Button(label=str(i))
            btn.add_css_class("ws-button")
            
            # Connect the click event once. *args prevents the TypeError.
            btn.on_click(lambda *args, idx=i: hyprland_dispatch(f"dispatch workspace {idx}"))
            
            self.add(btn)
            self.buttons.append(btn)
        
        # Initial state check
        self.update()
        
        # Poll Hyprland every 200ms
        helium.functions.Poll(200, self.update)

    def update(self):
        try:
            # 4. Fetch state without clearing any widgets
            active_data = json.loads(get_active_workspace())
            active_id = int(active_data.get("id", 1))

            # 5. Just flip the 'active' class on the existing buttons
            for i, btn in enumerate(self.buttons, 1):
                if i == active_id:
                    btn.add_css_class("active")
                else:
                    btn.remove_css_class("active")
                
        except Exception as e:
            print(f"Helium IPC Error: {e}")
            
        return True

class TopBar(Panel):
    def __init__(self):
        super().__init__(
            namespace="helium-bar",
            anchor=["top", "left", "right"],
            height=55,
            exclusive=True,
            layer="top"
        )

        self.layout = CenterBox()
        self.workspaces = WorkspaceIndicator()

        # Set center only as per your variant
        self.layout.set_center(self.workspaces)

        self.set_child(self.layout)
        self.show()

# Create the bar and run
TopBar()
print("Helium Shell initialized. Running main loop...")
helium.run()
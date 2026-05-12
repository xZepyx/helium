from helium.types import Panel, Box, Entry, Label
import helium

helium.init()
helium.load_css("gradle/style.css")

class Launcher(Panel):
    def __init__(self):
        super().__init__(
            namespace="helium-launcher",
            # Empty anchor centers the window on most compositors
            anchor=[], 
            width=500,
            height=60,
            exclusive=False,
            layer="overlay",
            kb_mode="exclusive" # Grabs keyboard focus immediately
        )

        # 1. Setup the Input Field
        self.entry = Entry()
        self.entry.add_css_class("launcher-input")
        
        # Connect the 'Enter' key press
        self.entry.on_activate(self.on_submit)

        # 2. Layout
        self.main_container = Box(
            orientation="vertical",
            children=[self.entry],
        )
        self.main_container.add_css_class("launcher-window")

        self.set_child(self.main_container)
        self.show()

    def on_submit(self):
        query = self.entry.get_text()
        print(f"User searched for: {query}")
        
        # Clear text after search
        self.entry.set_text("")
        
        # You could add logic here to close the launcher or spawn a process
        # self.hide() 

Launcher()
helium.run()
from helium.types import Panel, Label, Box, Button
from helium.functions import Poll
import helium
import datetime

# Initialize the framework
helium.init()
helium.load_css("gradle/style.css")

class Bar(Panel):
    def __init__(self):
        # 1. Initialize the Panel via helium.types.Panel
        # The arguments map to our C++ lambda wrapper in module.cpp
        super().__init__(
            namespace="my-shell", 
            anchor=["top", "left", "right"], 
            height=40
        )
        
        # 2. Setup the UI components
        self.clock_label = Label("Loading...")
        self.clock_label.add_css_class("clocklabel")
        self.set_child(self.clock_label)
        
        self.add_css_class("panel")
        self.show()
        
        # 3. Use the built-in library poller from helium.functions
        # We pass the method reference directly.
        Poll(1000, self.update_clock)

    def update_clock(self):
        """
        Updates the label with current time.
        Returns True to keep the timer alive.
        """
        now = datetime.datetime.now().strftime("%H:%M:%S")
        self.clock_label.set_label(now)
        return True 

# Instantiate and start the GTK main loop
Bar()
helium.run()
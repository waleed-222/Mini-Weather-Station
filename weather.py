import csv
import os
import tkinter as tk
import requests
from datetime import datetime, timedelta
from ttkbootstrap import Style
from ttkbootstrap.widgets import LabelFrame, Label


CSV_FILE = "dht_data.csv"


class DHTApp:
    def __init__(self, root):
        self.style = Style("pulse")  # Try also: minty, flatly, solar, morph...
        self.root = root
        self.root.title("DHT Sensor Monitor")
        self.root.geometry("400x300")

        # Frame for Date & Time
        self.datetime_frame = tk.Frame(self.root, pady=10)
        self.datetime_frame.pack()

        self.date_label = Label(self.datetime_frame, text="Date: --", font=("Helvetica", 14))
        self.date_label.pack()

        self.time_label = Label(self.datetime_frame, text="Time: --", font=("Helvetica", 14))
        self.time_label.pack()

        # Frame for Location
        self.location_frame=tk.Frame(self.root,pady=20)
        self.location_frame.pack()
        
        
        self.country_label=Label(self.location_frame,text="Country:--",font=("Helvetica",14))
        self.country_label.pack()
        
        self.city_label=Label(self.location_frame,text="City:--",font=("Helvetica",14))
        self.city_label.pack()
        

        # Frame for Sensor Cards
        self.card_frame = tk.Frame(self.root, pady=30)
        self.card_frame.pack()
        
        # Temperature Card
        self.temp_card = LabelFrame(self.card_frame, text="Temperature", bootstyle="primary", padding=10)
        self.temp_card.grid(row=0, column=0, padx=20)
        self.temp_value = Label(self.temp_card, text="-- °C", font=("Helvetica", 18, "bold"))
        self.temp_value.pack()

        # Humidity Card
        self.humid_card = LabelFrame(self.card_frame, text="Humidity", bootstyle="success", padding=10)
        self.humid_card.grid(row=0, column=1, padx=20)
        self.humid_value = Label(self.humid_card, text="-- %", font=("Helvetica", 18, "bold"))
        self.humid_value.pack()

        self.update_data()  # Trigger first update immediately
        self.schedule_update_at_zero()
        

    def schedule_update_at_zero(self):
        now = datetime.now()
        next_minute = (now + timedelta(minutes=1)).replace(second=1, microsecond=0)
        delay = (next_minute - now).total_seconds() * 1000  # convert to milliseconds

        self.root.after(int(delay), self.update_data)

    def update_data(self):
        if not os.path.exists(CSV_FILE):
            return  # No reschedule here to avoid double call

        with open(CSV_FILE, "r") as f:
            lines = list(csv.reader(f))
            if len(lines) < 2:
                return

            last_row = lines[-1]
            if len(last_row) != 3:
                return

            timestamp, temp, humid = last_row
            dt = datetime.strptime(timestamp, "%Y-%m-%d %H:%M:%S")

            try:
                respond = requests.get("https://ipinfo.io/json", timeout=3)
                data = respond.json()
                country = data.get("country", "--")
                timezone = data.get("timezone", "--/--")
                city = timezone.split("/")[-1] if "/" in timezone else "--"
            except Exception:
                country = "--"
                city = "--"

            self.date_label.config(text=f"Date: {dt.strftime('%Y-%m-%d')}")
            self.time_label.config(text=f"Time: {dt.strftime('%H:%M:%S')}")
            self.country_label.config(text=f"Country: {country}")
            self.city_label.config(text=f"City: {city}")
            self.temp_value.config(text=f"{temp} °C")
            self.humid_value.config(text=f"{humid} %")
    
       



if __name__ == "__main__":
    root = tk.Tk()
    app = DHTApp(root)
    root.mainloop()


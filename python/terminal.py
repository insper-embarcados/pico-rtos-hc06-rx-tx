import json
import tkinter as tk
from tkinter import ttk, scrolledtext
import serial
import serial.tools.list_ports
import threading

BAUDS = ["9600", "19200", "38400", "57600", "115200", "230400"]
DEFAULT_BAUD = "115200"
BG = "#1e1e1e"
FG = "#d4d4d4"
BG_ENTRY = "#ffffff"
FG_ENTRY = "#1e1e1e"
BG_TERMINAL = "#121212"
BG_BUTTON_CONNECT = "#2e7d32"
BG_BUTTON_DISCONNECT = "#7b2020"
BG_BUTTON_SEND = "#1565c0"
FONT_TERMINAL = ("Consolas", 10)
FONT_UI = ("Segoe UI", 9)


class PanelSerial:
    def __init__(self, parent, title, align="left"):
        self.serial = None
        self.running = False
        self.align = align

        frame = tk.Frame(parent, bg=BG)
        frame.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        tk.Label(frame, text=title, bg=BG, fg=FG,
                 font=("Segoe UI", 10, "bold")).pack(anchor="e" if align == "right" else "w")

        top = tk.Frame(frame, bg=BG)
        top.pack(fill=tk.X, pady=(2, 4))

        self.port_var = tk.StringVar()
        self.baud_var = tk.StringVar(value=DEFAULT_BAUD)

        if align == "right":
            self.btn_connect = tk.Button(top, text="Conectar", bg=BG_BUTTON_CONNECT, fg="white",
                                         font=FONT_UI, relief=tk.FLAT, padx=8,
                                         command=self.toggle_connection)
            self.btn_connect.pack(side=tk.RIGHT, padx=(4, 0))

            self.baud_cb = ttk.Combobox(top, textvariable=self.baud_var, values=BAUDS,
                                        width=8, font=FONT_UI, state="readonly")
            self.baud_cb.pack(side=tk.RIGHT, padx=(4, 0))
            self.baud_cb.set(DEFAULT_BAUD)

            self.port_cb = ttk.Combobox(top, textvariable=self.port_var,
                                        width=12, font=FONT_UI, state="readonly")
            self.port_cb.pack(side=tk.RIGHT, padx=(4, 0))
        else:
            self.port_cb = ttk.Combobox(top, textvariable=self.port_var,
                                        width=12, font=FONT_UI, state="readonly")
            self.port_cb.pack(side=tk.LEFT, padx=(0, 4))

            self.baud_cb = ttk.Combobox(top, textvariable=self.baud_var, values=BAUDS,
                                        width=8, font=FONT_UI, state="readonly")
            self.baud_cb.pack(side=tk.LEFT, padx=(0, 4))
            self.baud_cb.set(DEFAULT_BAUD)

            self.btn_connect = tk.Button(top, text="Conectar", bg=BG_BUTTON_CONNECT, fg="white",
                                         font=FONT_UI, relief=tk.FLAT, padx=8,
                                         command=self.toggle_connection)
            self.btn_connect.pack(side=tk.LEFT, padx=(0, 4))

        self.terminal = scrolledtext.ScrolledText(frame, bg=BG_TERMINAL, fg=FG,
                                                  font=FONT_TERMINAL, relief=tk.FLAT,
                                                  wrap=tk.WORD, state=tk.DISABLED)
        self.terminal.pack(fill=tk.BOTH, expand=True)

        # Configure tags once after terminal widget is created
        j = tk.RIGHT if align == "right" else tk.LEFT
        self.terminal.tag_config("info",    foreground="#4fc3f7", justify=j)
        self.terminal.tag_config("error",   foreground="#ef5350", justify=j)
        self.terminal.tag_config("sent",    foreground="#81c784", justify=j)
        self.terminal.tag_config("default", justify=j)

        bottom = tk.Frame(frame, bg=BG)
        bottom.pack(fill=tk.X, pady=(4, 0))

        self.input_var = tk.StringVar()
        self.entry = tk.Entry(bottom, textvariable=self.input_var, bg=BG_ENTRY, fg=FG_ENTRY,
                              font=FONT_TERMINAL, relief=tk.FLAT, insertbackground=FG_ENTRY)
        self.entry.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 4), ipady=4)
        self.entry.bind("<Return>", lambda e: self.send())

        self.btn_send = tk.Button(bottom, text="Enviar", bg=BG_BUTTON_SEND, fg="white",
                                  font=FONT_UI, relief=tk.FLAT, padx=10,
                                  command=self.send)
        self.btn_send.pack(side=tk.LEFT)

        self.refresh_ports()

    def write_terminal(self, text, tag=None):
        text = text or ""
        effective_tag = tag or "default"

        def _write():
            self.terminal.config(state=tk.NORMAL)
            self.terminal.insert(tk.END, text + "\n", effective_tag)
            self.terminal.see(tk.END)
            self.terminal.config(state=tk.DISABLED)

        self.terminal.after(0, _write)

    def refresh_ports(self):
        ports = [p.device for p in serial.tools.list_ports.comports()]
        self.port_cb["values"] = ports
        if ports and not self.port_var.get():
            self.port_var.set(ports[0])

    def toggle_connection(self):
        if self.serial and self.serial.is_open:
            self.disconnect()
        else:
            self.connect()

    def connect(self):
        port = self.port_var.get()
        baud = int(self.baud_var.get())
        if not port:
            self.write_terminal("[nenhuma porta selecionada]\n", tag="error")
            return
        try:
            self.serial = serial.Serial(port, baud, timeout=0.1)
            self.running = True
            self.btn_connect.config(text="Desconectar", bg=BG_BUTTON_DISCONNECT)
            self.write_terminal(f"[conectado em {port} @ {baud}]\n", tag="info")
            threading.Thread(target=self._read_loop, daemon=True).start()
        except serial.SerialException as e:
            self.write_terminal(f"[erro: {e}]\n", tag="error")

    def disconnect(self):
        self.running = False
        if self.serial:
            self.serial.close()
            self.serial = None
        self.btn_connect.config(text="Conectar", bg=BG_BUTTON_CONNECT)
        self.write_terminal("[desconectado]\n", tag="info")

    def _read_loop(self):
        while self.running and self.serial and self.serial.is_open:
            try:
                data = self.serial.read(256)
                if data:
                    text = data.decode("utf-8", errors="replace")
                    self.write_terminal(text)
            except serial.SerialException:
                self.running = False
                self.terminal.after(0, lambda: self.write_terminal("[conexao perdida]\n", tag="error"))
                self.terminal.after(0, lambda: self.btn_connect.config(text="Conectar", bg=BG_BUTTON_CONNECT))
                break

    def send(self):
        if not self.serial or not self.serial.is_open:
            return
        text = self.input_var.get()
        if not text:
            return
        try:
            self.serial.write((text + "\r\n").encode("utf-8"))
            self.write_terminal(f"> {text}\n", tag="sent")
        except serial.SerialException as e:
            self.write_terminal(f"[erro ao enviar: {e}]\n", tag="error")
        self.input_var.set("")


class App:
    def __init__(self, root):
        self.root = root
        self.settings_file = "settings.json"

        root.protocol("WM_DELETE_WINDOW", self.on_closing)
        root.title("Serial Terminal")
        root.configure(bg="#e0e0e0")
        root.geometry("733x620")
        root.minsize(500, 400)

        self._apply_style()

        toolbar = tk.Frame(root, bg="#e0e0e0")
        toolbar.pack(fill=tk.X, padx=6, pady=(6, 0))

        tk.Button(toolbar, text="Atualizar Portas", bg="#424242", fg="white",
                  font=FONT_UI, relief=tk.FLAT, padx=8,
                  command=self.refresh_all).pack(side=tk.LEFT)

        panes = tk.Frame(root, bg="#e0e0e0")
        panes.pack(fill=tk.BOTH, expand=True, padx=6, pady=6)

        panes.columnconfigure(0, weight=1, uniform="half")
        panes.columnconfigure(2, weight=1, uniform="half")
        panes.rowconfigure(0, weight=1)

        left_frame  = tk.Frame(panes, bg=BG)
        right_frame = tk.Frame(panes, bg=BG)

        left_frame.grid(row=0, column=0, sticky="nsew")
        tk.Frame(panes, bg="#e0e0e0", width=36).grid(row=0, column=1, sticky="ns")
        right_frame.grid(row=0, column=2, sticky="nsew")

        self.left  = PanelSerial(left_frame,  "Pico  (UART)")
        self.right = PanelSerial(right_frame, "HC-06  (Bluetooth)", align="right")

        self._load_settings()

    def _load_settings(self):
        try:
            with open(self.settings_file, "r") as file:
                settings = json.load(file)
            self.left.port_var.set(settings.get("pico_port", ""))
            self.left.baud_var.set(settings.get("pico_baud", DEFAULT_BAUD))
            self.right.port_var.set(settings.get("hc06_port", ""))
            self.right.baud_var.set(settings.get("hc06_baud", DEFAULT_BAUD))
        except (FileNotFoundError, json.JSONDecodeError):
            pass

    def _apply_style(self):
        style = ttk.Style()
        style.theme_use("clam")
        style.configure("TCombobox",
                        fieldbackground=BG_ENTRY, background=BG_ENTRY,
                        foreground=FG_ENTRY, arrowcolor="#555555", bordercolor=BG_ENTRY,
                        selectbackground=BG_ENTRY, selectforeground=FG_ENTRY)
        style.map("TCombobox",
                  fieldbackground=[("readonly", BG_ENTRY), ("disabled", BG_ENTRY)],
                  foreground=[("readonly", FG_ENTRY), ("disabled", FG_ENTRY)],
                  selectbackground=[("readonly", BG_ENTRY)],
                  selectforeground=[("readonly", FG_ENTRY)])

    def on_closing(self):
        settings = {
            "pico_port": self.left.port_var.get(),
            "pico_baud": self.left.baud_var.get(),
            "hc06_port": self.right.port_var.get(),
            "hc06_baud": self.right.baud_var.get()
        }
        with open(self.settings_file, "w") as file:
            json.dump(settings, file)
        self.root.destroy()

    def refresh_all(self):
        self.left.refresh_ports()
        self.right.refresh_ports()


if __name__ == "__main__":
    root = tk.Tk()
    App(root)
    root.mainloop()

import http.server
import socketserver
import json
import random
import time
import os

PORT = 8000
WEB_DIR = "web"

class MockHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def end_headers(self):
        self.send_header("Cache-Control", "no-cache, no-store, must-revalidate")
        self.send_header("Pragma", "no-cache")
        self.send_header("Expires", "0")
        super().end_headers()

    def do_GET(self):
        if self.path.startswith("/api/status"):
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            
            # Mock data
            data = {
                "mesauredTemperature": 92.5 + random.uniform(-0.5, 0.5),
                "targetTemperature": 96.5,
                "heaterPower": random.randint(0, 1000),
                "ecoTimeRemaining": 1200000 - (int(time.time()) % 1000) * 1000 # Just some counting down number
            }
            self.wfile.write(json.dumps(data).encode())
            return

        elif self.path.startswith("/api/config"):
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            
            data = {
                "tset": 96.5,
                "tband": 1.5,
                "eco_time": 0,
                "mqtt_enabled": 0,
                "mqtt_topic": "esp_silvia",
                "rref": 430,
                "pgain": 1100,
                "igain": 0,
                "dgain": 0,
                "apgain": 1500,
                "aigain": 0,
                "adgain": 0,
                "tunethres": 50,
                "tunestep": 500,
                "fw_version": "v1.0.0-mock",
                "git_commit": "abcdef",
                "build_time": "2023-01-01 12:00:00"
            }
            self.wfile.write(json.dumps(data).encode())
            return
            
        elif self.path.startswith("/heater_on"):
            print("Action: Heater ON")
            self.send_response(200)
            self.end_headers()
            return

        elif self.path.startswith("/heater_off"):
            print("Action: Heater OFF")
            self.send_response(200)
            self.end_headers()
            return
            
        elif self.path.startswith("/set_config"):
            print(f"Action: Set Config {self.path}")
            self.send_response(200)
            self.end_headers()
            return

        # Serve static files
        return http.server.SimpleHTTPRequestHandler.do_GET(self)

print(f"Starting mock server at http://localhost:{PORT}")
print(f"Serving files from ./{WEB_DIR}")
print("Press Ctrl+C to stop")

socketserver.TCPServer.allow_reuse_address = True
with socketserver.TCPServer(("", PORT), MockHandler) as httpd:
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass

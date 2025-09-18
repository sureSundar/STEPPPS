#!/usr/bin/env python3
"""
Simple HTTP server for STEPPPS Protocol testing
Serves the HTML file with proper CORS headers
"""

import http.server
import socketserver
import os
import sys
from pathlib import Path

class CORSHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', '*')
        super().end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.end_headers()

def main():
    # Change to STEPPPS directory
    steppps_dir = Path(__file__).parent
    os.chdir(steppps_dir)

    PORT = 8000

    with socketserver.TCPServer(("", PORT), CORSHTTPRequestHandler) as httpd:
        print(f"🚀 STEPPPS Protocol Server starting...")
        print(f"📁 Serving files from: {steppps_dir}")
        print(f"🌐 Open your browser and go to:")
        print(f"   http://localhost:{PORT}/simple-pixel-test.html")
        print(f"⚡ Server running on port {PORT}")
        print(f"🛑 Press Ctrl+C to stop")

        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print(f"\n🛑 Server stopped")
            sys.exit(0)

if __name__ == "__main__":
    main()
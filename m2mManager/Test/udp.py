import socket
import sys

# Default values (if no arguments are passed)
BUFFER_SIZE = 4096

def main():
    # Get server IP and port from arguments
    if len(sys.argv) != 3:
        print("Usage: python udp_client.py <server_ip> <server_port>")
        sys.exit(1)

    server_ip = sys.argv[1]
    try:
        server_port = int(sys.argv[2])
    except ValueError:
        print("Port must be an integer.")
        sys.exit(1)

    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(1)  # 1 second timeout for response

    try:
        while True:
            message = input("Enter message (or 'quit' to exit): ")
            if message.lower() == "quit":
                break

            # Ensure exactly one newline at the end
            message = message.rstrip("\n") + "\n"

            # Send message
            sock.sendto(message.encode(), (server_ip, server_port))

            # Try to receive response within 1 second
            try:
                data, addr = sock.recvfrom(BUFFER_SIZE)
                print(f"Response from {addr}: {data.decode()}")
            except socket.timeout:
                print("..")

    finally:
        sock.close()

if __name__ == "__main__":
    main()


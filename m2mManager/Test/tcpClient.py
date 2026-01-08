import socket
import sys

def main():

    #  must have server IP and Server Port
    if len(sys.argv) != 3:
        print("Usage: python tcpClient.py <server_ip> <server_port>")
        sys.exit(1)

    serverIp = sys.argv[1]
    try:
        servePort = int(sys.argv[2])
        
    except ValueError:
        print("Port must be an integer.")
        sys.exit(1)

    # Validate port 
    if not (0 <= servePort <= 65535):
        print("Error: Port must be  unsigned short ")
        sys.exit(1)

   
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.settimeout(1)

    try:
        
        client_socket.connect((serverIp, servePort))
        print("Enter 'quit' to Exit ")
        while True:
            # Get input from the user
            message = input("Enter M2M Command: ")

            # Exit condition: if the user types 'quit', break the loop
            if message.lower() == "quit":
                print("Exiting client.")
                break

            # Force a newline
            message = message.rstrip("\n") + "\n"

            # Send Message
            client_socket.sendall(message.encode())

            # Wait for a reply (with a timeout of 1 second)
            try:
                response = client_socket.recv(4096)
                if response:
                    print(f"M2M Response: {response.decode()}")
                else:
                    print("..")
            except socket.timeout:
                print("..")
    except (socket.error, ConnectionError) as e:
        print(f"Server Socket Error : {e}")
    finally:
        client_socket.close()

if __name__ == "__main__":
    main()


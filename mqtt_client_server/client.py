import socket

 

msgFromClient = "Hello UDP Server"
bytesToSend = str.encode(msgFromClient)
# serverAddressPort= ("10.13.210.82", 8080)
serverAddressPort= ("127.0.0.1", 9000)

bufferSize = 1024
 

# Create a UDP socket at client side
UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)
# Send to server using created UDP socket
UDPClientSocket.sendto(bytesToSend, serverAddressPort)
# msgFromServer = UDPClientSocket.recvfrom(bufferSize)
# msg = "Message from Server {}".format(msgFromServer[0])

print("----------------------Msg Sent------------------")
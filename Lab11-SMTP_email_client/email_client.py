from socket import *
import base64

# Choose a mail server (e.g. Google mail server)
# and call it mailserver
smtp_server = "smtp.qq.com"
smtp_port = 587
mailserver = (smtp_server, smtp_port)  # Fill in start #Fill in end
# Sender and receiver
# Fill in start
sender = "*****"
receiver = "*****"
# Fill in end

# Auth information (Encode with base64)
# Fill in start
user_account = "*****"
user_password = "*****"  # QQ邮箱开启SMTP服务后的授权码
encrypted_account = base64.b64encode(user_account.encode())
encrypted_password = base64.b64encode(user_password.encode())
# Fill in end

# Transfer text and image
with open("WLOP 结婚照.jpg", "rb") as imgfile:
    image_data = base64.b64encode(imgfile.read())
text_data = base64.b64encode("I love computer networks!".encode())

subject = "E-mail with pics supported! Lab done!"
# 提前处理好msg，登录完成后可以直接发送节省时间
msg = 'FROM: ' + sender + '\r\n'
msg += 'TO: ' + receiver + '\r\n'
msg += 'SUBJECT: ' + subject + '\r\n'
# msg += '\r\n'
# msg += 'I love computer networks!\r\n'
# msg = msg.encode()

# 使用MIME
msg += 'Content-Type:multipart/related; boundary="----=_NextPart_00_KH"\r\n'
msg += 'MIME-Version: 1.0\r\n'
msg += '\r\n'
msg = msg.encode()

msg += '------=_NextPart_00_KH\r\n'.encode()
msg += 'Content-Type: text/plain; charset=gb2312"\r\n'.encode()
msg += 'Content-Transfer-Encoding: base64\r\n'.encode()
msg += '\r\n'.encode()
msg += text_data
msg += '\r\n'.encode()

msg += '------=_NextPart_00_KH\r\n'.encode()
msg += 'Content-Type: image/jpeg; name="WLOP 结婚照.jpg"\r\n'.encode()
msg += 'Content-Transfer-Encoding: base64\r\n'.encode()
msg += '\r\n'.encode()
msg += image_data + b'\r\n'
msg += '\r\n'.encode()

msg += '------=_NextPart_00_KH--\r\n'.encode()
# msg处理完成

# Create socket called clientSocket
# and establish a TCP connection with mailserver
# Fill in start
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect(mailserver)
# Fill in end
recv = clientSocket.recv(1024).decode()
print(recv)
if recv[:3] != '220':
    print('220 reply not received from server.')

# Send HELO command and print server response.
heloCommand = 'HELO KethersHao\r\n'
clientSocket.send(heloCommand.encode())
recv1 = clientSocket.recv(1024).decode()
print(recv1)
if recv1[:3] != '250':
    print('250 reply not received from server.')

# Auth login
# Fill in start
loginCommand = "AUTH LOGIN\r\n"
clientSocket.send(loginCommand.encode())
recvAuthCmd = clientSocket.recv(1024).decode()
print(recvAuthCmd)

accountCommand = encrypted_account + b'\r\n'
clientSocket.send(accountCommand)
recvAccCmd = clientSocket.recv(1024).decode()
print(recvAccCmd)

passwordCommand = encrypted_password + b'\r\n'
clientSocket.send(passwordCommand)
recvPassCmd = clientSocket.recv(1024).decode()
print(recvPassCmd)
# Fill in end

# Send MAIL FROM command and print server response.
# Fill in start
fromCommand = "MAIL FROM: <" + sender + ">\r\n"  # 发件人邮箱地址
clientSocket.send(fromCommand.encode())
recv2 = clientSocket.recv(1024).decode()
print(recv2)
# Fill in end

# Send RCPT TO command and print server response.
# Fill in start
toCommand = "RCPT TO: <" + receiver + ">\r\n"  # 收件人邮箱地址
clientSocket.send(toCommand.encode())
recv3 = clientSocket.recv(1024).decode()
print(recv3)
# Fill in end

# Send DATA command and print server response.
# Fill in start
dataCommand = "DATA\r\n"
clientSocket.send(dataCommand.encode())
recv4 = clientSocket.recv(1024).decode()
print(recv4)
# Fill in end

# Send message data.
# Fill in start
clientSocket.send(msg)
# Fill in end

# Message ends with a single period.
# Fill in start
endmsg = "\r\n.\r\n"
clientSocket.send(endmsg.encode())
# Fill in end

# Send QUIT command and get server response.
# Fill in start
quitCommand = "QUIT\r\n"
clientSocket.send(quitCommand.encode())
recv5 = clientSocket.recv(1024).decode()
print(recv5)
# Fill in end

# Close connection
# Fill in start
clientSocket.close()
# Fill in end

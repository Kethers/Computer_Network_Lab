from socket import *
from pip._vendor.distlib.compat import raw_input

# serverName = '192.168.31.229'     # 同一个路由器网络环境下的测试
serverName = '127.0.0.1'
serverPort = 60000

input_cmd_file = raw_input('Input like Command Filename:\r\n')
input_args_num = len(input_cmd_file.split())
if input_args_num == 2:
    command = input_cmd_file.split()[0]
    filename = input_cmd_file.split()[1]
else:
    command = 0
    filename = 0
    print("filename error: filename needed.")

# establish TCP connection if command is correct
clientSocket = socket(AF_INET, SOCK_STREAM)
if (command == 'download') or (command == 'upload'):    # 命令正确则创建连接
    clientSocket.connect((serverName, serverPort))
    recv = clientSocket.recv(1024).decode()
    print(recv)
elif command != 0:  # 命令输入错误的提示
    print("command error: plz check your command, only support upload and download")

if command == 'download':
    clientSocket.send(input_cmd_file.encode())
    input_data = clientSocket.recv(1024)
    if input_data[:5] != 'error'.encode():  # 返回的不是错误提示，则接收二进制文件
        input_file = open(f'client_files/{filename}', 'wb')
        input_data = input_data
        while input_data:
            input_file.write(input_data)
            input_data = clientSocket.recv(1024)
        input_file.close()
        print("download complete.")

    else:
        print(input_data.decode())

elif command == 'upload':
    try:
        f = open(f'client_files/{filename}', 'rb')
        output_data = f.read()
        f.close()
        clientSocket.send(input_cmd_file.encode())
        recv = clientSocket.recv(1024)
        if recv[:5] != 'error'.encode():    # 返回的不是错误提示，则发送二进制文件
            clientSocket.send(output_data)
            print("upload complete.")
        else:
            print(recv.decode())

    except IOError:
        print("upload error: file does not exist.")

# Close TCP connection
clientSocket.close()

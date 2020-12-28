# import socket module
from socket import *
import sys  # in order to terminate the program
import threading


def webProcess(connectionSocket):
    # 打印线程号，并发送线程号信息给客户端
    thread_id = threading.currentThread().ident
    connection_msg = f"Connection established, thread {thread_id} is serving for you."
    connectionSocket.send(connection_msg.encode())
    print(f'current thread id is {thread_id}')

    message = connectionSocket.recv(1024).decode()  # Fill in start # Fill in end
    command = message.split()[0]
    filename = message.split()[1]
    if command == 'download':
        try:
            f = open(f'server_files/{filename}', 'rb')  # 若文件不存在则跳转到异常处理
            outputdata = f.read()
            f.close()
            # Send the content of the requested file to the client
            connectionSocket.send(outputdata)
            print(f"download {filename} complete.")

        except IOError:  # 要下载的文件不存在
            # Send response message for file not found
            # Fill in start
            print("download error: file does not exist")
            download_err = "error: fail to download the file because the file does not exist"
            connectionSocket.send(download_err.encode())
            # Fill in end

    elif command == 'upload':
        try:
            f = open(f'server_files/{filename}', 'rb')  # 要上传的文件已经存在，不允许覆写
            print("upload error: file already exists")
            f.close()
            error_mag = "error: file already exists and can't be overwritten"
            connectionSocket.send(error_mag.encode())

        except IOError:  # 要上传的文件并不存在，则发送回复，接收文件
            connectionSocket.send('you can upload your file now'.encode())
            f = open(f'server_files/{filename}', 'wb')
            inputdata = connectionSocket.recv(1024)
            while inputdata:
                f.write(inputdata)
                inputdata = connectionSocket.recv(1024)

            f.close()
            print(f"upload {filename} complete.")

    else:  # 这在客户端判断过了，应该根本用不上
        invalid_msg = "invalid command, only support upload and download"
        connectionSocket.send(invalid_msg.encode())

    # Close client socket
    # Fill in start
    print('')   # 换行让服务器的输出信息好看一点
    connectionSocket.close()
    # Fill in end


serverSocket = socket(AF_INET, SOCK_STREAM)

# Prepare a server socket
# Fill in start
serverPort = 60000
serverSocket.bind(('', serverPort))
serverSocket.listen(10)
# Fill in end

while True:
    # Establish the connection
    print('Ready to serve...')
    connectionSocket, addr = serverSocket.accept()  # Fill in start # Fill in end
    thread = threading.Thread(target=webProcess, args=(connectionSocket,))
    thread.start()

serverSocket.close()
sys.exit()  # Terminate the program after sending the corresponding data

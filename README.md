# Intro. to Network programming
Here are the labs of INP course (2022/9). Below are descriptions of what to do for each lab, and the directory includes the codes and some utilities for the test.
## Purpose
This course contains the basic knowledge of socket, TCP, UDP and IP, etc. The most important is to learn how to write a network program such as Domain Name Server or a simple chatroom. Also, I've learned lots of knowledges of C/C++ and linux, which helps us to find out the potential problem in the network and the system.

---
## Lab1 - Find the Magic Packet
### Purpose
To get familiar with ```tcpdump``` and ```wireshark``` and understand how packets are transfered on the internet.
### Content
Use ```nc``` command to connect to the lab server,then the server would send lots of packets to us. We have to use ```tcpdump``` to record the packets (save as a .pcap file) and write a script (or a program) to find the special packet which contains a flag.

Hint: use wireshark GUI can help you find the special pattern.

---
## Lab2 - Unpack the pak file
### Purpose
To know how to deal with a binary file given the format and how to manipluate files (create, read, write) in C/C++.
### Content
The pak file is a self-defined compressed file, it contains the header, the file entry and the file content. Unpack the file when the checksum is correct.

- File format: contain 4 sections

    | Header | File Entries | File Names | File Contents |
    | ------ | ------------ | ---------- | ------------- |

- Header format:
    | Magic <br> (32 bits)| File Names offset <br> (32 bits)| File Data offset <br> (32 bits)| Num of Files <br> (32 bits)|
    | :-----: | :------------: | :----------: | :-----------: |

    **Note**: The offset is the start of the section. For example, the File Names offset is the start of File Name section.

- File Entry format:

    | File Name offset <br> (32 bits)| File Size <br> (32 bits)| File Data offset <br> (32 bits)| Checksum <br> (64 bits)|
    | :-----: | :------------: | :----------: | :-----------: |



    **Note**: 
The File Name offset is the offset to the start of File Name section in the pak file.
To get the real checksum, cut the content into 8 bytes then XOR.
To get the correct File Size and Checksum, turn them into big endian.

If we get unpack the .pak file correctly, there will be a file named "checker"

Use the command to add the permission of execution to "checker" and run.
```
sudo chmod +x checker
./checker
```
Then we'll see the sucess message.

---

## Lab3.1 - TCP Client
### Purpose
To learn creating a socket, establishing a connection and interacting with a remote server.
### Content
We write a TCP client to connect to the lab server. Send "\GO" to the server, and it would send messages back. We have to compute the size of data (in bytes) sent from the server.

## Lab3.2 - Traffic Shaper
### Purpose
To know how to control the transmission rate at a given value.
### Content
We have to come up with a method to send data to the lab server at the given rate. Use ```tcpdump``` to record the flow (save as a .pcap file) and open it with ```wireshark``` to see the transmission flow so we know the stablility of the method.

### Demonstration
Screenshot of the result in wireshark: at rate of 1.6KB/s

![](https://i.imgur.com/ifbNOsP.png)

**Note**: you can see the graph through **I/O graphics** in **statistics**.

---

## Lab4 - nkat
### Purpose
To learn the fundamentals and details of fork(), dup() and execvp(). Also, we can learn a simple way for network server to provides its services to clients.
### Content
We have to write a server that provides a given command such as ```ls``` to whom connects to the server.

To start the server
```
./nkat <port> <command>
```
The client can use ```nc``` to connect to the server
```
nc <ip addr> <port>
```

### Demonstration
If we set the ```date``` command as the service, the client will get the result of "date" command on the server machine while connected.

- Server:

    ![](https://i.imgur.com/lQEILwj.png)

- Client:

    ![](https://i.imgur.com/eKzIXol.png)


---

## Lab5 - Simple Chatroom Server
### Purpose
To know how a chatroom works and how a server handles a huge amount of clients using poll(). 
### Content
We write a chatroom server to provide clients to chat in their terminal. (TCP)
Below are the details.

Server:
```
./chatserver <port>
```
Client:
```
nc <ip addr> <port>
```
The chatroom have to provide clients these commands:
- ```/name <name>``` : change his/her name in the chatroom to \<name\>
- ```/who``` : see all online users

While a user join/leave the chatroom, the server have to broadcast welcome/leaving message to others.

### Demonstration
While a user join, the server will give him a default name (random from aa.h). 

**1. There were 2 users joining the chatroom, and the second user left the chatroom.**

- Message shown on server:    
    ![](https://i.imgur.com/M0Fiil2.png)

- Message shown on 1st user:
    ![](https://i.imgur.com/pvNVCIW.png)

    
**2. A new user join the chatroom and said "Hello"**

- Message shown on 1st user:
    ![](https://i.imgur.com/31QBmac.png)


**3. If the 3rd user change his name:**

- Message shown on 1st user:
    ![](https://i.imgur.com/Qovrml0.png)

- Message shown on 3rd user:
    ![](https://i.imgur.com/YzE6aIF.png)


**4. the 1st user use /who to see the online users**
    
- Message shown on 1st user:    
    ![](https://i.imgur.com/zqgAswt.png)
    
---
    
## Lab6 - Reach the Limit Rate
### Purpose
To know how a server handles connections of 2 or more ports. Also, learn to achieve a high transmission rate in a rate limited environment.
### Content
We have to implement a server and a client program. (TCP)
The server program have to listen to 2 port, one is the command channel while the other is the data sink channel.

- **Command Channel**: clients can send these command to this channel
    - ```/reset``` : reset the num of received bytes to zero
    - ```/report``` : see the current data rate
    - ```/ping``` : ask the server to response "PONG"
    - ```/clients``` : request the current num of clients
- **Data Sink Channel**: receive the data sent by clients and count the num of received bytes

The client have to connect to 2 ports of the server, and send data to the server as quickly as possible.
The environment is set a limit tranmission rate, all we have to do is to find a method that the client can reach the highest data rate.

Start the server:
```
/server <port>
```
There's a test.sh in the lab6 directory, you can use it to get the result.


**Note**: If you want to use test.sh to test, you have to install ```tc``` tool first.
```
sudo apt-get install iproute2
```

---

## Lab7 - CTF Challenge
### Purpose
We have learned 2 properties of functions, re-entrant, and thread-safe in this course. This lab helped us to think about what potential problems may be caused if programmers didn't fully understand the mechanism and those properties.
### Content
There're 3 sample code provied. Complie and run each code file as the server, then interact with it. See the code to know how the server works.
- oracle_p1.cpp
- oracle_p2.cpp
- webcrawler.cpp

We have to interact with the server (in terminal) to get the flag.

**Note**: See the **report.pdf** for solutions.

---
    
## Lab8 - Robust UDP Challenge
### Purpose
Knowing the behavior of UDP that it does **NOT** check the arrival of the packet and re-transmit while failed, We learn to guarantee the transmission success.
### Content
We have to implement a server and a client. (UDP)
The server has files in the given path, it has to transmit those fils to the client, and the client has to save those files in the given path. (They are in different machine)
The network environment is extremely terrible that the lost rate are up to 80%.
We have to find a method to guarantee that all files can be transmitted correctly and reduce the needed time as possible.

- Server

    ```
    ./server <path/to/files> <num/of/files> <port>
    ```
- Client
    ```
    ./client <path/to/files> <num/of/file> <port> <ip addr>
    ```

Our method transmitted 1000 files in 23 seconds. 
If you want to try this lab, here's the link.
[Robust UDP Challenge](https://zoolab-org.github.io/lab_robust_udp/)

---

## Lab9 - Sudoku
### Purpose
To learn how programs communicate using unix domain socket.
### Content
We have to write a program that creates a unix domain socket with the given path and use the domain socket to interact with the lab server.
The main task is to fill in the sudoku table.
# About
This project is a university assignment where we were called to implement an application for file sharing.Server and client are multi-threaded and connected with TCP. Every client who joins the app shares their 'input' folder with every other online client and downloads other's 'input' folder. 
# Compiler and Run

- Server:
    - cd server
    - make 
    - ./server -p [port]
    
- Client:
    - cd client
    - make
    - cd [client folder]
    - ./client -d [input-folder] -sip [server-ip] -sp [server-port] -p [client-port] -w [num-of-threads] -b [buffer-size]
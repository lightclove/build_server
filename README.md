Client-server application for building/compiling c-sources on the server-side with returning of the result of building/compilation back to the client.
-- Done --
0. The server listens on the given port (by default, port 1234, can be set at startup through the -p option) and accepts requests from clients (in an infinite loop).
You can pass the server address and port to the client
(by default 127.0.0.1 1234, controlled by the -d and -p options), as well as the file path.
The client should send the given file to the server, before the file there is a heading of the form "File <filename> \ n" (filename is the file name).
The server should compile the file with the C compiler and in response send a compilation verdict (Result: OK / Fail),
if Fail - an error log.
  
@ToDo
1. Add the ability to compile multiple files.
File paths are passed to the client. The client sends these files to the server with the heading "Number: <number of files> \ n".
The files themselves follow, each with a header of "File <filename> \ n".
  
@ToDo
2. Add the ability to optionally transfer the Makefile.
The client has the option -m <file path>, when transferring the cli
  nt uses the header "Makefile: <filename> \ n".
In this case, the server should use make with this Makefile.
  
@ToDo
3. Add the ability to remotely update the server on the fly.
The option for the client is -u, the client adds the heading "Upgrade \ n" to the beginning and then the files as usual
(number, files themselves, makefile if given).
The server should compile as usual, and if everything is successful, then replace your process with a new version.
If compilation failed, send an error to the client in response.

@ToDo
4. Add the signal processing SIGUSR1 to the server.
Upon receipt of this signal, the server should terminate all connections, and then continue to work.
For debugging, you can start the client with a large file and send a signal to the server during the transfer.

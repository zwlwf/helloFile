# helloFile
## 
## Files description
1. client.c : client for Linux
2. server.c : server for Linux
3. client_win.c : client for Windows

## Usage
```bash
make
```

Run in server
```
nohup path/to/helloFile_server &
```

Run in client
```
alias push='path/to/helloFile push'
alias pull='path/to/helloFile pull'
push yourFilesInLocalpath # send files
pull  # receive files
```

Note: the server address used by clients is designed in config.h file and compiled into executive file so that we can run client everywhere we want.

## test/python
This shows an example of python, that can pull/push data between processes on different hosts.

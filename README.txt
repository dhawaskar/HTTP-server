HTTP web browser implementation in c.
Features implemented
	1. GET command
	2. POST command
	3. Persistent pipelining
	4. Error handling for following case
		a.400: 400 Bad Request
		b.404: Not Found Reason URL does not exist
		c.501: Not Implemented
		d.500: Internal Server Error

This project has the following directory

1.server.c
2.Makefile
3.README
4.www/ 

####	Steps to Compile and Run ####
Compile:
1. make clean all.
2. server code is generated.

Run:
1../server: will give the port number and note the ip address as well
2. open browser and run localhost:<port_number>/<file_name>
3. See the files you want to
4. To run pipelining and post command use the following sample commands from onother terminal
	(echo -en "GET /1.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\nGET /2.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"; sleep 10) | telnet 127.0.0.1 5001
	(echo -en "GET /1.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"; sleep 10) | telnet 127.0.0.1 5001
	(echo -en "GET /1.html HTTP/1.1\r\nHost: localhost\r\n\r\n"; sleep 10) | telnet 127.0.0.1 5001
	(echo -en "POST /1.html HTTP/1.1\r\nHost: localhost\r\n\r\nSANDESH\r\nPOST /2.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\nSANKETH"; sleep 10) | telnet 127.0.0.1 5001
	(echo -en "POST /1.html HTTP/1.1\r\nHost: localhost\r\n\r\nSANDESH"; sleep 10) | telnet 127.0.0.1 5001
	(echo -en "POST /2.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\nSANKETH"; sleep 10) | telnet 127.0.0.1 5001
	(echo -en "POST /1.html HTTP/1.1\r\nHost: localhost\r\n\r\nSANDESH\r\nGET /2.html HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"; sleep 10) | telnet 127.0.0.1 5001

#####################################

Assumptions made:
1. www/index.html is taken by defualt if file name is not mentioned
2. Each request handled in parallel with fork created chile
3. timer value will decrement once the server handles client request
4. pipeline is achived using structure that stores the comman,filename,HTTP version,content type,data to be posted
5. All the requests are handled in the order of arrival
######################################




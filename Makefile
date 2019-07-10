http_server:http_server.c user_method.h
	sudo gcc -o http_server http_server.c
clean:
	sudo rm -rf ./http_server ./*~ 


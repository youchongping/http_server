# http_server  
this is just a simple httpd .   
-------------------------------------       
---modify:  
	modify" server_addr.sin_addr.s_addr = inet_addr("172.16.0.12"); "  
	as your actual ip in http_srver.c  
---make:  
	make  
---run:  
	sudo ./http_server  
---test:  
	0.enter the host:port in any browser,  
	will call the GET method defult ,   
	and will display "take easy,everthing is ok" in browser.   

	1.or you can user RESTclient  in firefox to test it.    


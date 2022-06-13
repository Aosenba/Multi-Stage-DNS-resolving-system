# Multi-Stage-DNS-resolving-system
A 2- stage DNS resolving system involving a proxy server

Instructions: 
1. Run the DNS server specified with a port no . eg "./dns_server 3000".
   The server will be ready to listen any request made by the proxy server
2. Run the Proxy server in another port no . eg "./proxy_server 4200". 
   The server will be ready to listen any request made by the proxy server
3. Run the client specified with its ip address and the proxy port no. eg  " client 127.0.0.1 4200"
   There can be two types of request:
   i). Type 1: You can directly specify the domain name with its type. eg " www.google.com 1 ".
   ii).  Type 2 : You can specify the domain ip adress with its type .  eg " 10.10.10.10 2".
4. If the proxy server cannot process the request made by the client, it gets redirected to the DNS server wiht the same request made by the client
   and on receiving  the Response Message  from  DNS  Server,  the  proxy  server  forwards the  response  back  to the  client.
   The response can be of 2 types:
   i). Type 3 :   the  proxy  server  must  update  its  cache  with  the  fresh information  using  FIFO  scheme.
   ii). type 4 :  the entry is not present in the server's database and hence responds with a error message.

## 模拟

telnet www.baidu.com 80

GET / HTTP/1.1
host: www.baidu.com

然后两次回车

HTTP/1.1 200 OK
Accept-Ranges: bytes
Cache-Control: no-cache
Connection: keep-alive
Content-Length: 14615
Content-Type: text/html

uri : http://www.baidu.com:80/page/xxx?id=10&v=125#fr

http  -- 协议

www.baidu.com, host 

80 port

/page/xxx  path 

id=10&v=20 param

fr fragment

## reference 
 
https://github.com/nodejs/http-parser/blob/master/http_parser.h

## ragel  -- parser

ragel -G2 -C http11_parser.rl -o http11_parser.cc
## mysql c api doc 

https://www.mysqlzh.com/api/35.html

## c api examples 

https://zetcode.com/db/mysqlc/

http://dev.cs.ovgu.de/db/mysql/C.html

https://github.com/hholzgra/connector-c-examples

## mysql 下载

## mysql 第一次设置密码

```bash
update mysql.user 
    set authentication_string=PASSWORD("123"), plugin="mysql_native_password" 
    where User='root' and Host='localhost';    
flush privileges;

quit
```

## mysql 连接

## 创建database

```bash
create DATABASE burger;
```

## 语句数据结构

## MYSQL_TIME   结构用于在两个方向上传输时间数据。

## MYSQL_STMT 预处理语句的处理程序

## MYSQL_BIND 用于语句 Importing(发送到服务器的数据值)和输出(从服务器返回的结果值)：



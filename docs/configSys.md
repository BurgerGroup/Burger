## 配置系统

如果我们程序没有任何配置文件时，这样的程序对外是全封闭的，一旦程序需要修改一些参数必须要修改程序代码本身并重新编译，这样很不好，所以要用配置文件，让程序出厂后还能根据需要进行必要的配置

我们采用ini文件进行一些参数的配置

INI file的后缀名也不一定是".ini"也可以是".cfg"，".conf ”或者是".txt"。

## 经典格式

parameters，sections和comments。

例子

```
; 通用配置,文件后缀.ini
[common]

application.directory = APPLICATION_PATH  "/application"
application.dispatcher.catchException = TRUE

; 数据库配置
resources.database.master.driver = "pdo_mysql"
resources.database.master.hostname = "127.0.0.1"
resources.database.master.port = 3306
resources.database.master.database = "database"
resources.database.master.username = "username"
resources.database.master.password = "password"
resources.database.master.charset = "UTF8"


; 生产环境配置
[product : common]

; 开发环境配置
[develop : common]

resources.database.slave.driver = "pdo_mysql"
resources.database.slave.hostname = "127.0.0.1"
resources.database.slave.port = 3306
resources.database.slave.database = "test"
resources.database.slave.username = "root"
resources.database.slave.password = "123456"
resources.database.slave.charset = "UTF8"

; 测试环境配置
[test : common]
```
## 为了各种数据库的兼容，先构建基本的抽象类

- ISQLData -- 用于各种数据操作

- ISQLUpdate  -- crud操作中增删改

主要是要有execute()去执行sql语句

- ISQLQuery -- 查

query语句查询

- IStmt -- 语句

核心的就是去bind参数

- ITransaction -- 事务

核心就是begin, commit rollback等

- IDB -- database base


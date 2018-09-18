# boostserver
## 基于boost：：asio 搭建的服务器
### 目前完成的功能
1. tlv协议  HEAD大小目前为4，前两个字节为消息id，接着两个字节为数据域消息长度。HEAD为小端存储，消息格式为MSGID+MSGLEN+DATA。
2. 粘包处理  
3. 异步收发数据  
4. 心跳检测  
5. 连接管理和异常处理  
6. 回调函数封装和注册  
7. 增加最大连接数限制，避免emfile，linux下可根据进程允许打开的最多描述符修改此数值。

### 配合客户端
配合客户端示例查看 [https://github.com/secondtonone1/boostclient](https://github.com/secondtonone1/boostclient)
  
### 接下来要实现
1. 消息体序列化，配合msgpack库  
2. python 接口嫁接，  
3. 消息加密
4. 实现websocket，和现有tcp 兼容



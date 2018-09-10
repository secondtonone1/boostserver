# boostserver
## 基于boost：：asio 搭建的服务器，目前完成了tlv协议，粘包处理，异步收发数据。
## 2018 9 7 完成心跳检测，连接管理。处理异常连接
## //tlv 形式，HEAD 四字节存储消息长度，MSGID 四字节存储消息类型id 
所以消息发送的格式为 HEAD+MSGID+data， 长度为4字节+4字节+数据长度。配合客户端示例查看 [https://github.com/secondtonone1/boostclient](https://github.com/secondtonone1/boostclient)
## 接下来要实现序列化，python 接口嫁接， 回调函数封装。


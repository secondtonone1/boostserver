# boostserver
## 基于boost：：asio 搭建的服务器
### 目前完成的功能
1. tlv协议  HEAD大小目前为8，前四个字节为消息id，接着四个字节为数据域消息长度。格式为MSGID+MSGLEN+DATA
2. 粘包处理  
3. 异步收发数据  
4. 心跳检测  
5. 连接管理和异常处理  
### 配合客户端
配合客户端示例查看 [https://github.com/secondtonone1/boostclient](https://github.com/secondtonone1/boostclient)
### 存在的问题
消息id最大目前设置为1700，超过1700时，windows可能会出现短暂休眠，不影响正常发包，linux没有出现该问题。  
为避免上述问题，将最大消息id设置为1700.  

### 接下来要实现序列化，python 接口嫁接， 回调函数封装。



# 第一章：网络和socket
#
## 1、网络设计模式
### C/S模式
+ 客户端/服务器
+ 优点：
	+ 协议选用灵活，可以自定协议
	+ 缓存数据
+ 缺点：
	+ 对用户安全造成威胁
	+ 开发工作量大，联合调试困难
+ 使用场景
	+ 数据量大
### B/S模式
+ 浏览器/服务器
+ 优点：
	+ 不需要安装客户端
	+ 开发工作量较小
	+ 跨平台
+ 缺点：
	+ 协议选用固定，必须支持http
	+ 不能缓存数据
+ 使用场景
	+ 数量小
## 2、网络分层模型
### OSI参考模型
### TCP/IP模型
+ 应用层
+ 传输层
+ 网络层
+ 网络接口层  
#  
+ 数据封装会由操作系统内核实现，用户只负责处理应用层的数据，和调用其他层协议提供的函数发送数据和接收数据
### NAT映射
+ 在与内网相连的路由器上将内网ip映射成不同的公网ip（用不同端口区分），这样即使多个内网ip相同，但是依然可以找到目的ip
### 打洞
+ 两台计算机A，B通过一个服务器S通信，A，B不知道对方的ip，当A，B有互相直接通信的需求时，需要“打洞”，A和B向S发送数据，S会获得A和B的ip，S将A的ip发给B，B的ip发给A，这样A和B就有了对方的ip
## 3、socket
+ ip地址：在网络中唯一标识一台主机
+ 端口号：在主机中唯一标识一个进程
+ ip+端口：在网络环境中唯一标识一个进程（socket）
#
+ Linux下的socket是一个伪文件，这个伪文件的fd指向两个缓冲区，一个是读缓冲区，一个是写缓冲区
+ 两个进程通过两个socket通信，每个socket的读缓冲区连接对方socket的写缓冲区
### 网络字节序
+ TCP/IP规定，网路数据流采用大端字节序，主机的字节序可能与网络字节序不同，所以需要主机字节序和网络字节序相互转换
+ 字节序转换函数：
	+ uint32_t htonl(uint32_t hostlong)
	+ uint16_t htons(uint16_t hostshort)
	+ uint32_t ntohl(uint32_t netlong)
	+ uint16_t ntohs(uint16_t netshort)
	+ h:host, to:XXX到XXX, n:network, l:32位整数, s:16位整数
+ htonl(INADDR_ANY) INADDR_ANY表示从当前系统网卡中找一个有效的ip地址
### ip地址转换
+ 原来，由点分十进制转换成unsigned int 再转换成网络字节序IP地址
+ 为了方便，直接从点分十进制到网络字节序IP地址
+ 函数：
	+ int inet_pton(int af, const char * src, void * dst)
		+ af:指定版本 AF_INET:IPv4 AF_INET6:IPv6
		+ src:点分十进制字符串 
		+ dst：传出的网络字节序IP地址 
	+ int inet_ntop(int af, const void * src, char * dst, socklen_t size)
		+ size:点分十进制字符串的大小 
### struct sockaddr_in和struct sockaddr
+ struct sockaddr属于古老的数据类型，包含：
	+ 16位地址类型
	+ 14字节地址数据
+ struct sockaddr_in在前者基础上做了细分：
	+ 16位地址类型
	+ 16位端口号
	+ 32位IP地址
	+ 8字节填充
+ struct sockaddr已经淘汰，被struct sockaddr_in取代，但是由于历史遗留问题，有些函数的参数类型是struct sockaddr,所以传参时需要强转,比如：
	+ bind( , (struct sockaddr*)&sockaddr_in); accept()
	+
	+ connect()
### struct sockaddr_in结构体成员
+ man 7 ip 可以查看
+ sa_family_t sin_family; //地址类型，AF_INET/AF_INET6
+ int_port_t sin_port; //端口号，ntohs()/htons()
+ struct in_addr sin_addr; //ip地址 
+ struct in_addr{
+ uint32_t s_addr; //htonl()/ntohl(), inet_pton()/inet_ntop()
+ }  
## 4、socket通信的函数
### 1>创建socket
+  int socket(int domain, int type, int protocol)
+  成功返回socket文件描述符，失败返回-1
+  参数：
	+  domain：地址类型
		+  AF_INET ipv4
		+  AF_INET6 ipv6
		+  AF_UNIX 本地套接字
	+ type:传输方式
		+ SOCK_STREAM 流式协议，默认TCP
		+ SOCK_DGRAM  报式协议，默认UDP
		+ SOCK_RAM ICMP
	+ protocol
		+ 传0表示使用默认协议
### 2>绑定ip和端口号
+ int bind(int sockfd, const struct sockaddr * addr, socklen_t addrlen)
+ 成功：0，失败-1
+ 参数：
	+ sockfd:socket文件描述符
	+ addr:IP地址+端口号
	+ addrlen:addr所指向的结构体的字节长度
+ bind()如果不调用，操作系统会自动分配IP地址和端口号 
### 3> 指定同一时刻连接请求上限数
+ 注意不是连接上限数，而是同时发起连接请求的客户上限数，listen指在三次握手开始时服务器的状态，当连接建立后，该客户不会占用这个数
+ int listen(int sockfd, int backlog)
+ 成功：0，失败：-1
+ 参数：
	+ sockfd:socket文件描述符
	+ backlog:指定请求连接上限数
### 4>服务端接收请求连接的客户socket
+ int accept(int sockfd, struct sockaddr * addr, socklen_t * addrlen)
+ 成功：返回请求连接的客户sockfd，失败：-1
+ 参数：
	+ sockfd：本机socket文件描述符（bind()listen()完的）
	+ addr：传出参数，传出请求连接的客户机地址信息
	+ addrlen 传入传输参数（函数内部需要获得addr的有效长度，然后再修改addr长度即接收到客户addr的长度，并传出） 
+ accept()阻塞等待客户端连接 
### 5>客户端向服务器请求建立连接
+ int connect(int sockfd, const struct sockaddr * addr, socklen addrlen)
+ 成功：0，失败：-1
+ 参数：
	+ sockfd:本机socket文件描述符
	+ addr:传入参数，传入请求建立连接的服务器的地址信息
	+ addrlen:传入参数，传入addr指向的结构体长度
## 5、C/S结构基本流程
+ 服务器
	+ socket()
	+ bind() （struct sockaddr_in addr，给addr赋值）
	+ listen()
	+ accept()
	+ 阻塞等待客户端发来的请求
+ 客户端
	+ socket()
	+ 这里可以bind(),也可以不bind()(系统分配IP和端口号)
	+ connect()
+ 建立连接成功
+ 客户端write(),服务器read(),服务器write(),客户端read() 
### nc命名
+ nc + ip + 端口号 直接访问服务器
### 客户端的socket和服务端接收socket
+ 客户端的socket：lfd和服务端accept返回的socket:cfd是两个不同的socket，但是两个socket通过IP和端口号产生联系
	+ lfd的读端连接cfd的写端，lfd的写端连接cfd的读端
+ 两个socket是双向流动，全双工，均为内核缓冲区
## 6、封装错误处理
+ 代码见cs_plus
+ 通过一个wrap.c 将使用的系统函数再次封装
	+ perr_exit()封装了错误处理和exit()
```
		void perr_exit(char * s){ 
			perror(s);
			exit(-1);
		}
```	 
+ 自定义函数，函数除将源系统函数的名首字母大写外，参数、返回值应该与系统函数完全一样，（因为man不区分大小写，这样依然可以查看源系统函数)
	+ 将系统函数的调用和出错处理封装在自定义函数中
+ 在其他.c中调用自定义函数而非调用源系统函数，这样可以省去出错处理的代码
### read返回值
+ 返回0时才表示数据真正读完
+ 返回-1：
	+ errno == EINTR 被信号中断，重新调用read
	+ errno == EAGIN(EWOULDBLOCK)非阻塞方式读且无数据，轮询
	+ errnp == 其他值， 出错
# 第二章：高并发服务器
## 1、多线程/进程版
### 多进程版
+ 每有一个客户端连接就创建一个子进程
	+  父进程关闭cfd，子进程关闭lfd
+ 客户端断开后，子进程退出，父进程通过捕捉子进程的SIGCHLD信号回收子进程
### 多线程版
+ 每有一个客户连接就创建一个线程
	+ 分离线程
+ 所有处理交给线程，主线程只负责连接
### 多进程/线程的服务器会带来很大的开销，并且上限也很小，无法支持大量客户端连接，可以用于少量客户端
#
## 2、TCP状态转换
### 主动发起请求一端
+ CLOSED
	+ 关闭状态
+ 应用：主动打开 && 发送：SYN -> SYN_SENT
	+ 主动打开状态
+ 接收：SYN，ACK && 发送：ACK -> ESTABLISHED
	+ 建立连接状态
+ 应用：close && 发送：FIN -> FIN_WAIT_1
+ 接收：ACK -> FIN_WAIT_2
	+ 半关闭状态
+ 接收： FIN && 发送：ACK -> TINE_WAIT
	+ 2MSL超时重传：如果接收请求端没有收到，它会认为它发的FIN对方没有收到，会重发FIN，TIME_WAIT状态就是为了等这个FIN，如果在2MSL时间内没有收到，则认为对方已经收到了ACK，就会进入CLOSE
### 被动接收一端
+ CLOSE
	+ 关闭状态
+ 应用：被动打开  -> LISTEN
+ 接收：SYN && 发送：SYN ACK -> SYN_RCVD
+ 接收：ACK -> ESTABLISHED
+ 接收：FIN && 发送： ACK -> CLOSE_WAIT
+ 应用：close && 发送：FIN -> LAST_ACK
+ 接收：ACK -> CLOSE 
### netstat -apn | grep 6666查看含有端口6666的进程状态
### 2MSL
+ Maximum Segment Lifetime 最大生存时间 
### 半关闭
+ close存在局限性
	+ 会直接关闭文件描述符，无法只关闭读/写
	+ 只能关闭一个文件描述符，如果dup2了多个文件描述符指向一个socket那么这种关闭是不完全的
+ shutdown
	+ int shutdown(int sockfd, int how)
	+ sockfd：需要关闭的文件描述符
	+ how：允许为shutdown操作选择以下几种方式
		+ SHUT_RD(0)：关闭sockfd上的读功能，该socket不在接收数据，任何当前socket接收的数据都会被无声丢弃掉
		+ SHUT_WR(1)：关闭socket写功能
		+ SHUT_RDWR(2)：关闭读写功能，相当于调用shutdown两次
	+ shutdown会关闭所有的socket文件描述符，而不是一个
	+ 实际上是由内核管理关闭socket的读/写缓冲区  
### 端口复用
+ 由于server端如果主动发起断开请求，会进入TIME_WAIT状态，占用的端口导致无法重启server
+ int setsockopt(int sockfd, int level, int optname, const void* optval, socklen_t optlen)
	+ 在socket()之后bind()之前调用
	+ 非常复杂的参数，详情查书
	+ int opt = 1；
	+ setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
+ 允许重用本地地址 
# 第三章：多路IO复用
+ 将accept和read请求的任务交给内核，当内核收到请求时会将请求交给server处理
	+ 避免了server阻塞
	+ 不需要创建很多线程
## 1、select
+ int select(int nfds, fd_set *readfds, fd_set * writefds, fd_set * exceptfds, struct timeval *timeout)
	+ nfds文件描述符表最后的文件描述符位置+1
	+ readfds 文件描述符是否可读
	+ writefds 文件描述符是否可写
	+ exceptfds 文件描述符是否异常
		+  fd_set 是一个位图
	+ timeout 监听的时间
		+ NULL永远等
		+ timeout成员均为0，检查后立刻返回
+ 成功 返回满足条件的事件个数 如 即监听fd1的读，又监听fd1的写，两个都满足条件，就返回2而不是返回1
+ 失败返回-1
+ void FD_ZERO(fd_set* set) 将set清空成0值
+ void FD_CLR(int fd, fd_set* set)将fd从set中移除
+ void FD_SET(int fd, fd_set *set)将fd添加到set中
+ void FD_ISSET(int fd, fd_set* set)判断fd是否在set中
+ select的readfds writefds exceptfds均为传入传出参数，在传入时选择监听那些文件描述符，传出后将发生事件的文件描述符位置为1，未发生的置为0
### select的作用
+ 指示内核等待多个事件中任何一个事件发生，并且仔仔有一个或多个事件发生或经历一段时间后唤醒它
+ select告知内核对哪些文件描述符感兴趣以及等待多长时间
	+ 不局限于socket
### select的缺点
+ 文件描述符存在上限1024，如果想改变，必须重新编译Unix内核
+ 每次查找文件描述符都要循环遍历0->1023即使只有少量描述符，可以自己维护一个数组解决此问题
+ 监听集合和满足监听条件的集合都是同一个集合，每次使用都会修改，监听集合，所以还需要保存原来的
### select1模型客户端1
+ 通过select解决了标准输入阻塞会忽略socket的消息的情况，但是出现了新的问题：
	+ 如果客户端在发送数据后，关闭，可能数据尚未被服务器处理完成，这样收到的数据就少了一些
	+ 解决方法：半关闭
### select2客户模型2
+ 相比1的优点
	+ 在标准输入关闭时，不会直接关闭进程，而是只关闭socket的写端，保留socket的读端，这样在读到最后socket读端没有数据后退出
	+ fgets只会buf-1个，其余都会存在于缓冲区中，在fgets调用后，新的while循环再次调用select等待新的工作，而不管缓冲区的剩余字符，而改用writen在读取错误时会报错，从而增强了程序健壮性
### select单监听服务器
+ 见select3
+ 原理
	+ 使用select函数等待读请求的发生，在收到请求后传给用户应用程序，对于读操作，会FD_ISSET serverfd是否处于fd_set中，如果是则accept
	+ 对于剩下的读请求，则进行处理和回传
	+ 使用自定义的一个数组增加和删除已经建立连接的客户端的socket和断开连接的客户端socket
+ 特点
	+ 用select监听请求,select第5个参数传NULL值是阻塞的
	+ accept和read不会阻塞
	+ 每次循环只会处理一个建立连接请求，剩余请求留到下次循环  
### 拒绝服务攻击
+ 如果一个客户端只发送了一个字节（没有换行符）然后进入睡眠，服务器会阻塞于下一个read调用，等待其余的数据
	+ 注意：readline需要读入换行符才停止，但是对于read等直接对于缓冲区的I/O不会出现此类拒绝服务攻击
+ 拒绝服务攻击：当服务器被单个客户阻塞而不服务其他客户时，就形成拒绝服务
+ 解决理念：服务器不能阻塞于单个客户
	+ 使用非阻塞I/O
	+ 让每个客户用单独的线程服务
	+ 设置I/O超时
## 2、poll 
+ int poll(struct pollfd *fds, nfds_t nfds, int timeout)
	+ fds 是一个struct pollfd的首地址
	+ nfds 数组里元素的个数
	+ timeout 等待时间返回
		+ -1 阻塞等
		+ 0 立刻返回
		+ >0 等待指定毫秒数，如果系统时间不够毫秒，向上取值
+ struct pollfd{
	+ int fd; 文件描述符
	+ short events； 监听的事件
		+ 常用值
		+ POLLIN 读
		+ POLLOUT 写
		+ POLLERR 出错
	+ short revents; 返回的事件
+ }
+ 失败：-1 定时器到时间没有文件描述符就绪：0 其他情况：返回就绪描述符个数，即revents != 0的个数
+ 特点
	+ 和select的思想一样，将监听请求的任务交给内核处理，但它是select的改进版
	+ 文件描述符上限可以突破1024
		+ ulimit -a查看open files
		+ cat /proc/sys/fs/file-max 查看硬件要求的打开的文件描述符上限
		+ sudo vi /etc/security/limits.conf 在# End of file 之前添加
			+ * soft nofile 5000
				+ 打开文件描述符下线
			+ * hard nofile 20000
				+ 上限
		+ 注销用户或重启系统启用设置
	+ 监听事件和返回事件的集合是分离的
	+ 如果监听数组中的struct pollfd成员fd是一个赋值，则POLL会忽略这个成员的events，返回时将它的revents成员的值置为0
	+ 不需要自定义一个文件描述符数组，而是使用struct pollfd
		+ pollfd的 events 只能监听一中方式，如果想监听多种，在数组中加入相同fd但是events不同的成员
		+ client数组的第0个值为服务端fd，和select的client数组不同，这个数组监听都是从1开始 
+ 缺点
	+ 如果数组中只有少数的fds，依然需要将数组遍历，而不是只遍历已经建立好连接的，依然没有改变select的的这个缺点
## 3、epoll
+ epoll是select/poll的增强版
	+ epoll获取事件时，无需遍历整个文件描述符集，而是只需要遍历被内核I/O事件异步唤醒而加入Ready队列的描述符
		+ 如果获取事件很多，select/poll的效率和epoll是一样的
	+ 可以修改文件描述符上限 
### epoll_create()
+ 创建监听事件的数据结构
+ int epoll_create(int size)
	+ size 告诉内核监听文件描述符的个数，跟内存大小有关
		+ 这个值是建议给内核的值，具体的值内核会做调整
	+ 返回值指向存在于内核的的红黑树的根节点，size即为红黑树的节点数
	+ 失败返回-1，设置errno
### epoll_ctl()
+ 控制epoll监听文件描述符的哪些事件
+ int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
	+ epfd epoll_create()函数的返回值
	+ op 可以对文件描述符的控制有哪些(对树的操作)
		+ EPOLL_CTL_ADD 增加节点
		+ EPOLL_CTL_MOD 修改节点
		+ EPOLL_CTL_DEL	删除节点
	+ fd 对哪个文件描述进行操作
	+ event 传递struct epoll_event类型的地址，监听事件的发生
### epoll_wait()
+ 开始监听
+ int epoll_wait(int epfd, struct epoll_event* events, int maxevents, int timeout)
	+ epfd epoll_create()返回值
	+ events struct epoll_event类型数组的首地址
		+ 传出参数，传出符合监听要求的文件描述数组
	+ maxevents events数组的容量
	+ timeout 等待时间
		+ -1 阻塞
		+ 0 立刻返回，非阻塞
		+ >0 指定等待时间后返回，毫秒
+ 成功：返回就绪的文件描述符个数，失败：-1，时间到没有就绪文件描述符返回0
### struct epoll_event
+ {
	+ uint32_t events;
	+ epoll_data_t data;
+ }
	+ event指的是监听的事件，一般为
		+ EPOLLIN 读
		+ EPOLLOUT 写
		+ EPOLLERR 出错
+ typedef union epoll_data{
	+  void *ptr;
	+  int fd;
	+  uint32_t u32;
	+  uint64_t u64;
+ }epoll_data_t;
### epoll原理
+ epoll需要两个数据结构，一个是内核维护的红黑树，一个是用户定义的数组
	+ 通过epoll_create()创建红黑树 
	+ 通过epoll_clt()将需要监听的事件存放于红黑树中
	+ 通过epoll)_wait()监听事件的发生并把发生的事件传入数组中
	+ 遍历数组，就可以根据数组结构体成员中的文件描述符来处理事件
### epoll的两种触发方式（工作模式）
+ 水平触发（LT） socket缓存有数据就触发 
	+ 低速工作模式 支持阻塞/非阻塞
	+ 默认工作模式，select和poll也是这种模式
	+ 保证了数据的完整输出
+ 边缘触发（ET） socket缓存有数据到来的那次才触发
	+ 高速工作模式，只支持非阻塞
	+ 不能保证数据的完整
### epoll非阻塞I/O模型
+ 使用边缘触发+非阻塞socket
	+ 相比只有边缘触发，可以一次性读完数据
	+ 相比水平触发，可以减少epoll_wait()的调用次数
## 4、epoll反应堆模型
+ 基于epoll非阻塞I/O模型
+ 是libevent的核心
	+ libevent是一个运用广泛的跨平台的第三方库
### epoll反应堆和传统epoll的区别
+ epoll工作流程
	+ 服务器--监听--监听到fd--可读--epoll返回--read--处理buf--write--epoll继续监听
+ epoll反应堆工作流程
	+ 服务器--监听--监听到fd--可读--epoll返回--read--fd从红黑树上删除--设置监听fd“写事件”（fd插入到红黑树）--处理buf--等待epoll_wait()返回--回写客户端--fd从红黑树删除--设计监听fd读事件（fd插入到红黑树）--epll_wait()
+ epoll反应堆的特点在于它的写也是由epoll监听的，这样无论读写都不会阻塞
+ epoll使用的是event.data.fd
+ epoll反应堆使用的是event.data.ptr,ptr指向一个结构体
	+ 这个结构体是自己定义的，但是一定要包含回调函数
``` 
struct myevents{
		int fd; //文件描述符
		int events; //对应的事件
		void* arg; //泛型参数
		void(* call_back)(int fd, int events, void* arg); //回调函数
		int status; //文件描述符是否在红黑树上？在：1，不在：0
		char buf[BUFLEN];
		int len; //buf的长度
		long last_active; //节点加入到红黑树上的时间，为了防止一个文件描述符保持连接但不发数据
		//当超过一定时间都没有更新这个值的话，就会从红黑树中删除
	 }；
```
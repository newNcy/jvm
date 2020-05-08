# jvm
# 环境
+ linux 其他平台没试过

# 依赖
+ libzip 加载jar里的class
+ libffi 调用native方法

# 现有的功能
+ 类加载，执行
+ 异常

# 还需要
+ 多线程
+ 垃圾回收
+ 更加规范的结构和流程

# 输出
+ log.h/cpp里可以关掉debug的输出，直接返回就行了
+ log::bytecode 打印每条字节码的类名，方法名，字节码地址，参数,以及执行完后的栈和局部变量情况


### 运行输出
![stdout](https://github.com/newNcy/jvm/blob/master/screenshot/%24EN9WYEQY3%40GTQ7QZ17%7BN3T.png)
![stderr for debug](https://github.com/newNcy/jvm/blob/master/screenshot/2.png)
左边是当前运行的字节码具体信息，右边是栈和局部变量

# 开发记录
## 多线程支持
java线程在开始运行前是个普通的java对象，语言标准里貌似有线程状态啥的，这是java层面上的细节，现在在jvm层先略过，首要目标是能跑起来多线程代码。



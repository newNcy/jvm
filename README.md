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

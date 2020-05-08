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
java代码中调用Thread及其子类的start方法时，会在native层调用java_lang_Thread_start0方法。所以得通过实现java_lang_Thread_start0来实现创建用户线程。
在java_lang_Thread_start0中，可以拿到该Thread类的java对象，使用这个对象来创建一个cpp的thread类（我们自己的），然后使用std::thread创建真正能并行的线程
```c++
NATIVE void java_lang_Thread_start0(environment * env, jreference cls)
{   
    auto t = env->get_vm()->new_thread(cls);
    std::thread(&thread::start, t).detach();
}
```
由于之前没有多线程，上述函数空实现，所以java类库创建的守护线程其实都没有跑起来，实现这个函数之后发现守护线程直接while(true)，虚拟机就不会退出。
针对这个问题，可以在调用主函数之后轮询线程，用户线程结束就清除，如果没有用户线程了就退出。
```c++
...
main_thread->call(main_method, jargs);
bool all_done = false;
while (!all_done) {
  all_done = true;
  for (auto it = threads.begin() ; it != threads.end(); it ++) {
     if ((*it)->is_daemon()) continue;
     if ((*it)->finish) {
          it = threads.erase(it);
     }else {
          all_done = false;
     }
  }
 }
...
```
运行结果：
![三个线程分别打印自己名字（类名:序号）](https://github.com/newNcy/jvm/blob/master/screenshot/3.png)
结果乱七八糟，而且有时候还会出现奇奇怪怪的异常。


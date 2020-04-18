
#2020-4-17 抛出java/lang/ClassCastException
溯源
1.[java/util/concurrent/atomic/AtomicReferenceFieldUpdater$AtomicReferenceFieldUpdaterImpl.<init>::00122] 0xa5 if_acmpeq 288 244
2.288 244 分别从locals 2 6处取出
3.6处引用来自java/lang/reflect/Field.getType:()Ljava/lang/Class; this为5处变量
4.5处引用来自java/security/AccessController.doPrivileged:(Ljava/security/PrivilegedExceptionAction;)Ljava/lang/Object; 
....瞎看一会找到原因了，
java_lang_Class_getDeclaredFields0里返回的反射field的type字段置成所属的类了，应该是该属性的type(Class对象)

#2020-4-19 sun/reflect/generics/parser/SignatureParser.advance方法出现assert失败
根据名称推断是discriptor解析失败了
查看源码为
```java
private void advance(){
	assert(index <= input.length);
	index++;
}
```
应该是解析到最后一个字符但没有解析成功，可能性：
1.给定描述字符串有问题，整个错的
2.jvm使用的字符串结束不是\0，所以加载之后使用的名称结尾有误，不全或者多出
3.字符串的解析使用到了诸如换行符之类的需要在Properties中设置的

思路:
1.看Properties中各个属性作用
2.把分析的unicode string打印出来


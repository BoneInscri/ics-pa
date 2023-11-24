1. target

命令行转化为一系列的指令序列！

==每个指令序列前面是一个tab！==

```
target: [dependencies]
    ``<command>
    ``<command 2>
    ``...
```

```
clean:
    ``rm -f *.o core
```

这里如果加了一个依赖，那么这个target只有在依赖发生改变的时候才会执行！





2. default target



3. 多用宏

```
CC：C compiler C编译器
```

有默认的一些宏定义，环境变量的宏会加载到Makefile中！

```
make -p
```

查看



4. $@

target的名字

```
client: client.c
	$(CC) client.c -o $@
server: server.c
   	$(CC) server.c -o $@
```



5. $?

```
client: client.c
        $(CC) $? -o $@
server: server.c
        $(CC) $? -o $@
```



6. $^

所有的依赖项（去重）

```
# print the source to the screen
viewsource: client.c server.c
        less $^
```



7. $+

所有的依赖项（不去重）

```
# print the source to the screen
viewsource: client.c server.c
        less $+
```



8. $<

only need the first dependency!






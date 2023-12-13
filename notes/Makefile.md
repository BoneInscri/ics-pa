https://www.gnu.org/software/make/manual/make.pdf



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



9. 条件判断

```shell
ifeq (...)
	...
endif
```



10. wildcard

4.3.3 The Function wildcard

One use of the wildcard function is to get a list of all the C source files in a directory, like this:

```shell
$(wildcard *.c)
```



比如下面这个语句：

```shell
ifeq ($(wildcard $(NEMU_HOME)/src/nemu-main.c),)
  $(error NEMU_HOME=$(NEMU_HOME) is not a NEMU repo)
endif
```

就是检查 $(NEMU_HOME)/src/nemu-main.c是否存在，如果不存在，就报错，即抛出error！



11. **-include**

Include another makefile

If you want make to simply ignore a makefile which does not exist or cannot be remade, with no error message, use the -include directive instead of include, like this: **-include filenames...**

This acts like include in every way **except that there is no error (not even a warning)** if any of the filenames (or any prerequisites of any of the filenames) do not exist or cannot be remade.

For compatibility with some other make implementations, sinclude is another name for -include.



比较一下include

The include **directive** tells make to **suspend** reading the current makefile and read one or more other makefiles before continuing. 

The directive is a line in the makefile that looks like this: include filenames... 

filenames can contain shell file name patterns. 

**If filenames is empty, nothing is included and no error is printed.**





12. **patsubst**

We can change the list of C source files into a list of object files by replacing the ‘.c’ suffix with ‘.o’ in the result, like this: 

```shell
$(patsubst %.c,%.o,$(wildcard *.c))
```



比如

```shell
remove_quote = $(patsubst "%",%,$(1))
```



13. **call**

The call function is unique in that **it can be used to create new parameterized functions**. You can write a complex expression as the value of a variable, then use call to expand it with different values.

The syntax of the call function is: 

```shell
$(call variable,param,param,...)
```

When make expands this function, it assigns each param to temporary variables $(1), $(2), etc



结合一下就可以写出一个移除引号的函数。

```shell
GUEST_ISA ?= $(call remove_quote,$(CONFIG_ISA))
ENGINE ?= $(call remove_quote,$(CONFIG_ENGINE))
NAME    = $(GUEST_ISA)-nemu-$(ENGINE)
```

NAME = rsicv64-nemu-interpreter



14. **在MAKEFILE中使用shell指令**

```shell
FILELIST_MK = $(shell find ./src -name "filelist.mk")
```

就是找到所有src目录下名字为 filelist.mk的文件

Here are some examples of the use of the shell function:

```shell
contents := $(shell cat foo)
files := $(shell echo *.c)
export HI = $(shell echo hi)
export PATH = $(shell echo /usr/local/bin:$$PATH)
```



```shell
$(shell command) 
```

Execute a shell command and return its output



15. **目录黑名单和源文件黑名单**

```shell
DIRS-BLACKLIST-y += $(DIRS-BLACKLIST)
SRCS-BLACKLIST-y += $(SRCS-BLACKLIST) $(shell find $(DIRS-BLACKLIST-y) -name "*.c")
```



DIRS-BLACKLIST:

```shell
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb
```

SRCS-BLACKLIST:

```shell
SRCS-BLACKLIST-$(CONFIG_TARGET_AM) += src/device/alarm.c
```



16. **filter-out**

```shell
$(filter-out pattern...,text) 
```

Select words in text that **do not match any of the pattern words**



17. **CC**

````shell
gcc 
````

表示编译用的编译器



18. **CC_OPT**

```shell
CONFIG_CC_OPT="-O2"
```



19. **if**

```shell
$(if condition,then-part[,else-part])
```

Evaluate the condition condition; if it’s non-empty substitute the expansion of the then-part otherwise substitute the expansion of the else-part.



20. **CFLAGS**

基本就表示complier flags ，即编译选项。





21. **带颜色输出**

```shell
COLOR_RED := $(shell echo "\033[1;31m")
COLOR_END := $(shell echo "\033[0m")
$(warning $(COLOR_RED)Warning: .config does not exists!$(COLOR_END))
```



22. **直接用@符号！**

```shell
@$(MAKE)
```



23. **-s**

the -s option to make is needed to silence messages about entering subdirectories



24. `.PHONY`

The prerequisites of the special target 

.PHONY are considered to be phony targets. 

When it is time to consider such a target, make will run its recipe unconditionally, regardless of whether a file with that name exists or what its last-modification time is. 

**Targets that do not refer to files but are just actions are called phony targets**



25. **dir**

```shell
dir := /foo/bar # directory to put the frobs in
```

```shell
$(dir names...)
```

Extract the directory part of each file name.





26. **basename**

```shell
$(basename names...)
```

Extracts all but the suffix of each file name in names. If the file name contains a period, the basename is everything starting up to (and not including) the last period. Periods in the directory part are ignored. **If there is no period, the basename is the entire file name.**

```shell
$(basename src/foo.c src-1.0/bar hacks)
->
produces the result ‘src/foo src-1.0/bar hacks’.
```

就是将所有文件名的后缀去掉！



27. **notdir**

```shell
$(notdir names...) 
```

Extracts all but the directory-part of each file name in names. If the file name contains no slash, it is left unchanged. Otherwise, **everything through the last slash is removed from it.**

```shell
$(notdir src/foo.c hacks)
->
produces the result ‘foo.c hacks’.
```

就是将所有文件路径的目录全去掉，只保留文件名！



28. **addprefix**

```shell
$(addprefix prefix,names...)
```

The argument **names** is regarded as a series of names, separated by whitespace; **prefix** is used as a **unit**. The value of prefix is prepended to the front of each individual name and the resulting larger names are **concatenated** with **single spaces** between them

```shell
$(addprefix src/,foo bar)
->
produces the result ‘src/foo src/bar’.
```

给names中的每个name加一个前缀 prefix





29. **$***

The stem with which an implicit rule matches

If the **target is dir/a.foo.b** and the target **pattern** is **a.%.b** then the stem is dir/foo



30. **shell 输出结果重定向**

```shell
RESULT = .result
$(shell > $(RESULT))
```



31. **MAKECMDGOALS**

The targets given to make on the command line. Setting this variable has no effect on the operation of make.

```shell
make run -> run
make clean -> clean
make gdb -> gdb
```

Make will set the special variable MAKECMDGOALS to the list of goals you specified on the command line. 



32. **info**

```shell
$(info text...)
```

This **function** does nothing more than print its (expanded) argument(s) to standard output. **No makefile name or line number is added. The result of the expansion of this function is the empty string.**



**Make有一些自己特定的变量和函数！**



33. **filter**

```shell
$(filter pattern...,text)
```

Select words in text that **match one of the pattern words.**

Returns all **whitespace-separated** words in text that do **match any of the pattern words**, **removing any words that do not matc**h. The patterns are written using ‘%’, just like the patterns used in the patsubst function above.





34. **subst**

```shell
$(subst from,to,text)
```

Replace **`from`** with **`to`** in text

```shell
$(subst ee,EE,feet on the street)
->
produces the value ‘fEEt on the strEEt’.
```

就是替换text中特定的字符串！



35. **word**

```shell
$(word n,text)
```

**Returns the nth word of text**. The legitimate values of n **start from 1**. If n is bigger than the number of words in text, the value is empty. 

```shell
For example, $(word 2, foo bar baz) returns ‘bar’.
```

注意从1开始！



使用subst 和 word 抽取单词。

```shell
ARCH_SPLIT = $(subst -, ,$(ARCH))
# $(ARCH) -> riscv64-nemu
# ARCH_SPLIT -> riscv64 nemu
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))
# ISA -> riscv64
# PLATFORM -> nemu
```



36. **flavor**

```shell
$(flavor variable)
```

The **flavor** function, like the origin function, does not operate on the values of variables **but rather it tells you something about a variable.**

The result of this function is a string that identifies the flavor of the variable variable: 

- ‘undefined’ if variable was never defined. 
- ‘recursive’ if variable is a recursively expanded variable. 
- ‘simple’ if variable is a simply expanded variable.

```shell
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif
```

上面的判断就是检查SRCS是否被定义！





37. **addsuffix**

```shell
$(addsuffix suffix,names...)
```

Append suffix to each word in names.

给names中的每个name加一个前缀 prefix

**For example, $(addsuffix .c,foo bar) produces the result ‘foo.c bar.c’.**



38. **sort**

```shell
$(sort list)
```

**Sorts the words of list in lexical order, removing duplicate words.** The output
is a list of words **separated by single spaces.**

Thus, $(sort foo bar lose) returns the value ‘bar foo lose’.

单词排序，去重！





39. **join**

```shell
$(join list1,list2)
```

**Concatenates the two arguments word by word:** 

the two first words (one from each argument) concatenated form the first word of the result, the two second words form the second word of the result, and so on. **So the nth word of the result comes from the nth word of each argument.** If one argument has more words that the other, the extra words are copied unchanged into the result.

就是进行拼接！

For example, ‘$(join a b,.c .o)’ produces ‘a.c b.o’. 

Whitespace between the words in the lists is not preserved; **it is replaced with a single space**. This function can merge the results of the dir and notdir functions, to produce the original list of files which was given to those two functions

两个list成员一 一对应进行拼接！



40. **常用工具！**

```shell
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf
```

- AS / CC / CXX 编译
- LD 链接 .ld
- AR 库 .a
- OBJDUMP 反汇编 .asm
- OBJCOPY 生成二进制文件 .bin
- READELF 分析elf文件 .elf 





41. **DEFAULT_GOAL**

Sets the default goal to be used if no targets were specified on the command line




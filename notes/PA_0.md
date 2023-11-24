1. 磁盘分布

```shell
df -h
```



2. 关机

```shell
sudo poweroff
```

关闭虚拟机一定要用poweroff，防止文件损坏！



3. 切换权限

```shell
su -
```



4. 检查网络状态

```shell
ping mirrors.tuna.tsinghua.edu.cn -c 4
```

```shell
ping www.baidu.com -c 4
```



5. 修改软件源

```shell
bash -c 'echo "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu/ jammy main restricted universe multiverse" > /etc/apt/sources.list'
```



6. 更新软件源头

```shell
sudo apt-get update
```



7. 基础工具和库

```shell
sudo apt-get install build-essential    
# build-essential packages, include binary utilities, gcc, make, and so on
sudo apt-get install man                # on-line reference manual
sudo apt-get install gcc-doc            # on-line reference manual for gcc
sudo apt-get install gdb                # GNU debugger
sudo apt-get install git                # revision control system
sudo apt-get install libreadline-dev    # a library used later
sudo apt-get install libsdl2-dev        # a library used later
sudo apt-get install llvm llvm-dev      # llvm project, which contains libraries used later
sudo apt-get install llvm-11 llvm-11-dev # only for ubuntu20.04
```



8. 选择合适的编辑器

vim or emacs



9. 利用vimtutor 学习vim

或者玩游戏学习vim：

- [Vim Adventures](http://vim-adventures.com)
- [Open Vim Tutorials](http://www.openvim.com/tutorial.html)
- [Vim Genius](http://www.vimgenius.com)



10. 神奇的vim指令可以快速完成一些编辑工作

```
i1<ESC>q1yyp<C-a>q98@1
```

<C-a> 就是crtl+a

```
1
2
3
.....
98
99
100
```

先初始化第一行，然后记录第二行的产生，后续的工作就是重复上面这个过程！

```
<C-v>24l4jd$p
```

```
aaaaaaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbb
cccccccccccccccccccccccccddddddddddddddddddddddddd
eeeeeeeeeeeeeeeeeeeeeeeeefffffffffffffffffffffffff
ggggggggggggggggggggggggghhhhhhhhhhhhhhhhhhhhhhhhh
iiiiiiiiiiiiiiiiiiiiiiiiijjjjjjjjjjjjjjjjjjjjjjjjj
```

将上面的文本改成

```
bbbbbbbbbbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaaaaaaaaaaa
dddddddddddddddddddddddddccccccccccccccccccccccccc
fffffffffffffffffffffffffeeeeeeeeeeeeeeeeeeeeeeeee
hhhhhhhhhhhhhhhhhhhhhhhhhggggggggggggggggggggggggg
jjjjjjjjjjjjjjjjjjjjjjjjjiiiiiiiiiiiiiiiiiiiiiiiii
```

<C-v> 就是crtl+v



11. 如何开启vim语法高亮？

```
cp /etc/vim/vimrc ~/.vimrc
```

```
cd ~
```

```
ls -a
```

显示隐藏文件

```
vim .vimrc
```

vim 的配置文件就是这里的vimrc，修改它就可以了！



12. 神奇的diff指令

```
diff -u fileA fileB
```

https://www.gnu.org/software/diffutils/manual/html_node/Unified-Format.html



13. 询问神奇的man

```
man man
```

shell 指令、系统调用、库函数、设备文件、配置文件

很多东西都可以询问man



14. ^N 表示什么？

表示ctrl + N



15. CR 表示什么？

<Enter> 键！



16. 写一个HELLO WORLD程序，需要完成GNU/Linux 上的编译、写一个Makefile、还要学会用gdb进行调试！



```
gcc hello.c -g -Wall -Werror -o hello
```



```
CC = gcc
FILES = hello.c  
OUT_EXE = hello 
 
build: $(FILES)
        $(CC) -o $(OUT_EXE) $(FILES)
 
clean:
        rm -f *.o 
        rm -f $(OUT_EXE) 
rebuild: clean build
```



17. 学会使用tmux

安装

```
apt-get install tmux
```

配置

```
cd ~
vim .tmux.conf

bind-key c new-window -c "#{pane_current_path}"
bind-key % split-window -h -c "#{pane_current_path}"
bind-key '"' split-window -c "#{pane_current_path}"
```

使用

```
man tmux
```

https://www.51cto.com/article/664989.html



18. 思考一下GUI中的滚动条是怎么实现的？

https://zhuanlan.zhihu.com/p/130840880



19. 命令行有时候比图形化界面更加高效！

**如何比较两个文件是否完全相同?** 通过diff指令，文件大就用md5sum比较md5！

**如何列出一个C语言项目中所有被包含过的头文件?** 

```
find . -name "*.[ch]" | xargs grep "#include" | sort | uniq
```

find、xargs、grep、sort、uniq的使用方法都可以用man查询

文本更加高效、工具组合解决复杂问题

==工具的组合也是一种编程的思想==





20. 常见的命令行工具

- 文件管理 - `cd`, `pwd`, `mkdir`, `rmdir`, `ls`, `cp`, `rm`, `mv`, `tar`
- 文件检索 - `cat`, `more`, `less`, `head`, `tail`, `file`, `find`
- 输入输出控制 - 重定向, 管道, `tee`, `xargs`
- 文本处理 - `vim`, `grep`, `awk`, `sed`, `sort`, `wc`, `uniq`, `cut`, `tr`
- 正则表达式
- 系统监控 - `jobs`, `ps`, `top`, `kill`, `free`, `dmesg`, `lsof`



**询问man 就可以了！**



21. 下面这个课程一定要找个时间学一下

[The Missing Semester of Your CS Education](https://missing.csail.mit.edu/)



22. 不要轻易使用root用户，最好是新建一个用户，然后用 sudo ！





23. 获取项目代码

```shell
git clone -b 2022 git@github.com:NJU-ProjectN/ics-pa.git ics2022
```



24. 配置git

```shell
git config --global user.name "211220000-Zhang San" 
# your student ID and name
git config --global user.email "zhangsan@foo.com"   
# your email
git config --global core.editor vim                 
# your favorite editor
git config --global color.ui true
```



25. 初始化项目

```
git branch -m master
bash init.sh nemu
bash init.sh abstract-machine
```

上面的init.sh 会在~/.bashrc中添加一些环境变量

让环境变量生效

```
source ~/.bashrc
```

在bash中使用，不要在fish中使用！

检测是否成功？

```
echo $NEMU_HOME
echo $AM_HOME
cd $NEMU_HOME
cd $AM_HOME
```



26. 切换到pa0分支！

```
git branch

git checkout -b pa0
```



27. 修改Makefile的学号和姓名！

```makefile
STUID = 20051124  		# your student ID
STUNAME = BoneInscri    # your Chinese name
```



28. 使用git diff查看修改的变化

```shell
git diff
```



29. 提交修改

```
git add .
git commit
```



30. 查看commit记录

```
git log
```



31. 分支切换

```
git checkout master
git checkout pa0
```



32. 一般方法：

先切换到一个新的分支，然后在那个分支上进行开发，然后将这个分支的修改merge到master！



33. 进入.config的界面，默认直接退出 <Exit> 就可以了

```
make
```



34. 需要知道编译器输出的都是什么东西？

CC、CXX、LD



35. 清空编译内容

```
make clean
```



36. 运行NEMU

```
make run
```



37. debug NEMU

```
make gdb
```



38. 注释掉assert！ 

```
assert(0);
```





C 代码：18564 行

头文件：2382 行






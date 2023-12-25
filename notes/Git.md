1. 为什么要有git？

就是一个代码“存档”！

就是一个版本控制系统！



2. 安装和配置

```
apt-get install git
```

```
git config --global user.name "Zhang San"        
# your name
git config --global user.email "zhangsan@foo.com"    
# your email
git config --global core.editor vim            
# your favourite editor
git config --global color.ui true
```



3. git 的初始化

```
git init
```



4. 查看所有commit记录

```
git log
```



5. 查看文件有哪些变化

```
git status
```



6. 添加文件

```
git add file.c
```



7. 添加所有

```
git add -A
```



8. 配置 .gitignore 可以选择性忽略一些文件



9. 每一个记录都有一个hash值，你不需要输入整个hash code！

回到过去

```
git reset --hard b87c
```



10. master是主分支，查看所有分支

```
git branch
```



11. 切换分支

```
git checkout ?
```



12. 切换到一个“旧”分支后，如果对代码进行了修改，需要重新建一个分支！

```
git checkout -B 分支名
```

然后commit！



13. 更多的使用方法，问man！

```
man git
```



14. 可视化的git操作

http://onlywei.github.io/explain-git-with-d3/



15. 删除远程分支

```
git push origin --delete [branch_name]
```



16. 撤销commit

````shell
git reset HEAD^
````




##### git 基本用法

- git add filename
  - 添加文件\文件夹到工作区
- git commit -m " " 
  - 将暂存区的文件提交到git本地仓库
- git status     
  - 查看当前状态下各哦你工作区的文件状态，绿色代表对暂存区的删除增加操作，红色不加-a不会提交到本地仓库
- git log       
  - 查看修改日志
- git pull origin master    
  - 将远程仓库项目拉下来
- git rm -r --cachede filename  
  - 删除文件或者文件夹
- git push -u origin master 
  - 将本次修改的提交到远程仓库
- git remote rm origin
  - 断开连接
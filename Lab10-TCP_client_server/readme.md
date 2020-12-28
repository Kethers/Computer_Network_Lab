# 测试说明

## 客户端输入命令测试

aaa 应当提示缺少文件名
aaa 111 应当提示命令错误
upload 应当提示缺少文件名

## upload 命令测试

upload 1.txt 应当提示文件已经存在，不能被覆写
upload 2.txt 查看文本文件上传后是否正确
upload FFXV-wedding.jpg 查看图片文件上传后是否正确

## download 命令测试

download 111.txt 应当提示服务器没有该文件
download 1.txt 查看下载下来的文件是否正确
download Origin.jpg 查看下载下来的图片是否正确

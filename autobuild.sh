#!/bin/bash

# 脚本执行过程中遇到错误时立即停止执行
set -e
# 删除 build 目录下的所有文件和子目录
rm -rf `pwd`/build/*
cd `pwd`/build &&
	cmake .. &&
	make
cd ..
# 将 src/include 目录下的所有文件和子目录复制到 lib 目录中
cp -r `pwd`/src/include `pwd`/lib
#! /usr/bin/env python
# coding: utf-8

__author__ = 'hzzhongyaping'

import os,sys
import os.path
from hashlib import md5
from hashlib import sha1
import shutil
import logging
class Utils:

    @staticmethod
    def create_dirs(to_create_path):
      #home/hzhudingyao/Desktop/apkaudit/binwork/93779959C4327BED6FE972EC129A665D_4/unzip/res/layout
        #创建多级目录，比如c:\\test1\\test2,如果test1 test2都不存在，都将被创建
        dirs = to_create_path.split(os.sep)
        path = ''
        for dir in dirs:
         #   if len(dir.strip())==0:
          #      continue
       
            dir += os.sep
            path = os.path.join(path, dir)
            if not os.path.exists(path):
                os.mkdir(path, 0o777)

    @staticmethod
    def delete_file(to_del_file):
        if os.path.exists(to_del_file):
            os.remove(to_del_file)

    @staticmethod
    def delete_dirs(to_del_dirs):
        if os.path.exists(to_del_dirs):
            shutil.rmtree(to_del_dirs)

    @staticmethod
    def md5_file(file_path):
        with open(file_path, "rb") as file:
            m = md5()
            while True:
                strRead = file.read(1024 * 1024)
                if not strRead:
                    break
                m.update(strRead)
            return m.hexdigest().upper()

    @staticmethod
    def sha1_file(file_path):
        with open(file_path,"rb") as file:
            s = sha1()
            while True:
                strRead = file.read(1024*1024)
                if not strRead:
                    break
                s.update(file.read())
        return s.hexdigest().upper()





    #获取脚本文件的当前路径
    @staticmethod
    def getCurPath():
        #获取脚本路径
        path = sys.path[0]
        #判断为脚本文件还是py2exe编译后的文件，如果是脚本文件，则返回的是脚本的目录，如果是py2exe编译后的文件，则返回的是编译后的文件路径
        if os.path.isdir(path):
            return path
        elif os.path.isfile(path):
            return os.path.dirname(path)


    #获取路径的父目录
    @staticmethod
    def getParentPath(strPath):
        if not strPath:
            return None;

        lsPath = os.path.split(strPath);
        #print(lsPath);
        #print("lsPath[1] = %s" %lsPath[1]);
        if lsPath[1]:
            return lsPath[0];
        lsPath = os.path.split(lsPath[0]);
        return lsPath[0];

# def main():
#     Utils.create_dirs("d:\\test1\\test2")
#     #Utils.create_dirs("/home/test1/test2")
#     #print Utils.md5_file("C:\\Users\\hzzhongyaping\\Desktop\\hellojni.apk")


# if __name__ == "__main__":
#     main()


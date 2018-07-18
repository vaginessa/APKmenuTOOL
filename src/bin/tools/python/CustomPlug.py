#coding: utf-8
__author__ = 'hzfengqizhong'
import os,re,platform
import zipfile
import sys
from GameApkDetect import GameApkDetect
from zipmgr import zipmgr

if __name__ == "__main__":
    '''
    自定义插件要求:
    函数映射关系要求：
        插件序号：自定义函数名，其中插件序号从1开始

    自定义函数要求：
        1、filepath: 选中的文件或者文件夹路径。 toolspath：右键工具安装目录下的tools目录，里面为常用工具
        2、函数内部不能使用print等函数向控制台打印数据
        3、返回必须是字典，且要求键为result，如 result = {'result':u'我是返回的结果'}
    '''

    operate = {'1':'func1', '2':'func2', '3':'func3', '4':'func4', '5':'func5','6':'func6'}

    def func1(filepath, toolspath):
        #自定义函数体
        result = {'result':u'请到"安装目录/tools/python/CustomPlug.py"中的func1函数中添加自定义功能!'}
        return result

    def func2(filepath, toolspath):
        result = {'result':u'请到"安装目录/tools/python/CustomPlug.py"中的func2函数中添加自定义功能!'}
        return result

    def func3(filepath, toolspath):
        result = {'result':u'请到"安装目录/tools/python/CustomPlug.py"中的func3函数中添加自定义功能!'}
        return result

    def func4(filepath, toolspath):
        result = {'result':u'请到"安装目录/tools/python/CustomPlug.py"中的func4函数中添加自定义功能!'}
        return result

    def func5(filepath, toolspath):
        result = {'result':u'请到"安装目录/tools/python/CustomPlug.py"中的func5函数中添加自定义功能!'}
        return result
    def func6(filepath, toolspath):
        result = {'result':u'请到"安装目录/tools/python/CustomPlug.py"中的func6函数中添加自定义功能!'}
        return result

    '''
    ###########################################
           注意： 以下内容不要修改
    ###########################################
    '''
    def fun(filepath, toolspath, index):
        # print filepath
        # print toolspath
        result = ''
        try:
            function = operate.get(index)
            if function and function != '':
                ret = globals()[function](filepath, toolspath)
                if ret and type(ret) == dict:
                    if ret.has_key('result') and ret['result'] != '':
                        result = ret['result']
                    else:
                        result = u'自定义函数未设置返回值'
                else:
                    result = u'自定义函数没有返回值或者返回值不是字典类型'
            else:
                result = u'请到"安装目录/tools/python/CustomPlug.py"中的operate中注册函数'

        except Exception,e:
            result = u"自定义插件" + str(index) + u"处理异常，原因：" + str(e.message)

        print result



    '''
    脚本运行入口
    '''
    if len(sys.argv) != 4:
        print u"输入的参数不正常"
    else:
        # filepath = unicode(sys.argv[1],"gbk")
        # toolspath = unicode(sys.argv[2],"gbk")
        # index = unicode(sys.argv[3],"gbk")

        filepath = sys.argv[1]
        toolspath = sys.argv[2]
        index = sys.argv[3]

        fun(filepath, toolspath, index)
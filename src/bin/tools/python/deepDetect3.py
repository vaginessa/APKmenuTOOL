#coding:utf-8
__author__='wangzhixiang'
import sys
import os
import zipfile
import subprocess
from GameApkDetect3 import GameApkDetect
from zipmgr import zipmgr

class deepDetect:
	def __init__(self,dic):
		self.tools_path=dic['tools_path']
		self.apk_path=dic['apk_path']
		self.type=''
		self.name_list=zipmgr.getZipNameList(self.apk_path)
		self.rootpath=''
	def _run(self,args):
		print(self._flatten(args))
		p=subprocess.Popen(args,stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             shell=False)
		ret=p.communicate()
		return self._out2str(ret)
	def _out2str(text_seq):
		return str.strip(" ".join(id.decode() for id in text_seq).replace("\r", "").replace("\n", ""))
	def _flatten(self,x):		
		result = []
		for el in x:
			if isinstance(x, list) and not isinstance(el, str):
				result.extend(self._flatten(el))
			else:
				result.append(el)
		return result
	def checkXposed(self):
		#print(self.apk_path)
		if not os.path.exists(self.apk_path):
			print(u'apk 路径不存在')
			return False
		self.rootpath=os.path.split(self.apk_path)[0]
		xposed_file=''
		flag=False
		for name in self.name_list:
			if 'xposed_init' in name:
				xposed_file=name
				flag=True
				break
		if flag:
			self.type='xposed_apk'
			zfobj = zipfile.ZipFile(self.apk_path)
			content=zfobj.read(xposed_file)
			zfobj.close()#即使在上面某些发生异常应该也没事，zfobj的析构函数会调用close
			#print(content.decode())
			print(u'该APP为Xposed插件APP,入口点为：'+content.decode())
			return True
		else:
			return False
	def checkGame(self):
		if not os.path.exists(self.apk_path):
			print(u'apk 路径不存在')
			return False
		gameDetect = GameApkDetect(self.apk_path)
		dits = gameDetect.getResult()
		if dits['isu3d']:
			print(u'该APP为游戏类APP，游戏引擎为U3D'),
			print(u'U3D脚本已经加密' if dits['issafe'] else u' U3D脚本未加密')
			return True
		elif dits['isflash']:
			print(u'该APP为游戏类APP，游戏引擎为Flash')
			return True
		elif dits['isneox']:
			print(u'该APP为游戏类APP，游戏引擎为neox')
			return True
		elif dits['iscocos']:
			print(u'该APP为游戏类APP，游戏引擎为cocos')
			return True
		else:
			return False
	def getResult(self):
		if self.checkGame():
			return
		elif self.checkXposed():
			return
		else:
			print(u'该APP为普通APP')
			return

#if __name__=='__main__':
#	dic={'tools_path':'','apk_path':u'C:\\Users\\13251\\Downloads\\17省赛\\半决赛\\xposed.apk'}
#	deep=deepDetect(dic)
#	deep.getResult()
if __name__=='__main__':
	if len(sys.argv) != 3:
		print(u"输入的参数不正确")
	else:
		dit = {'apk_path':'','tools_path':''}
		dit['apk_path'] = sys.argv[1]
		dit['tools_path'] = sys.argv[2]
		deep=deepDetect(dit)
		deep.getResult()
#coding:utf-8

import os
import struct
import random
import sys
import re
from zipmgr import zipmgr
from Utils import Utils
from importlib import reload
reload(sys)

class GameApkDetect:
    def __init__(self,apkpath):
        self.which_game = ''
        self.zipnamelist = zipmgr.getZipNameList(apkpath)
        self.unzippath = '';
        self.apkpath = apkpath
        self.u3d_regex = re.compile('libmono.so|libunity.so', re.I)
        self.flash_regex = re.compile('libCore.so', re.I)
        self.neox_regex = re.compile('res.npk', re.I)
        self.u3d_dllpath = 'assets/bin/Data/Managed'
        self.coco_regex=re.compile('libcocos2d(.*).so', re.I)
    def __del__(self):
        if os.path.exists(self.unzippath):
            Utils.delete_dirs(self.unzippath)

    def isGameApk(self):
        # if os.sep == '\\':
        #     self.u3d = self.u3d.replace('/',os.sep)
        #     self.flash = self.flash.replace('/',os.sep)
        #     self.neox = self.neox.replace('/',os.sep)
        if not self.zipnamelist:
            return

        try:
            for filename in self.zipnamelist:
                find = self.u3d_regex.search(filename)
                if find:
                    self.which_game = 'u3d'
                    break
                find = self.flash_regex.search(filename)
                if find:
                    self.which_game = 'flash'
                    break
                find = self.neox_regex.search(filename)
                if find:
                    self.which_game = 'neox'
                    break
                find = self.coco_regex.search(filename)
                if find:
                    self.which_game='coco'
                    break

        except Exception as e:
            pass

    #如果是u3d游戏，判断C#脚本是否加密，Assembly-CSharp.dll标志32字节为0未加密，不为0，加密
    def  isCrypt(self):
        ACdll_path = ''
        ACfdll_path = ''

        root,filename = os.path.split(self.apkpath)
        self.unzippath = os.path.join(root,"%s_%d" % (Utils.md5_file(self.apkpath), random.randint(0, 100)))

        if os.path.exists(self.unzippath):
            Utils.delete_dirs(self.unzippath)
        os.mkdir(self.unzippath,0o777)

        if self.u3d_dllpath+'/'+"Assembly-CSharp.dll" in self.zipnamelist:
            filename = zipmgr.unzip_file_nocrete_dir(self.apkpath, self.u3d_dllpath+'/'+"Assembly-CSharp.dll", self.unzippath)
            ACdll_path = os.path.join(self.unzippath,filename)
        elif self.u3d_dllpath+'/'+"Assembly-CSharp-firstpass.dll" in self.zipnamelist:
            filename = zipmgr.unzip_file_nocrete_dir(self.apkpath, self.u3d_dllpath+'/'+"Assembly-CSharp-firstpass.dll", self.unzippath)
            ACfdll_path = os.path.join(self.unzippath,filename)

        if len(ACdll_path) > 0 and os.path.exists(ACdll_path):
            with open(ACdll_path,'rb') as acdll:
                acdll.seek(0x20,0)
                ACdllflag =  struct.unpack("b",acdll.read(1))[0]
                if not ACdllflag == 0:#标志位第32字节为0未加密，不为0，加密
                    return True
        elif len(ACfdll_path) > 0 and os.path.exists(ACfdll_path):
            with open(ACfdll_path,'rb') as acfdll:
                acfdll.seek(0x20,0)
                ACfdllflag =  struct.unpack("b",acfdll.read(1))[0]
                if not ACfdllflag == 0:#标志位第32字节为0未加密，不为0，加密
                    return True

        return False

    def getResult(self):
        self.isGameApk()
        result_dit = {'isu3d':False,'issafe':False,'isflash':False,'isneox':False,'iscocos':False}

        if self.which_game == 'u3d':
            result_dit['isu3d'] = True
            if self.isCrypt():
                result_dit['issafe'] = True
        elif self.which_game=='flash':
            result_dit['isflash']=True
        elif self.which_game=='neox':
            result_dit['isneox']=True
        elif self.which_game=='coco':
            result_dit['iscocos'] = True
        return result_dit

if __name__ == "__main__":
    gameDetect = GameApkDetect('E:\\测   试\\g42_alpha.apk')
    # gameDetect = GameApkDetect('E:\\test\\g42_alpha.apk')
    #dits = gameDetect.getResult()
    gameDetect.test()
    #print(dits)



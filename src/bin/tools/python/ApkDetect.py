#coding: gbk
import os,re,platform
import zipfile
import sys
from GameApkDetect import GameApkDetect
from zipmgr import zipmgr

reload(sys)
sys.setdefaultencoding('gbk')

class ApkDetect:
    def __init__(self,dit):
        # self.apk_path = unicode(dit['apkPath'], "utf-8") #下载后的APK地址   apkpath = unicode(apk_path , "utf-8")  #对路径进行utf-8编码
        self.apk_path = dit['apkPath'] #下载后的APK地址   apkpath = unicode(apk_path , "utf-8")  #对路径进行utf-8编码
        self.tools_path = dit['tools_path']
        self.xmltree = ''#通过aapt获取的manifest.xml
        self.wrapperSdk = r'Failed'
        self.lastError = ''
        self.bugrptId = ''
        self.appdit = {'wrapperSdk':'', 'lastError':'', 'bugrptID':''}
        self.zipnamelist = []
        self.apkInfo = ''
        self.iconName = ''
        self.rootpath = ''
        #其中APKProtect加固是没有不判断application的
        self.applicationName = [r'com\.secneo\.apkwrapper|com\.secneo\.guard\.ApplicationWrapper|com\.secshell\.shellwrapper\.SecAppWrapper|com\.bangcle\.protect|com\.secshell\.secData\.ApplicationWrapper', r'com\.qihoo\.util\.StubApplication',
                   r'com\.payegis\.ProxyApplication', r'com\.nqshield\.NqApplication', r'com\.tencent\.StubShell\.TxAppEntry',
                   r'com\.ijiami\.residconfusion\.ConfusionApplication|com\.shell\.SuperApplication', r'com\.edog\.AppWrapper|com\.chaosvmp\.AppWrapper',
                   r'com\.ali\.mobisecenhance\.StubApplication', r'com\.baidu\.protect\.StubApplication',r'com\.netease\.nis\.wrapper\.MyApplication']
        #print(self.applicationName)
        self.soName = [r'libDexHelper\S*\.so|libsecexe\S*\.so|libSecShell\.so', r'libjiagu\S*\.so|libprotectClass\S*\.so',
          r'libegis\S*\.so|libegisboot\S*\.so|libegismain\S*\.so|libNSaferOnly\S*\.so', r'libnqshield\S*\.so',
          r'libtxRes\S*\.so|libshell\S*\.so', r'libexecgame\.so|ijiami\S*.dat', r'lib\wdog\.so', r'libmobisec\w*\.so|libaliutils\S*\.so',
          r'libbaiduprotect\S*\.so', r'libnesec\.so|assets/data\.db|assets/clazz\.jar|libdexfix\.so', r'libAPKProtect\S*\.so']
        self.protectflag_dict = {1: u"梆梆加固",2: u"360加固", 3: u"通付盾加固",
                    4: u"网秦加固", 5: u"腾讯加固",6: u"爱加密加固",
                    7: u"娜迦加固",8: u"阿里加固", 9: u"百度加固",10:u"网易云加密",
                    11: u"APKProtect加固",0: u"NO WRAPPER"}
        self.appName_regex = [re.compile(self.applicationName[0], re.I), re.compile(self.applicationName[1], re.I),
                 re.compile(self.applicationName[2], re.I), re.compile(self.applicationName[3], re.I),
                 re.compile(self.applicationName[4], re.I), re.compile(self.applicationName[5], re.I),
                 re.compile(self.applicationName[6], re.I), re.compile(self.applicationName[7], re.I),
                 re.compile(self.applicationName[8], re.I), re.compile(self.applicationName[9], re.I)]
        self.soName_regex = [re.compile(self.soName[0], re.I), re.compile(self.soName[1], re.I), re.compile(self.soName[2], re.I),
                re.compile(self.soName[3], re.I), re.compile(self.soName[4], re.I), re.compile(self.soName[5], re.I),
                re.compile(self.soName[6], re.I), re.compile(self.soName[7], re.I), re.compile(self.soName[8], re.I),
                re.compile(self.soName[9], re.I), re.compile(self.soName[10], re.I)]

        # self.bugrptID_regex = re.compile("A: android:name(0x01010003)="BUGRPT_APPID" (Raw: "BUGRPT_APPID")
        # A: android:value(0x01010024)="A007600419" (Raw: "A007600419")")

    #读取APK文件名列表
    def getZipNameList(self,apk_path):
        self.lastError=''
        if not os.path.isfile(apk_path):
            self.lastError=u'apk文件不存在'
            return False
        if not zipfile.is_zipfile(apk_path):
            self.lastError=u'非法的apk文件'
            return False
        try:
            zfobj = zipfile.ZipFile(apk_path)
            self.zipnamelist = zfobj.namelist()
            zfobj.close()
        except Exception as e:
            # print "%s" % e
            self.lastError=u'获取apk中文件列表异常'
            return False
        return True

    #通过aapt获取的manifest.xml
    def getXmlInfo(self):
        xml_cmd = ''
        self.lastError=''

        if 'Windows' in platform.system():
            #xml_cmd = "%s%saapt.exe d xmltree \"%s\" AndroidManifest.xml "%(self.aapt_path,os.sep, self.apk_path)
            xml_cmd = "java -jar %s%sAXMLPrinter2.jar \"%s\" " % (self.tools_path,os.sep, self.apk_path)

        try:
            strxml = os.popen(xml_cmd)
            self.xmltree = strxml.read()
            # print self.xmltree
        except Exception as e:
            # print "aapt Mainfestxml error"
            self.lastError = 'get AndroidManifest.xml error'
            return False
        return True

    #从xml中检测加壳信息
    def checkManifest(self):
        for key in range(len(self.applicationName)):
            result = self.appName_regex[key].search(self.xmltree)
            if result:
                return key+1
            else:
                continue
        return 0

    #通过aapt获取apk的信息
    def getApkInfo(self):
        apkinfo_cmd = ''
        aaptlines=[]
        if 'Windows' in platform.system():
            apkinfo_cmd = "%s%saapt.exe  dump badging \"%s\" "%(self.tools_path, os.sep, self.apk_path)

        try:
            linestr = os.popen(apkinfo_cmd)
            self.apkInfo = linestr.readlines()
        except Exception as e:
            return False

        for aaptline in self.apkInfo:
             #从package中获取icon信息
             # print aaptline
             self.iconName = ''
             if  aaptline.find('application: label=')>-1:
                 pattern = r'icon=\'(\S*)\''
                 m = re.search(pattern,aaptline)
                 if m:
                     self.iconName = m.group(1)
                     print(self.iconName)
                     break

        if self.iconName == '':
            return False
        return True

    def saveIcon(self):
        if not os.path.exists(self.apk_path):
            print u'apk 路径不存在'
            return False
        result = self.getApkInfo()
        if not result:
            print u'获取apk图标路径失败'
            return False
        self.rootpath,file = os.path.split(self.apk_path)
        result = zipmgr.unzip_allIocn_to_dir(self.apk_path, self.iconName, self.rootpath)
        if not result:
            print u'解压apk图标失败'
            return False

        # icon_path = os.path.join(self.rootpath, filename)
        # print icon_path
        print u'提取图标完成'
        return True

    def getSaveLib(self):
        result=self.saveLib()
        if result:
            print(u'提取so文件完成')
        else:
            print(u'提取失败')
    #提取lib
    def saveLib(self):
    	if not os.path.exists(self.apk_path):
            print u'apk 路径不存在'
            return False
        namelist=zipmgr.getZipNameList(self.apk_path)
        soPath=[]
        for name in namelist:
            if name.find(".so")!=-1 and name.split('/')[0]=='lib':
        		soPath.append(name)
        if len(soPath)!=0:
            self.rootpath=os.path.split(self.apk_path)[0]
            flag=False
            for name in soPath:
            	result=zipmgr.unzip_file(self.apk_path,name,self.rootpath)
            	if result!=None:
            		flag=True
            	else:
            		continue
            if flag:
            	
            	return True
            else:
            	
            	return False
        else:
        	print u'该apk不存在so文件'
        	return False
    def checkRight(self):
        if not os.path.exists(self.apk_path):
            print u'apk 路径不存在'
            return False
        namelist=zipmgr.getZipNameList(self.apk_path)
        soList={}
        for name in namelist:
            if name.split('/')[0]=='lib':

                smallFile=name.split('/')[1]
                if(len(name.split('/'))==2):
                    strfile=""
                else:
                    strfile=name.split('/')[2]
                if smallFile in soList.keys():
                    fileList=soList[smallFile]
                    fileList.append(strfile)
                    soList[smallFile]=fileList
                else:
                    fileList=[]
                    fileList.append(strfile)
                    soList[smallFile]=fileList
        #print(soList)
        if(len(soList)!=0):
            fileTree=self.getFileTree()
            if fileTree:
                print(fileTree)
                for smallFile in soList.keys():
                    for sss in soList.keys():
                        ret=cmp(soList[smallFile],soList[sss])
                        if ret!=0:
                            print(u'该APK so文件不整齐')
                        return False
                print(u'该APK so文件整齐')
                return True

    def getFileTree(self):
        fileName=self.apk_path
        temp=fileName.split('.')
        temp.remove('apk')
        fileName='.'.join(temp)
        fileName = fileName+'_lib'
        result=self.saveLib()
        if result:
            treeCmd='tree /F %s'%(fileName)
            tree=os.popen(treeCmd)
            fileTree=tree.read()
            deleteCmd='rd /s /q %s'%fileName
            os.popen(deleteCmd)
            fileTree=unicode(fileTree,'gbk')
            index=fileTree.find(fileName.upper())
            fileTree=fileTree[index+len(fileName):]
            return fileTree
        else:
            return None



        
    #获取APK的加壳SDK信息
    def getWrapperSdk(self):
        self.lastError = ''
        index = self.checkManifest()
        if index != 0:
            self.wrapperSdk = self.protectflag_dict[index]
            # print self.apk_path + ": Manifest: " + self.wrapperSdk
            # print "[WrapperSdk] ",self.wrapperSdk
            return True

        try:
            find = False
            for fileName in self.zipnamelist:
                if not find:
                    for key in range(len(self.soName)):
                        result = self.soName_regex[key].search(fileName)
                        if result:
                            # print key
                            self.wrapperSdk = self.protectflag_dict[key+1]
                            find = True
                            break
                else:
                     break
            if not find:
                self.wrapperSdk = self.protectflag_dict[0]
            # print self.apk_path + ": So: " + self.wrapperSdk
            #print "[WrapperSdk] ",self.wrapperSdk
            return True
        except Exception, e:
            # print "parser wrap sdk error: "
            # logging.error(e)
            self.lastError = 'parser wrap sdk error'
            return False

    def getBurptID(self):
        try:
            pattern = r'Raw: "BUGRPT_APPID"\)\s+A: android:value\S+ \(Raw: "(\S*)"'
            m = re.search(pattern,self.xmltree)
            if m:
                self.bugrptId = 'BUGRPT_APPID:'+str(m.group(1))
                return True
        except Exception,e:
            self.bugrptId = ''
        return False

    #该函数最后调用，更新全局字典类型
    def  getAppDit(self):
        self.appdit['lastError'] = self.lastError
        self.appdit['wrapperSdk'] = self.wrapperSdk
        self.appdit['bugrptID'] = self.bugrptId

    #主函数，外部调用该函数
    def apkDetect(self):
        if not self.getXmlInfo():
            return

        if not self.getZipNameList(self.apk_path):
            return
        self.getWrapperSdk()
        self.getBurptID()

    def result(self):
        self.apkDetect()
        self.getAppDit()
        #print(self.appdit)
        return self.appdit

if __name__ == "__main__":
   #初始化地址
   if len(sys.argv) != 4:
      print u"输入的参数不正确"
   else:
      dit = {'apkPath':'','tools_path':''}
      dit['apkPath'] = unicode(sys.argv[1],"gbk")
      dit['tools_path'] = unicode(sys.argv[2],"gbk")
      flag = unicode(sys.argv[3],"gbk")#
      ad = ApkDetect(dit)
      if flag == '1':  #表示获取图标
         ad.saveIcon()
      elif flag=='2':
      	ad.getSaveLib()
      elif flag=='3':
        ad.checkRight()
      else:  #查壳
         dit_result = ad.result()#
         if len(dit_result) != 3:
            print u"解析加壳sdk失败"
         else:
            if dit_result['wrapperSdk'] == 'Failed':
               print dit_result['lastError'],
            else:
               print dit_result['wrapperSdk'],
               if dit_result['bugrptID'] != '':
                  print dit_result['bugrptID'],

#if __name__ == "__main__":
#    #初始化地址
#    dit = {'apkPath':'D:\\Android\MyApplication2\\app\\testctf2_align.apk','tools_path':'D:\\apktools\\tools'}
#    ad = ApkDetect(dit)
    
#    dit_result = ad.result()
    
#    if len(dit_result) != 3:
#       print u"解析加壳sdk失败"
#    else:
#       if dit_result['wrapperSdk'] == 'Failed':
#          print dit_result['lastError'],
#       else:
#          print dit_result['wrapperSdk'],
#          if dit_result['bugrptID'] != '':
#             print dit_result['bugrptID'],
    
#       print 'ddd'


    #初始化地址
    #dit = {'apkPath':u'E:\\测   试\\input.apk','aaptPath':'E:\\client\\android\\apktool\\bin\\x64\\tools'}
#    ad = ApkDetect(dit)
#    ad.checkRight()
    #ad.saveIcon()

#coding: gbk
__author__ = 'hzfengqizhong'
import os,re,platform
import zipfile
import sys
import subprocess
from compiler.ast import flatten
from GameApkDetect import GameApkDetect
from zipmgr import zipmgr

reload(sys)
sys.setdefaultencoding('gbk')

class ToolsCall:
    def __init__(self, dits):
        self.file_path = dits['file_path']
        self.tools_path = dits['tool_path']
        self.lastError = ''
        self.saved_file_path = ''

    @staticmethod
    def _run(args):
        p = subprocess.Popen(flatten(args), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                             shell=False)
        ret = p.wait()
        output = ToolsCall._out2str(p.stdout.readlines())
        error = ToolsCall._out2str(p.stderr.readlines())
        return output if ret is 0 else error

    @staticmethod
    def _out2str(text_seq):
        return str.strip(" ".join(text_seq).replace("\r", "").replace("\n", "")).decode('utf-8')

    def AxmlToXml(self):
        xml_cmd = ''
        self.lastError=''

        if not os.path.exists(self.file_path):
            self.lastError = u'AXML文件路径不存在'
            return False

        length = len(self.file_path)
        self.saved_file_path = self.file_path[0:length-4] + '_new.xml'

        if 'Windows' in platform.system():
            xml_cmd = "java -jar %s%sAXMLPrinter2.jar \"%s\" " % (self.tools_path,os.sep, self.file_path)

        if 'Linux'in platform.system():
            xml_cmd = "java -jar %s%sAXMLPrinter2.jar \"%s\" "%(self.tools_path,os.sep, self.file_path)

        try:
            result = os.popen(xml_cmd)
            content = result.read()

            with open(self.saved_file_path, 'wb') as file:
                file.write(content)
                file.flush()

            if not os.path.exists(self.saved_file_path):
                self.lastError = 'AXML -> XML error '
                return False
        except Exception as e:
            self.lastError = str(e)
            return False
        return True

    def getXml(self):
        result = self.AxmlToXml()
        if result:
            print self.saved_file_path
            return

        print self.lastError

    def zipAligns(self):
        xml_cmd = ''
        self.lastError=''

        if not os.path.exists(self.file_path):
            self.lastError = u'apk文件路径不存在'
            return False

        length = len(self.file_path)
        self.saved_file_path = self.file_path[0:length-4] + '_align.apk'

        if 'Windows' in platform.system():
            xml_cmd = "%s%szipalign.exe -f 4  \"%s\" \"%s\" "%(self.tools_path,os.sep, self.file_path, self.saved_file_path)

        if 'Linux'in platform.system():
            pass

        xml_cmd = "%s%szipalign.exe" % (self.tools_path,os.sep)

        try:
            # result = os.popen(xml_cmd) #输出文件会延迟
            self._run([xml_cmd, "-f", "4", self.file_path, self.saved_file_path])

            if not os.path.exists(self.saved_file_path):
                self.lastError = 'zipalign failed '
                return False
        except Exception as e:
            self.lastError = str(e)
            return False
        return True

    def apkAlign(self):
        result = self.zipAligns()
        if result:
            print self.saved_file_path
            return

        print self.lastError


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print u"输入的参数不正常"
    else:
        dit = {'file_path':'','tool_path':''}
        dit['file_path'] = unicode(sys.argv[1],"gbk")
        dit['tool_path'] = unicode(sys.argv[2],"gbk")
        flag = unicode(sys.argv[3],"gbk")

        tc = ToolsCall(dit)
        if flag == 'axml':
            tc.getXml()
        elif flag == 'align':
            tc.apkAlign()
        else:
            pass


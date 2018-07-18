__author__ = 'hzfengqizhong'
import os,re,platform
import zipfile
import sys
import subprocess
from GameApkDetect3 import GameApkDetect
from zipmgr import zipmgr
from importlib import reload

reload(sys)
def flatten(x):
    result = []
    for el in x:
        if isinstance(x, list) and not isinstance(el, str):
            result.extend(flatten(el))
        else:
            result.append(el)
    return result
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
        ret = p.communicate()
        return ToolsCall._out2str(ret)

    @staticmethod
    def _out2str(text_seq):
        return str.strip(" ".join(id.decode() for id in text_seq).replace("\r", "").replace("\n", ""))

    def AxmlToXml(self):
        xml_cmd = ''
        self.lastError=''

        if not os.path.exists(self.file_path):
            self.lastError = 'AXML文件路径不存在'
            return False

        length = len(self.file_path)
        self.saved_file_path = self.file_path.replace('.xml','.txt')

        if 'Windows' in platform.system():
            xml_cmd = "java -jar %s%sAXMLPrinter2.jar \"%s\" " % (self.tools_path,os.sep, self.file_path)

        if 'Linux'in platform.system():
            xml_cmd = "java -jar %s%sAXMLPrinter2.jar \"%s\" "%(self.tools_path,os.sep, self.file_path)

        try:
            result = os.popen(xml_cmd)
            content = result.read()
            if len(content)==0:
                self.lastError = 'AXML -> XML error '
                print(self.lastError)
                return False
            with open(self.saved_file_path, 'wb') as file:
                file.write(content)
                file.flush()

            if not os.path.exists(self.saved_file_path):
                self.lastError = 'AXML -> XML error '
                print(self.lastError)
                return False
        except Exception as e:
            self.lastError = str(e)
            print(self.lastError)
            return False
        return True

    def getXml(self):
        result = self.AxmlToXml()
        if result:
            print(self.saved_file_path)
            return

        print(self.lastError)

    def zipAligns(self):
        xml_cmd = ''
        self.lastError=''

        if not os.path.exists(self.file_path):
            self.lastError = 'apk文件路径不存在'
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
            print(self.saved_file_path)
            return

        print(self.lastError)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("输入的参数不正常")
    else:
        dit = {'file_path':'','tool_path':''}
        dit['file_path'] = sys.argv[1]
        dit['tool_path'] = sys.argv[2]
        flag = sys.argv[3]

        tc = ToolsCall(dit)
        if flag == 'axml':
            tc.getXml()
        elif flag == 'align':
            tc.apkAlign()
        else:
            pass


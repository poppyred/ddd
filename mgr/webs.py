import soaplib
import os
import sys
from soaplib.core.util.wsgi_wrapper import run_twisted
from soaplib.core.server import wsgi
from soaplib.core.service import DefinitionBase
from soaplib.core.service import soap
from soaplib.core.model.clazz import Array
from soaplib.core.model.binary import Attachment
from soaplib.core.model.clazz import ClassModel
from soaplib.core.model.primitive import Integer,String,Boolean
from mgr_misc import _lineno
import traceback
import msg

class C_ProbeCdrModel(ClassModel):
    __namespace__ = "C_ProbeCdrModel"
    Name=String
    Id=Integer

class HelloWorldService(DefinitionBase):
    @soap(String, _returns=String)
    def say_hello(self, name):
    	return "Hello %s!" % name

    @soap(_returns=Array(String))
    def GetCdrArray(self):
    	L_Result=["1","2","3"]
    	return L_Result

    @soap(_returns=C_ProbeCdrModel)
    def GetCdr(self):
    	#L_Obj=C_ProbeCdr()
    	L_Model=C_ProbeCdrModel()
    	L_Model.Name='ababaa'
    	L_Model.Id=1
    	return L_Model

    @soap(String, String, _returns=Integer)
    def update_view(self, ip, port):
    	#print >> sys.stderr, "python mgr.py" + " ".join(ip) + " ".join(port)
    	os.system("python mgr.py " + ip + " " + port)
    	return 1


def run_websvr(ip=msg.g_websvr_ip, port='7788'):
    try:
        pid = os.fork()
        if pid == 0:
            soap_app=soaplib.core.Application([HelloWorldService], 'dnspro')
            wsgi_app=wsgi.Application(soap_app)
            sys.stderr.write(_lineno() + ' listening on ' + ip + ':' + str(port) + '\n')
            sys.stderr.write(_lineno() + ' wsdl is at: http://' + ip + ':' + str(port) + '/dnspro/?wsdl\n')
            run_twisted( ( (wsgi_app, "dnspro"),), int(port))
    except OSError, e:
        sys.stderr.write(traceback.format_exc())

#if __name__=="__main__":
#	soap_app=soaplib.core.Application([HelloWorldService], 'tws')
#	wsgi_app=wsgi.Application(soap_app)
#	print 'listening on ' + sys.argv[1] + ':' + sys.argv[2]
#	print 'wsdl is at: http://' + sys.argv[1] + ':' + sys.argv[2] + '/SOAP/?wsdl'
#	run_twisted( ( (wsgi_app, "SOAP"),), int(sys.argv[2]))


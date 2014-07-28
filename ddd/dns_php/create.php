<?php  

ini_set('soap.wsdl_cache_enabled',"0");
require_once("msg.php"); 
require_once("SoapDiscovery.class.php");
//第一个参数是类名（生成的wsdl文件就是以它来命名的），即Service类，第二个参数是服务的名字（这个可以随便写）。 
$disco = new SoapDiscovery('dns_msg','soap');
$disco->getWSDL();
?>
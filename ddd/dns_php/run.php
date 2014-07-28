<?php
ini_set('soap.wsdl_cache_enabled',"0");
require_once(dirname(__FILE__).'/msg.php');
//生成soap服务端
$server = new SoapServer('dns_msg.wsdl', array('soap_version'=>SOAP_1_2));
$server->setClass("dns_msg"); //注册类的所有方法  
$server->handle(); //处理请求    
?>

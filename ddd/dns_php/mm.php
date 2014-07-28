<?php
	require_once("msg.php");
	
	print_r($_POST);
	
	$type = $_POST['type'];
	$view = $_POST['view'];
	$opt = $_POST['opt'];
	$domain = $_POST['domain'];
	$ip_in = $_POST['ip'];
	$ip = array();
	//$ip[] = "192.68.22.121";
	//$ip[] = "192.68.12.11";
	$ip[] = "192.68.55.71";
	//$ip [] = $ip_in;
	//$dns = new dns_msg();
	//$dns->init_target("dns");
	//$answer = $dns->send_msg(1,$type,$view,$opt,$domain,$ip);
	
	ini_set('soap.wsdl_cache_enabled',"0");
	$soap = new SoapClient("http://localhost/dnsproxy/dns_msg.wsdl");
	
	$answer = $soap->send_msg(1,1,5,1,"www.ha123231321.com",$ip);
	print_r ($answer);
	echo "answer:".$answer."\n";
   
?>

<?php
	require_once(dirname(__FILE__).'/msg.php');
	error_reporting(E_ALL^E_NOTICE);
	ini_set('soap.wsdl_cache_enabled',"0");
	
	#print_r($_POST);
	/*	
	$type = $_POST['type'];
	$view = $_POST['view'];
	$opt = $_POST['opt'];
	$domain = $_POST['domain'];
	$ip_in = $_POST['ip'];
	*/
	
	$ip = array();
	$ip [] = "121.9.13.184";
	$dns = new dns_msg();
	$dns->init_target("dns");
	$answer = $dns->send_msg(1,1,0,3,"ibss.efly.cc",$ip);
	
	//$soap = new SoapClient("http://localhost/dnsproxy/dns_msg.wsdl");
	
	//$answer = $dns->send_msg(1,1,5,1,"www.ha123231321.com",$ip);
	/*
	print_r ($answer);
	if ($answer != "01")
	{
		echo "<script language='javascript'>alert('操作失败！请检查后台服务是否正常开启！');</script>"; 
	}
	else
	{
		echo "<script language='javascript'>alert('操作成功！');</script>"; 
	}
   
	echo "<script>location.href='http://192.168.22.126/dnsproxy/tester.php';</script>";*/
	echo "answer:".$answer."\n";
	
	
   
?>

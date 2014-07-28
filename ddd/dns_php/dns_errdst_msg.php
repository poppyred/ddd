<?PHP

	require_once(dirname(__FILE__).'/msg.php');
	error_reporting(E_ALL^E_NOTICE);
	ini_set('soap.wsdl_cache_enabled',"0");
	
	$ip = $argv[1];
	
	$dns = new dns_msg();
	$dns->init_target("dns");
	$answer = $dns->init_pktlog($ip);
	
	echo $answer;
	
?>
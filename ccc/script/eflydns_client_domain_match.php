<?php

function get_whois_info($domain)
{
	$url="http://whois.hichina.com/whois/api_whois?host=$domain&oper=0";
	$html = file_get_contents($url);

	$content = json_decode($html,true);
	return $content;
}

#$array_input = array('ns1.eflydns.net','ns2.eflydns.net');
#$domain = 'eflydns.com';
$domain  = @$_POST['domain'];
$array_input = json_decode($_POST['ns']);

$ret = get_whois_info($domain);
$nameservers = $ret['module']['nameServers'];

$temp = 0;
foreach($nameservers as $nameserver)
{
	foreach($array_input as $eflydns_ns)
	{
		if (strcasecmp($nameserver,$eflydns_ns) == 0)
		{
			$temp = 1;
			break;
		}
	}
	
	if ($temp)
	{
			$temp = 1;
			break;
	}
	
}

if ($temp)
{
	print_r(result_init(0,'',''));
}
else
{
	print_r(result_init(1,"NOT CHANGE!",''));
}



function result_init($ret,$error,$data)
{
	$json_result = array();
	$json_result['ret'] = $ret;
	$json_result['error'] = $error;
	$json_result['descmap'] = $data;
	
	return json_encode($json_result);
}


?>
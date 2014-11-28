<?php

ini_set('date.timezone','Asia/Shanghai');


$eflydns_nsaddr = array('ns1.eflydns.net',
						'ns2.eflydns.net',
						'ns3.eflydns.net',
						'ns4.eflydns.net',
						'ns5.eflydns.net',
						'ns6.eflydns.net');
						
$month_array = array('jan' => '01',
					 'feb' => '02',
					 'mar' => '03',
					 'apr' => '04',
					 'may' => '05',
					 'jun' => '06',
					 'jul' => '07',
					 'aug' => '08',
					 'sep' => '09',
					 'oct' => '10',
					 'nov' => '11',
					 'dec' => '12');

function get_whois_info($domain)
{
	$url="http://whois.hichina.com/whois/api_whois?host=$domain&oper=0";
	$html = file_get_contents($url);

	$content = json_decode($html,true);
	return $content;
}

function get_mktime($date)
{
	$year=((int)substr("$date",0,4));//取得年份
	$month=((int)substr("$date",5,2));//取得月份
	$day=((int)substr("$date",8,2));//取得几号
	
	return mktime(0,0,0,$month,$day,$year);
}						
						
function match_ns($nameservers)
{
	global $eflydns_nsaddr;
	
	$temp = 0;
	foreach($nameservers as $nameserver)
	{
		foreach($eflydns_nsaddr as $eflydns_ns)
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
		$nameservers['msg'] = '您的域名已经使用 Eflydns 的域名解析服务器';
	}
	else
	{
		$nameservers['msg'] = '请注意，您的域名尚未使用 Eflydns 的域名解析服务器';
	}
	
	return $nameservers;

}

function match_update($time)
{
	$ret = array($date);

	echo $time."\n";
	$time = strtotime($time);
	
	echo $time."\n";

	$now = time();
	echo $now."\n";
	
		
	$hours = round(($now-$time)/3600);
	echo $hours;
	exit();
	
	if ($hours <= 72)
	{
		$ret['msg'] = "该域名的最新更新时间没有超过72小时，请耐心等待(最多等待".$hours."个小时)";
	}
	else
	{
		$ret['msg'] = "";
	}
	
	return $ret;
	
}

if (isset($argv[1]))
{
	$domain = $argv[1];
	
}
else
{
	$domain = @$_POST['domain'];
}

if ($domain == '')
{
	return;
}

$ret = array();
$info = get_whois_info($domain);

$code = $info['code'];
$ret['RetMsg'] = $info['msg'];

if ($code != '1000')
{
	$ret['Ret'] = 1;
	
	print_r(json_encode($ret));
	return;
}
else
{
	$ret['Ret'] = 0;
}

$info  = $info['module'];


$ret['Updated'] = $info['updatedDate'];
#$ret['Updated'] = match_update($ret['Updated']);

$ret['Nameservers'] = $info['nameServers'];
$ret['Nameservers'] = match_ns($ret['Nameservers']);

$ret['BeginDate'] = $info['standardFormatCreationDate'];
$ret['EndDate'] = $info['standardFormatExpirationDate'];

$beginday = $info['standardFormatCreationDate'];
$endday = $info['standardFormatExpirationDate'];

$beginday=str_replace('年','-',$beginday); 
$beginday=str_replace('月','-',$beginday); 

$endday=str_replace('年','-',$endday); 
$endday=str_replace('月','-',$endday); 


$beginday = get_mktime($beginday);
$endday = get_mktime($endday);
$now = time();


$LDays = round(($endday-$now)/3600/24);
$TDays = round(($endday-$beginday)/3600/24);


$ret['LeaveDays'] = $LDays;
$ret['TotalDays'] = $TDays;


$ret['StatusInfo'] = $info['statusInfos'];
$ret['OriginalInfo'] = $info['originalInfo'];

print_r($ret);
exit();
print_r(json_encode($ret));
?>

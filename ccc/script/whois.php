<?php

ini_set('date.timezone','Asia/Shanghai');
require_once(dirname(__FILE__).'/whois.class.php');

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


function deal_updatetime($time)
{
	#echo $time;
	global $month_array;

	if (strstr($time,':') && strstr($time,'-'))
	{
		$time[10] = ' ';
		$time = substr($time,0,-1);
		return trim($time);
	}
	
	else if (strstr($time,'-'))
	{
		$time = explode('-',$time);
		$time[1] = $month_array[$time[1]];
		
		$time = "$time[2]-$time[1]-$time[0]";
		#print_r($time);
		return $time." 00:00:00";
	}
	
	#可能还有更多种情况
	else
	{
		return '';
	}
	

}

function get_updatetime($domain)
{

	$ret = array();
	$query = new Whois($domain);
	$query->whois_query();
	$date = $query->whois_get_updatetime();

	$date = deal_updatetime($date);
	$time = strtotime($date);
	

	$now = time();
	
	
	$hours = round(($now-$time)/3600);
	#echo $hours;
	if ($hours <= 72)
	{
		$ret['time'] = $date;
		$ret['msg'] = "该域名的最新更新时间没有超过72小时，请耐心等待(最多等待".(72-$hours)."个小时)";
	}
	else
	{
		$ret['time'] = $date;
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


#$ret['Updated'] = $info['updatedDate'];
$ret['Updated'] = get_updatetime($domain);

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
$ret['OriginalInfo']=str_replace("\r\n",'<br/>',$ret['OriginalInfo']); 

#print_r($ret);
print_r(json_encode($ret));
?>

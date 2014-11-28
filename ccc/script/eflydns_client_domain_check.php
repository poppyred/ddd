<?php

ini_set('date.timezone','Asia/Shanghai');
require_once(dirname(__FILE__).'/mysql.class.php');
require_once(dirname(__FILE__).'/whois.class.php');

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
	#print_r($content);
	return $content;
}

function set_domain_status($domain,$value)
{
	global $mysql;
	
	$sql = "update `zone` set `is_author` = '$value' where `domain` = '$domain'";
	#print_r($sql);
	$mysql->query($sql);

}

function set_domain_whois($zone_id,$endtime)
{
	global $mysql;
	$now = date("Y-m-d H:i:s");
	
	$sql = "update `zone_whois` set `update_time` = '$now',`expiration_time`  = '$endtime' where `zone_id` = '$zone_id'";
	#print_r($sql);
	$mysql->query($sql);
	

}

function deal_updatetime($time)
{
	#echo $time;
	global $month_array;

	if (strstr($time,':') && strstr($time,'-'))
	{
		$len = strlen($time);
		
		$time[10] = ' ';
		if ($len > 19)
		{
			
			$time = substr($time,0,-1);
			#echo $time.':'.strlen($time)."\n";
		}
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
		return '未收录';
	}
	

}

function get_endtime($query)
{

	$ret = array();
	$date = $query->whois_get_endtime();
	#print_r($date);

	$date = deal_updatetime($date);
	
	return $date;
	
}

$domains = array();

$eflydns_nsaddr = file_get_contents("./eflydns_ns.conf");
$eflydns_nsaddr =  json_decode($eflydns_nsaddr);						
$id  = @$_GET['id'];
#$id = 13;

$mysql = new MySQL();
$mysql -> opendb("dnspro_client", "utf8");

$sql = "select `domain`,`id` from `zone` where `client_id` = '$id'";

$result = $mysql->query($sql);
$num_rows = mysql_num_rows($result);
if ($num_rows == NULL)
{
    echo "数据为空";
    exit;
}
while( ($row = mysql_fetch_array($result)) ) 
{
	$domain = $row['domain'];
	$zone_id = $row['id'];
	
	$domains[$domain] = $zone_id; 
	
	
}


foreach($domains as $domain => $zone_id)
{
	$query = new Whois($domain);
	$query->whois_query();
	
	$ret = get_whois_info($domain);
	#$nameservers = $ret['module']['nameServers'];
	
	$nameservers = $query->whois_get_nameserver();
	#print_r($nameservers);
	
	#$endtime = $ret['module']['standardFormatExpirationDate'];
	
	#$endtime=str_replace('年','-',$endtime); 
	#$endtime=str_replace('月','-',$endtime); 
	#$endtime = substr($endtime,0,-3);
	#$endtime = $endtime." 00:00:00";
	
	$endtime = get_endtime($query);
	
	set_domain_whois($zone_id,$endtime);
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
		set_domain_status($domain,1);
	}
	else
	{
		set_domain_status($domain,0);
	}
	
}


?>
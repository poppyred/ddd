<?php

ini_set('date.timezone','Asia/Shanghai');
require_once(dirname(__FILE__).'/whois.class.php');
		
$eflydns_nsaddr = file_get_contents("./eflydns_ns.conf");
$eflydns_nsaddr =  json_decode($eflydns_nsaddr);		
						
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
					 
$status_array = array('clientDeleteProhibited'=> array('status'=>'clientDeleteProhibited','desc'=>'注册商设置禁止删除','tip'=>'保护域名的一种状态，域名不能被删除，可在注册商处解除该状态'),
					  'clientTransferProhibited'=> array('status'=>'clientTransferProhibited','desc'=>'注册商设置禁止转移','tip'=>'保护域名的一种状态，域名不能转移注册商，可在注册商处解除该状态'),
					  'clientUpdateProhibited'=> array('status'=>'clientUpdateProhibited','desc'=>'注册商设置禁止更新','tip'=>'域名信息(包括注册人/管理联系人/技术联系人/付费联系人/DNS)不能被修改，但可设置或修改解析记录，可在注册商处解除该状态'),
					  'serverDeleteProhibited'=> array('status'=>'serverDeleteProhibited','desc'=>'注册局设置禁止删除','tip'=>'保护域名的一种状态，域名不能被删除，需联系注册局解除该状态'),
					  'serverTransferProhibited'=> array('status'=>'serverTransferProhibited','desc'=>'注册局设置禁止转移','tip'=>'保护域名的一种状态，域名不能转移注册商，需联系注册局解除该状态'),
					  'serverUpdateProhibited'=> array('status'=>'serverUpdateProhibited','desc'=>'注册局设置禁止更新','tip'=>'域名信息(包括注册人/管理联系人/技术联系人/付费联系人/DNS)不能被修改，但可设置或修改解析记录，需联系注册局解除该状态'),
					  'clientRenewProhibited'=> array('status'=>'clientRenewProhibited','desc'=>'注册商设置禁止续费','tip'=>'域名不能被续费，通常处于该状态表示该域名处于仲裁或法院争议期，需联系注册商确认原因'),
					  'ok' => array('status'=>'ok','desc'=>'正常状态','tip'=>'可正常使用')
					  );
					
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
		return '未收录';
	}
	

}

function get_updatetime($query)
{

	$ret = array();
	$date = $query->whois_get_updatetime();

	$date = deal_updatetime($date);
	$time = strtotime($date);
	

	$now = time();
	
	
	$hours = round(($now-$time)/3600);
	#echo $hours;
	if ($hours <= 72)
	{
		$ret['time'] = $date;
		$ret['msg'] = "该域名的最新更新时间没有超过72小时，请耐心等待解析全球同步(最多等待".(72-$hours)."个小时)";
	}
	else
	{
		$ret['time'] = $date;
		$ret['msg'] = "";
	}
	
	
	return $ret;
	
}

function get_begintime($query)
{

	$ret = array();
	$date = $query->whois_get_begintime();
	#print_r($date);

	$date = deal_updatetime($date);
	
	return $date;
	
}

function get_endtime($query)
{

	$ret = array();
	$date = $query->whois_get_endtime();
	#print_r($date);

	$date = deal_updatetime($date);
	
	return $date;
	
}


function get_nameserver($query)
{

	$ret = array();
	$ret = $query->whois_get_nameserver();
	
	foreach ($ret as $key=>$ns)
	{
		$ret[$key] = strtolower($ns);
	}
	
	
	$ret = match_ns($ret);
	return $ret;
	
}


function get_status($query)
{

	global $status_array;
	
	$ret = array();
	$temp = $query->whois_get_status();
	
	foreach ($temp as $key=>$status)
	{
		$ret[] = $status_array[$status];
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

#$info = get_whois_info($domain);

$query = new Whois($domain);
$query->whois_query();

$ret['Ret'] = 0;
$ret['RetMsg'] = "";



$ret['BeginDate'] = get_begintime($query);
$ret['EndDate'] = get_endtime($query);

if ($ret['BeginDate'] == '未收录' && $ret['EndDate'] == '未收录')
{
	$ret['Ret'] = 1;
	$ret['RetMsg'] = "该域名暂无相关信息<br/>请检查域名填写格式是否正确<br/>（例：不要携带如 ‘www’ 等子域名）";
	print_r(json_encode($ret));
	return;
}

$ret['Updated'] = get_updatetime($query);
$ret['Nameservers'] = get_nameserver($query);

$beginday = $ret['BeginDate'];
$endday = $ret['EndDate'];

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

$ret['StatusInfo'] = get_status($query);
$ret['OriginalInfo'] = $query->whois_get_context();
$ret['OriginalInfo'] = '<br/>'.str_replace("\n",'<br/>',$ret['OriginalInfo']); 


#print_r($ret);
print_r(json_encode($ret));
?>

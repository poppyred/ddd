<?php

require_once(dirname(__FILE__).'/mysql.class.php');
require_once(dirname(__FILE__).'/msg.php');

error_reporting(E_ALL^E_NOTICE);


$dns = new dns_msg();
$dns->init_target("dns");

$mysql = new MySQL();
$mysql -> opendb("NewDns", "utf8");

if(domain_init())
{
	echo "domain init failed\n";
}


if(extend_init())
{
	echo "extend init failed\n";
}


if(view_init())
{
	echo "extend init failed\n";
}

if(mask_init())
{
	echo "extend init failed\n";
}

if(errdst_init())
{
	echo "errdst init failed\n";
}

if(illreq_init())
{
	echo "illreq init failed\n";
}

if(pktlog_init())
{
	echo "pktlog init failed\n";
}

return 0;


function errdst_init()
{
	global $dns,$mysql;
	
	$sql = "select * from `view_errdst` where `status` = 'true'";
	$result = $mysql->query($sql);
		
		

	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{
		$ip = $row['ip'];	
		$state = $row['status'];
		$view = $row['view_id'];
		if ($state == 'true')
		{
			$dns->send_errdst($ip,$view,1);
		}
	}
	
	return 0;
}

function illreq_init()
{
	global $dns,$mysql;
	
	$sql = "select * from `view_illreq` where `status` = 'true'";
	$result = $mysql->query($sql);
		
		

	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{
		$ip = $row['ip'];	
		$state = $row['status'];
		$view = $row['view_id'];
		if ($state == 'true')
		{
			$dns->send_illreq($ip,$view,1);
		}
	}
	
	return 0;
}


function pktlog_init()
{
	global $dns,$mysql;
	
	$sql = "select * from `config_pktlog` where `status` = 'true'";
	$result = $mysql->query($sql);
		
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{	
		$value = $row[1];
		$dns->init_pktlog($value);
	}
	
	return 0;
}


function domain_init()
{
	global $dns,$mysql;
	
	$sql = "select * from `view_domain`,`view_index` where `view_domain`.`type` = 'general' and `view_domain`.`status` = 'true' and `view_domain`.`view_id`=`view_index`.`id` and `view_index`.`status` = 'true'";
	$result = $mysql->query($sql);
		
		

	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{
		$domain_id = $row[0];
		$domain = $row[1];
		$view_id = $row[2];
		$statu = $row[4];
		
		if($statu != "true")
		{
			continue;
		}
				
				
		$ip = array();
		$sql = "select `ip` from `view_domain_ip` where `domain_id` = '$domain_id' and `status` = 'true'";
		$ret = $mysql->query($sql);
		if ($ret)
		{
			while( ($row_in = mysql_fetch_array($ret)) ) 
			{
				$ip[] = $row_in[0];
			}
			mysql_free_result($ret);
		}
		
		$dns->init_msg(1,1,$view_id,1,$domain,$ip);
	}
	
	return 0;
	
	
}


function extend_init()
{
	global $dns,$mysql;
	
	$sql = "select * from `view_domain`,`view_index` where `view_domain`.`type` = 'extend' and `view_domain`.`status` = 'true' and `view_domain`.`view_id`=`view_index`.`id` and `view_index`.`status` = 'true'";
	$result = $mysql->query($sql);
			
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{
		$domain_id = $row[0];
		$domain = $row[1];
		$view_id = $row[2];
		$statu = $row[4];
		
		if($statu != "true")
		{
			continue;
		}
				
				
		$ip = array();
		$sql = "select `ip` from `view_domain_ip` where `domain_id` = '$domain_id' and `status` = 'true'";
		$ret = $mysql->query($sql);
		if($ret)
		{
			while( ($row = mysql_fetch_array($ret)) ) 
			{
				$ip[] = $row[0];
			}
			mysql_free_result($ret);
		}
		
		$dns->init_msg(1,2,$view_id,1,$domain,$ip);
	}
	
	return 0;
}



function view_init()
{
	global $dns,$mysql;
	
	$sql = "select * from `view_index` where `status` = 'true'";
	$result = $mysql->query($sql);
			
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{
		$view_id = $row[0];
		$statu = $row[2];
		
		if($statu != "true")
		{
			continue;
		}
			
		$ip = array();
				
		$sql = "select `ip` from `view_index_ip` where `index_id` = '$view_id' and `status` = 'true'";
		$ret = $mysql->query($sql);
		if($ret)
		{
			while( ($row = mysql_fetch_array($ret)) ) 
			{
				$ip[] = $row[0];
			}
			mysql_free_result($ret);
			
		}
		
		$dns->init_msg(1,3,$view_id,1,NULL,$ip);
	}
	
	return 0;
}



function mask_init()
{
	global $dns,$mysql;
	
	$sql = "select * from `view_mask`,`view_index` where `view_mask`.`status` = 'true' and `view_mask`.`viewid` = `view_index`.`id` and `view_index`.`status` = 'true'";
	$result = $mysql->query($sql);
			
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{
		$network = $row[1];
		$view_id = $row[2];
		
		$dns->init_msg(1,4,$view_id,1,$network,NULL);
	}
	
	return 0;
}
?>



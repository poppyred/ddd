<?php

define ("DB_SERVER","127.0.0.1");
define ("DB_USERNAME","root");
define ("DB_PASSWORD","qwer");
define ("DB_DB","NewDns");

$path = dirname(__FILE__);

require_once "{$path}/mysqlconnection.php";
define ("DOMAIN_OLD_FILENAME","{$path}/domain_memory.txt");
define ("DOMAIN_DIFF_FILENAME","{$path}/domain_differ.txt");

openlog("make-domain-differ",  LOG_ODELAY, LOG_LOCAL3);

//copy_view_file();
//get_view_file();

$old_view = getVeiws(DOMAIN_OLD_FILENAME);
$new_view = get_view_from_mysql();
$diffs = compare_views($old_view,$new_view);
write_didder($diffs);

function getVeiws($file)
{
	$views = array();
	if(!is_file($file))
	{
		return $views;
	}
	$fh = fopen( $file, 'r' );
	while($line = fgets( $fh ))
	{
		if (preg_match( '/^(.*?)\|(.*?)\|(.*)$/', $line, $found ))
		{
			$ipaddrs = trim($found[3]);
			if($ipaddrs == "")
			{
				$views[$found[1]][$found[2]] = array();
				continue;
			}
			$tok = strtok($ipaddrs, " ");
			while($tok !== false) 
			{
			    $views[$found[1]][$found[2]][$tok] = 1;
			    $tok = strtok(" ");
			}
		}
	}
	fclose($fh);
	return $views;
}

function compare_views($old,$new)
{
	$diffs = array();
	$diffs["create"] = compare_create($old,$new);
	$diffs["drop"] = compare_drop($old,$new);
	$diffs["insert"] = compare_insert($old,$new);
	$diffs["remove"] = compare_remove($old,$new);
	return $diffs;
}

function compare_create($old,$new)
{
	$creates = array();
	foreach($new as $domain=>$ids)
	{
		foreach($ids as $id => $ips)
		{
			if(!isset($old[$domain][$id]))
			{
				$creates[$domain][$id] = $ips;
			}
		}
	}
	return $creates;
}

function compare_drop($old,$new)
{
	$drops = array();
	foreach($old as $domain=>$ids)
	{
		foreach($ids as $id => $ips)
		{
			if(!isset($new[$domain][$id]))
			{
				$drops[$domain][$id] = $ips;
			}
		}
	}
	return $drops;
}

function compare_insert($old,$new)
{
	$insert = array();
	foreach($new as $domain=>$ids)
	{
		foreach($ids as $id => $ips)
		{
			if(!isset($old[$domain][$id]))
			{
				continue;
			}
			
			foreach($ips as $ip=>$val)
			{
				if(!isset($old[$domain][$id][$ip]))
				{
					$insert[$domain][$id][$ip] = 1;
				}
			}
		}
	}
	return $insert;
}

function compare_remove($old,$new)
{
	$remove = array();
	foreach($old as $domain=>$ids)
	{
		foreach($ids as $id => $ips)
		{
			if(!isset($new[$domain][$id]))
			{
				continue;
			}
			
			foreach($ips as $ip=>$val)
			{
				if(!isset($new[$domain][$id][$ip]))
				{
					$remove[$domain][$id][$ip] = 1;
				}
			}
		}
	}
	return $remove;
}

function get_view_from_mysql()
{
	$views = array();
	$domain_ips = array();
	$connection = new mysqlconnection(DB_SERVER,DB_USERNAME,DB_PASSWORD);
	$connection->useDB(DB_DB);
	
	$statement = "select `domain_id`,`ip` from `view_domain_ip` where `status`='true'";
	$result = $connection->query($statement);
	while($row = mysql_fetch_array($result, MYSQL_NUM)) 
	{
		$domain_ips[$row[0]][$row[1]] = 1;
	}
	
	$statement = "select `id`,`view_id`,`domain` from `view_domain` where `status`='true' and type='general'";
	$result = $connection->query($statement);
	
	while($row = mysql_fetch_array($result, MYSQL_NUM)) 
	{
		if(isset($domain_ips[$row[0]]))
		{
			$views[$row[2]][$row[1]] = $domain_ips[$row[0]];
		}
		else
		{
			$views[$row[2]][$row[1]] = array();
		}
	}
	return $views;
}

function copy_view_file()
{
	if(is_file(DOMAIN_NEW_FILENAME))
	{
		system("cp -f ".DOMAIN_NEW_FILENAME." ".DOMAIN_OLD_FILENAME);
	}
}

function write_didder($diffs)
{
	file_put_contents(DOMAIN_DIFF_FILENAME,"");
	foreach($diffs["create"] as $domain=>$ids)
	{
		foreach($ids as $id=>$ips)
		{
			
			$output = sprintf ("1|%s|%s\n", $domain, $id);
			file_put_contents(DOMAIN_DIFF_FILENAME,$output,FILE_APPEND);
			foreach($ips as $ip=>$val)
			{
				$output = sprintf ("2|%s|%s|%s\n", $domain, $id,$ip);
				file_put_contents(DOMAIN_DIFF_FILENAME,$output,FILE_APPEND);
			}
		}
	}
	
	foreach($diffs["drop"] as $domain=>$ids)
	{
		foreach($ids as $id=>$ips)
		{
			
			$output = sprintf ("4|%s|%s\n", $domain, $id);
			file_put_contents(DOMAIN_DIFF_FILENAME,$output,FILE_APPEND);
		}
	}
	
	foreach($diffs["insert"] as $domain=>$ids)
	{
		foreach($ids as $id=>$ips)
		{
			foreach($ips as $ip=>$val)
			{
				$output = sprintf ("2|%s|%s|%s\n", $domain, $id,$ip);
				file_put_contents(DOMAIN_DIFF_FILENAME,$output,FILE_APPEND);
			}
		}
	}
	
	foreach($diffs["remove"] as $domain=>$ids)
	{
		foreach($ids as $id=>$ips)
		{
			foreach($ips as $ip=>$val)
			{
				$output = sprintf ("3|%s|%s|%s\n", $domain, $id,$ip);
				file_put_contents(DOMAIN_DIFF_FILENAME,$output,FILE_APPEND);
			}
		}
	}
}
?>

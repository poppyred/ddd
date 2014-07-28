<?php

define ("DB_SERVER","127.0.0.1");
define ("DB_USERNAME","root");
define ("DB_PASSWORD","qwer");
define ("DB_DB","NewDns");

$path = dirname(__FILE__);

require_once "{$path}/mysqlconnection.php";
define ("VIEW_OLD_FILENAME","{$path}/view_memory.txt");
define ("VIEW_DIFF_FILENAME","{$path}/view_differ.txt");

//copy_view_file();
//get_view_file();

$old_view = getVeiws(VIEW_OLD_FILENAME);

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
		if (preg_match( '/^(.*?)\|(.*)$/', $line, $found ))
		{
			$ipaddrs = trim($found[2]);
			if($ipaddrs == "")
			{
				$views[$found[1]] = array();
				continue;
			}
			$tok = strtok($ipaddrs, " ");
			while($tok !== false) 
			{
			    $views[$found[1]][$tok] = 1;
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
	foreach($new as $id=>$ips)
	{
		if(!isset($old[$id]))
		{
			$creates[$id] = $ips;
		}
	}
	return $creates;
}

function compare_drop($old,$new)
{
	$drops = array();
	foreach($old as $id=>$ips)
	{
		if(!isset($new[$id]))
		{
			$drops[$id] = $ips;
		}
	}
	return $drops;
}

function compare_insert($old,$new)
{
	$insert = array();
	foreach($new as $id=>$ips)
	{
		if(!isset($old[$id]))
		{
			continue;
		}
		
		foreach($ips as $ip=>$val)
		{
			if(!isset($old[$id][$ip]))
			{
				$insert[$id][$ip] = 1;
			}
		}
	}
	return $insert;
}

function compare_remove($old,$new)
{
	$remove = array();
	foreach($old as $id=>$ips)
	{
		if(!isset($new[$id]))
		{
			continue;
		}
		
		foreach($ips as $ip=>$val)
		{
			if(!isset($new[$id][$ip]))
			{
				$remove[$id][$ip] = 1;
			}
		}
	}
	return $remove;
}

function get_view_from_mysql()
{
	$views = array();
	$view_ips = array();
	$connection = new mysqlconnection(DB_SERVER,DB_USERNAME,DB_PASSWORD);
	$connection->useDB(DB_DB);
	
	$statement = "select `index_id`,`ip` from `view_index_ip` where `status`='true'";
	$result = $connection->query($statement);
	while($row = mysql_fetch_array($result, MYSQL_NUM)) 
	{
		$view_ips[$row[0]][$row[1]] = 1;
	}
	
	$statement = "select `id` from `view_index` where `status`='true'";
	$result = $connection->query($statement);
	
	while($row = mysql_fetch_array($result, MYSQL_NUM)) 
	{
		if(isset($view_ips[$row[0]]))
		{
			$views[$row[0]] = $view_ips[$row[0]];
		}
		else
		{
			$views[$row[0]] = array();
		}
	}
	return $views;
}

function write_didder($diffs)
{
	file_put_contents(VIEW_DIFF_FILENAME,"");
	foreach($diffs["create"] as $id=>$ips)
	{
		$output = sprintf ("1|%s\n",$id);
		file_put_contents(VIEW_DIFF_FILENAME,$output,FILE_APPEND);
		foreach($ips as $ip=>$val)
		{
			$output = sprintf ("2|%s|%s\n",$id,$ip);
			file_put_contents(VIEW_DIFF_FILENAME,$output,FILE_APPEND);
		}
	}
	
	foreach($diffs["drop"] as $id=>$ips)
	{
		$output = sprintf ("4|%s\n",$id);
		file_put_contents(VIEW_DIFF_FILENAME,$output,FILE_APPEND);
	}
	
	foreach($diffs["insert"] as $id=>$ips)
	{
		foreach($ips as $ip=>$val)
		{
			$output = sprintf ("2|%s|%s\n",$id,$ip);
			file_put_contents(VIEW_DIFF_FILENAME,$output,FILE_APPEND);
		}
	}
	
	foreach($diffs["remove"] as $id=>$ips)
	{
		foreach($ips as $ip=>$val)
		{
			$output = sprintf ("3|%s|%s\n",$id,$ip);
			file_put_contents(VIEW_DIFF_FILENAME,$output,FILE_APPEND);
		}
	}
}
?>

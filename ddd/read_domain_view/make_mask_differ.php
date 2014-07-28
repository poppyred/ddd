<?php

define ("DB_SERVER","127.0.0.1");
define ("DB_USERNAME","root");
define ("DB_PASSWORD","qwer");
define ("DB_DB","NewDns");

$path = dirname(__FILE__);

require_once "{$path}/mysqlconnection.php";
define ("MASK_OLD_FILENAME","{$path}/mask_memory.txt");
define ("MASK_DIFF_FILENAME","{$path}/mask_differ.txt");

//copy_view_file();
//get_view_file();

$old_view = getVeiws(VIEW_OLD_FILENAME);
$new_view = get_view_from_mysql();

$diffs = compare_views($old_view,$new_view);
write_didder($diffs);


function getVeiws($file)
{
	$masks = array();
	if(!is_file($file))
	{
		return $masks;
	}
	$fh = fopen( $file, 'r' );
	while($line = fgets( $fh ))
	{
		if (preg_match( '/^(.*?)\|(.*)$/', $line, $found ))
		{
			$ipaddr = trim($found[1]);
			$viewid = trim($found[2]);
			$masks[$ipaddr] = $viewid;
		}
	}
	fclose($fh);
	return $masks;
}

function compare_views($old,$new)
{
	$diffs = array();
	$diffs["create"] = compare_create($old,$new);
	$diffs["drop"] = compare_drop($old,$new);
	return $diffs;
}

function compare_create($old,$new)
{
	$creates = array();
	foreach($new as $mask => $id)
	{
		if(!isset($old[$mask]))
		{
			$creates[$mask] = $id;
		}
	}
	return $creates;
}

function compare_drop($old,$new)
{
	$drops = array();
	foreach($old as $mask => $id)
	{
		if(!isset($new[$mask]))
		{
			$drops[$mask] = $id;
		}
	}
	return $drops;
}

function get_view_from_mysql()
{
	$masks = array();
	$connection = new mysqlconnection(DB_SERVER,DB_USERNAME,DB_PASSWORD);
	$connection->useDB(DB_DB);
	
	$statement = "select `network`,`viewid` from `view_mask`";
	$result = $connection->query($statement);
	while($row = mysql_fetch_array($result, MYSQL_NUM)) 
	{
		$ipaddr = trim($row[0]);
		$id = trim($row[1]);
		$masks[$ipaddr] = $id;
	}
	return $masks;
}

function write_didder($diffs)
{
	file_put_contents(MASK_DIFF_FILENAME,"");
	foreach($diffs["create"] as $mask=>$id)
	{
		$output = sprintf ("1|%s|%s\n",$mask,$id);
		file_put_contents(MASK_DIFF_FILENAME,$output,FILE_APPEND);
	}
	
	foreach($diffs["drop"] as $mask=>$id)
	{
		$output = sprintf ("4|%s\n",$mask);
		file_put_contents(MASK_DIFF_FILENAME,$output,FILE_APPEND);
	}
}
?>

<?php

define ("DB_SERVER","127.0.0.1");
define ("DB_USERNAME","root");
define ("DB_PASSWORD","qwer");
define ("DB_DB","EflyDns");
define ("DEFAULT_IP","0.0.0.0");

$path = dirname(__FILE__);

require_once "{$path}/mysqlconnection.php";
define ("INDEX_OLD_FILENAME","{$path}/index_memory.txt");
define ("INDEX_NEW_FILENAME","{$path}/index_mysql.txt");
define ("INDEX_DIFF_FILENAME","{$path}/index_differ.txt");

openlog("make-index-differ",  LOG_ODELAY, LOG_LOCAL3);

//copy_view_file();
get_view_file();

$old_view = getVeiws(INDEX_OLD_FILENAME);
$new_view = getVeiws(INDEX_NEW_FILENAME);
$diffs = compare_views($old_view,$new_view);
write_differ($diffs);

closelog();

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
		if (preg_match( '/^id:(.*?)\|view_ip:(.*?)\|view_action:(.*?)\|$/', $line, $found ))
		{
			$views[$found[1]]["view_ip"] = $found[2];
			$views[$found[1]]["view_action"] = $found[3];
		}
	}
	fclose($fh);
	return $views;
}

function compare_views($old,$new)
{
	$diffs = array();
	$diffs["add"] = compare_add($old,$new);
	$diffs["del"] = compare_del($old,$new);
	$diffs["update"] = compare_update($old,$new);
	return $diffs;
}

function compare_add($old,$new)
{
	$adds = array();
	foreach($new as $id=>$view)
	{
		if(empty($old[$id]))
		{
			$adds[$id] = $view;
		}
	}
	return $adds;
}

function compare_del($old,$new)
{
	$dels = array();
	foreach($old as $id=>$view)
	{
		if(empty($new[$id]))
		{
			$dels[$id] = $view;
		}
	}
	return $dels;
}

function compare_update($old,$new)
{
	$updates = array();
	foreach($new as $id=>$view)
	{
		if(!empty($old[$id]))
		{
			if($old[$id]["view_action"]!=$new[$id]["view_action"])
			{
				$updates[$id] = $view;
			}
		}
	}
	return $updates;
}

function get_view_file()
{
	$connection = new mysqlconnection(DB_SERVER,DB_USERNAME,DB_PASSWORD);
	$connection->useDB(DB_DB);
	$statement = "select `id`,`view_ip`,`view_action` from `view_index` where `status`='true'";
	$result = $connection->query($statement);
	
	$output = "";
	file_put_contents(INDEX_NEW_FILENAME,$output);
	while ($row = mysql_fetch_array($result, MYSQL_NUM)) 
	{
		$output = sprintf ("id:%s|view_ip:%s|view_action:%s|\n", $row[0], $row[1],$row[2]);
		file_put_contents(INDEX_NEW_FILENAME,$output,FILE_APPEND);
		//echo $output;
	}
}

function copy_view_file()
{
	if(is_file(INDEX_NEW_FILENAME))
	{
		system("cp -f ".INDEX_NEW_FILENAME." ".INDEX_OLD_FILENAME);
	}
}

function write_differ($diffs)
{
	file_put_contents(INDEX_DIFF_FILENAME,"");
	foreach($diffs["del"] as $id=>$view)
	{
		$output = sprintf ("del|id:%s|domain:%s|direct_ip:%s|\n", $id, DEFAULT_IP,$view["view_action"]);
		file_put_contents(INDEX_DIFF_FILENAME,$output,FILE_APPEND);
		syslog(LOG_INFO, $output);
	}
	
	foreach($diffs["add"] as $id=>$view)
	{
		$output = sprintf ("add|id:%s|domain:%s|direct_ip:%s|\n", $id, DEFAULT_IP,$view["view_action"]);
		file_put_contents(INDEX_DIFF_FILENAME,$output,FILE_APPEND);
		syslog(LOG_INFO, $output);
	}
	
	foreach($diffs["update"] as $id=>$view)
	{
		$output = sprintf ("update|id:%s|domain:%s|direct_ip:%s|\n", $id, DEFAULT_IP,$view["view_action"]);
		file_put_contents(INDEX_DIFF_FILENAME,$output,FILE_APPEND);
		syslog(LOG_INFO, $output);
	}
}
?>

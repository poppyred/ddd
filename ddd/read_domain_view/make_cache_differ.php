<?php
error_reporting(E_ERROR);
date_default_timezone_set('Asia/Manila');
$yestoday = getYestoday();
define ("DEFAULT_CACHE_FILE","http://127.0.0.1/dns_domain_rank/default/default.txt");
define ("DATE_CACHE_FILE","http://127.0.0.1/dns_domain_rank/date/{$yestoday}");

$path = dirname(__FILE__);

define ("INDEX_OLD_FILENAME","{$path}/cache_memory.txt");
define ("INDEX_DIFF_FILENAME","{$path}/cache_differ.txt");

openlog("make-cache-differ",  LOG_ODELAY, LOG_LOCAL3);

$new_cache = getCacheFromHttp();
if($new_cache == false)
{
	file_put_contents(INDEX_DIFF_FILENAME,"");
	echo "get http file fail.\n";
	return;
}

$old_cache = getCacheFromFile();

$diffs = compare_cache($old_cache,$new_cache);
write_differ($diffs);

closelog();

function getCacheFromHttp()
{
	$caches = array();
	$content = file_get_contents(DATE_CACHE_FILE);
	if($content == false)
	{
		$content = file_get_contents(DEFAULT_CACHE_FILE);
	}
	
	if($content == false)
	{
		return false;
	}
	
	$tok = strtok($content, "\n");
	while ($tok !== false) 
	{
    	$domain = trim($tok);
		if($domain != "")
		{
			$caches[$domain] = 1;
		}
    	$tok = strtok("\n");
	}
	return $caches;
}

function getCacheFromFile()
{
	$caches = array();
	$content = file_get_contents(INDEX_OLD_FILENAME);
	
	$tok = strtok($content, "\n");
	while ($tok !== false) 
	{
		$domain = trim($tok);
		if($domain != "")
		{
			$caches[$domain] = 1;
		}
    	$tok = strtok("\n");
	}
	return $caches;
}

function compare_cache($old,$new)
{
	$diffs = array();
	$diffs["add"] = compare_add($old,$new);
	$diffs["del"] = compare_del($old,$new);
	return $diffs;
}

function compare_add($old,$new)
{
	$adds = array();
	foreach($new as $domain=>$id)
	{
		if(empty($old[$domain]))
		{
			$adds[] = $domain;
		}
	}
	return $adds;
}

function compare_del($old,$new)
{
	$dels = array();
	foreach($old as $domain=>$id)
	{
		if(empty($new[$domain]))
		{
			$dels[] = $domain;
		}
	}
	return $dels;
}

function write_differ($diffs)
{
	file_put_contents(INDEX_DIFF_FILENAME,"");
	foreach($diffs["del"] as $domain)
	{
		$output = sprintf ("del|id:%s|domain:%s|direct_ip:%s|\n", "0",$domain, "null");
		file_put_contents(INDEX_DIFF_FILENAME,$output,FILE_APPEND);
		syslog(LOG_INFO, $output);
	}
	
	foreach($diffs["add"] as $domain)
	{
		$output = sprintf ("add|id:%s|domain:%s|direct_ip:%s|\n", "0",$domain, "null");
		file_put_contents(INDEX_DIFF_FILENAME,$output,FILE_APPEND);
		syslog(LOG_INFO, $output);
	}
}

function getYestoday()
{
        return date( "Ymd",time()-2*24*3600);
}
?>

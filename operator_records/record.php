<?php

$filename = $argv[1];
$domain = "dprktimes.com";
$replace_domain = ".".$domain;

$fd = @fopen($filename, "r") or die("Couldn't open file"); ;

$arr = array();
$cnt = 0;

while($s=fgets($fd,1048)){ 
$cnt ++;
#echo "$cnt:$s - ";

$s = split(' ',$s);
print_r($s);
$s[0] = trim($s[0]);
#$rev = strrev($s[0]);
$record = substr($s[0],0,-1);

$value = trim($s[1]);

#echo "$record:$value\n";


$arr[$record] = $value;
#$value = substr($value,0,-1);
$record =  str_replace($replace_domain, '', $record);

$url ="http://api.efly.cc/eflydns/record.php?opt=insert&user=yanfa@efly.cc&domain=".$domain."&host=".$record."&type=A&view=联通&val=".$value."&ttl=60";
echo $url."\n";

$ret = file_get_contents($url);
$ret = json_decode($ret,true);
print_r($ret);
usleep(1000);






#$len = strlen($rev);
#$i = 0;
#$temp =0;

/*
for ($i = 0; $i < $len; $i ++)
{

	#echo $rev[$i]."\n";
	if($rev[$i] == '.')
	{
		$temp ++;
	}
	#echo $temp;
	if($temp == 3 )
	{
		$rev = substr($rev,0,$i+1);
		#echo $rev;
		$rev = strrev($rev);
		$length = strlen($rev);
		$rev =  substr($rev,1,($length-1));
		#echo "result:".$rev;
		break;
	}
	
	if ($i == $len-1)
	{
		$rev = substr($rev,0,$i+1);
		#echo $rev;
		$rev = strrev($rev);
		#echo "result:".$rev;
		break;
		
	}
	
}

echo $rev."\n";
if (isset($arr[$rev]))
{
	$arr[$rev] ++;
}
else
{
	$arr[$rev] = 0;
}

*/
}
#print_r($arr);

#print_r($s);
 
#break;


?>


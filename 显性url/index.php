<?php
#echo "123";
#$url = "www.baidu.com";
#Header("Location:$url"); 

require_once(dirname(__FILE__).'/mysql.class.php');
ini_set('date.timezone','Asia/Shanghai');

$request = $_SERVER['HTTP_HOST'];

$mysql = new MySQL();
$mysql -> opendb("dns_http_ref", "utf8");

$sql = "select * from `http_ref` where `src` = '$request' ";

	$result = $mysql->query($sql);
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL)
    {
        echo "数据为空";
        exit;
    }
	while( ($row = mysql_fetch_array($result)) ) 
	{
		#print_r($row);
		$dst = $row['dst']; 
	}
	
	
$url = "http://".$dst;
#$url="http://www.eflydns.com"; 
echo "<script language=\"javascript\">"; 
echo "window.location= \"$url\"";
echo "</script>"

?>



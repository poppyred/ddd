<?php

	ini_set('date.timezone','Asia/Shanghai');
	require_once(dirname(__FILE__).'/mysql.class.php');

	$ns = array();
	
	$mysql = new MySQL();
	$mysql -> opendb("dnspro_client", "utf8");
	
	$sql = "select `val` from `nameserver`";

	$result = $mysql->query($sql);
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL)
    {
        echo "数据为空";
        exit;
    }
	while( ($row = mysql_fetch_array($result)) ) 
	{
	
		$ns[] = $row['val']; 
	
	}
	
	$ns = json_encode($ns);
	
	$handle = fopen("./eflydns_ns.conf", "w+");
	if (fwrite($handle, $ns) === FALSE) {
        echo "不能写入到文件 $filename\n";
        exit;
    }
	else
	{
		echo "更新成功\n";
	}
	
	fclose($handle);


?>



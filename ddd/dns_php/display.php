<?php

	require_once(dirname(__FILE__).'/mysql.class.php');

	
	$cnt = 0;
	$mysql = new MySQL();
	$mysql -> opendb("NewDns", "utf8");
	$id = 0;
	$page = 1;
	$i = 0;
	$type = $_GET['type'];
	
	if(isset($_GET['page']))
	{
		$id = 25*($_GET['page'] -1);
		$page = $_GET['page'];
	}
	
	if(isset($_GET['type']))
	{
	switch($_GET['type'])
	{
	case 1:
		$table = "view_domain";
		echo "	<div class='container'>
		<table class='table table-hover table-bordered table-striped ' >
		<caption><button type='button' class='btn btn-block btn-large btn-danger disabled' disabled='disabled'><i class='icon-info-sign icon-white'></i> 域名记录</button></caption>
		<thead>
		<tr>
		<th>序号</th>
		<th>域名</th>
		<th>对应视图</th>
		<th>劫持指向</th>
		<th>状态</th>
		</tr>
		</thead>
		<tbody>";
		
		$sql = "select * from `view_domain` where `type` = 'general'";
		$result = $mysql->query($sql);
			
		$num_rows = mysql_num_rows($result);
		if ($num_rows == NULL){return -1;}
		while( ($row = mysql_fetch_array($result)) ) 
		{
			$cnt ++;
		
			if ($cnt > (25*($page-1)) && $cnt <= (25*$page))
			{
				$domain_id = $row[0];
				$domain = $row[1];
				$view_id = $row[2];
				$statu = $row[4];
				$id ++;
				
				
				$ip = "";
				$sql = "select `ip` from `view_domain_ip` where `domain_id` = '$domain_id'";
				$ret = $mysql->query($sql);
				while( ($row = mysql_fetch_array($ret)) ) 
				{
					$ip .="[";
					$ip .= $row[0];
					
					$ip .="]   ";
				}
				
				echo "<tr class='warning'> 
				<td>$id</td>
				<td>$domain</td>
				<td>$view_id</td>
				<td>$ip</td>
				<td>$statu</td>
				</tr>";
				
			if ($id%25 == 0)
			{
				break;
			}
			}
		}
			
	
		mysql_free_result($result);
		break;
	case 2:
		$table = "view_domain";
		echo "	<div class='container'>
		<table class='table table-hover table-bordered table-striped ' >
		<caption><button type='button' class='btn btn-block btn-large btn-danger disabled' disabled='disabled'><i class='icon-info-sign icon-white'></i> 泛解析记录</button></caption>
		<thead>
		<tr>
		<th>序号</th>
		<th>域名</th>
		<th>对应视图</th>
		<th>劫持指向</th>
		<th>状态</th>
		</tr>
		</thead>
		<tbody>";
		
		$sql = "select * from `view_domain` where `type` = 'extend'";
		$result = $mysql->query($sql);
			
		$num_rows = mysql_num_rows($result);
		if ($num_rows == NULL){return -1;}
		while( ($row = mysql_fetch_array($result)) ) 
		{
			$cnt ++;
		
			if ($cnt > (25*($page-1)) && $cnt <= (25*$page))
			{
				$domain_id = $row[0];
				$domain = $row[1];
				$view_id = $row[2];
				$statu = $row[4];
				$id ++;
				$ip = "";
				
				$sql = "select `ip` from `view_domain_ip` where `domain_id` = '$domain_id'";
				$ret = $mysql->query($sql);
				while( ($row = mysql_fetch_array($ret)) ) 
				{
					$ip .="[";
					$ip .= $row[0];
					
					$ip .="]   ";
				}
				
				echo "<tr class='warning'> 
				<td>$id</td>
				<td>$domain</td>
				<td>$view_id</td>
				<td>$ip</td>
				<td>$statu</td>
				</tr>";
				
				
			if ($id%25 == 0)
			{
				break;
			}
			}
		}
			
	
		mysql_free_result($result);
		break;
		

	case 3:
		$table = "view_index";
		echo "	<div class='container'>
		<table class='table table-hover table-bordered table-striped ' >
		<caption><button type='button' class='btn btn-block btn-large btn-danger disabled' disabled='disabled'><i class='icon-info-sign icon-white'></i> 视图记录</button></caption>
		<thead>
		<tr>
		<th>视图序号</th>
		<th>视图名称</th>
		<th>指向DNS服务器</th>
		<th>状态</th>
		</tr>
		</thead>
		<tbody>";
		$sql = "select * from `view_index`";
		$result = $mysql->query($sql);
			
		$num_rows = mysql_num_rows($result);
		if ($num_rows == NULL){return -1;}
		while( ($row = mysql_fetch_array($result)) ) 
		{
			$cnt ++;
		
			if ($cnt > (25*($page-1)) && $cnt <= (25*$page))
			{
				$name = $row[1];
				$view_id = $row[0];
				$statu = $row[2];
				$id ++;
				
				$ip = "";
				
				$sql = "select `ip` from `view_index_ip` where `index_id` = '$view_id'";
				$ret = $mysql->query($sql);
				while( ($row = mysql_fetch_array($ret)) ) 
				{
					$ip .="[";
					$ip .= $row[0];
					
					$ip .="]   ";
				}
				
				echo "<tr class='warning'> 
				<td>$view_id</td>
				<td>$name</td>
				<td>$ip</td>
				<td>$statu</td>
				</tr>";
				
				
			if ($id%25 == 0)
			{
				break;
			}
			}
		}
			
	
	mysql_free_result($result);
		
		
		break;
	case 4:
		$table = "view_mask";
		echo "	<div class='container'>
		<table class='table table-hover table-bordered table-striped ' >
		<caption><button type='button' class='btn btn-block btn-large btn-danger disabled' disabled='disabled'><i class='icon-info-sign icon-white'></i> 掩码信息</button></caption>
		<thead>
		<tr>
		<th>序号</th>
		<th>掩码</th>
		<th>对应视图</th>
		</tr>
		</thead>
		<tbody>";
		$sql = "select * from `view_mask`";
		$result = $mysql->query($sql);
			
		$num_rows = mysql_num_rows($result);
		if ($num_rows == NULL){return -1;}
		while( ($row = mysql_fetch_array($result)) ) 
		{
			$cnt ++;
		
			if ($cnt > (25*($page-1)) && $cnt <= (25*$page))
			{
			$network = $row[1];
			$view_id = $row[2];
			$id ++;
			echo "<tr class='warning'> 
				<td>$id</td>
				<td>$network</td>
				<td>$view_id</td>
				</tr>";
				
			if ($id%25 == 0)
			{
				break;
			}
			}
		}
			
	
	mysql_free_result($result);
		break;
	default:
		break;
		}
	}
	
	
	
	
	else
	{

		$table = "view_domain";
		echo "	<div class='container'>
		<table class='table table-hover table-bordered table-striped ' >
		<caption><button type='button' class='btn btn-block btn-large btn-danger disabled' disabled='disabled'><i class='icon-info-sign icon-white'></i> 域名记录</button></caption>
		<thead>
		<tr>
		<th>序号</th>
		<th>域名</th>
		<th>对应视图</th>
		<th>劫持指向</th>
		<th>状态</th>
		</tr>
		</thead>
		<tbody>";
		
	$sql = "select * from `view_domain` where `type` = 'general'";
		$result = $mysql->query($sql);
			
		$num_rows = mysql_num_rows($result);
		if ($num_rows == NULL){return -1;}
		while( ($row = mysql_fetch_array($result)) ) 
		{
			$cnt ++;
		
			if ($cnt > (25*($page-1)) && $cnt <= (25*$page))
			{
				$domain_id = $row[0];
				$domain = $row[1];
				$view_id = $row[2];
				$statu = $row[4];
				$id ++;
				
				
				$ip = "";
				$sql = "select `ip` from `view_domain_ip` where `domain_id` = '$domain_id'";
				$ret = $mysql->query($sql);
				while( ($row = mysql_fetch_array($ret)) ) 
				{
					$ip .="[";
					$ip .= $row[0];
					
					$ip .="]   ";
				}
				
				echo "<tr class='warning'> 
				<td>$id</td>
				<td>$domain</td>
				<td>$view_id</td>
				<td>$ip</td>
				<td>$statu</td>
				</tr>";
				
				
				if ($id%25 == 0)
				{
					break;
				}
			}
		}
			
	
		mysql_free_result($result);
	}
	



	
	
	$sql = "select count(*) from `$table`";
	$result = $mysql->query($sql);
	$num_rows = mysql_num_rows($result);
	if ($num_rows == NULL){return -1;}
	while( ($row = mysql_fetch_array($result)) ) 
	{
		$total = $row[0];
	}
	
	$max = ($total/25 +1);
	
	
	echo"</tbody>
		</table>";
		
	echo "<div class='pagination pagination-centered'>
		<ul>";
		
	echo "<li><a href='tester.php?page=1&type=$type'><<</a></li>";
	$temp = $page -2;
	for ( $i = 0 ; $i <  5; $i++)
	{
		if($temp >= $max)
		{
			break;
		}
		
		if ($temp <= 0)
		{
			$temp ++;
			continue;
		}
		
		if($temp == $page)
		{
			echo "<li class = 'disabled'><a href='#'>$page</a></li>";
		}
		else
		{
			echo "<li><a href='tester.php?page=$temp&type=$type'>$temp</a></li>";
		}
		$temp ++;
		
	}

	echo "
	</ul>
	</div>
	</div>";


?>
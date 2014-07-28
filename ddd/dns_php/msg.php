<?php
	require_once(dirname(__FILE__).'/mysql.class.php');
	
   
	class dns_msg
	{
		private $tar = "dns";

		public function init_target($tar)
		{
			$this->tar = $tar;
		}
		
		
		public function send_msg($msg_id,$type, $vid, $opt, $domain, $ip)
		{
			$tar = $this->tar;
			if (!file_exists("/tmp/$tar.sock"))
			{
				echo "dns_proxy is power off";
				
				return 0;
			}
		
			
			
			
			$fp = fsockopen ("unix:///tmp/$tar.sock", NULL ,&$errno, &$errstr);  

			$msg = "";
			$msg = $this->make_msg($msg_id,$type, $vid, $opt, $domain, $ip);
			
			//echo "send:".$msg."\n";
			fwrite($fp,$msg);
			$answer = fread($fp,5);
			fclose($fp);
			
			//echo $answer."\n";
			$answer = rtrim($answer);
			switch($answer)
			{
			case "01":
				$this->deal_mysql($type, $vid, $opt, $domain, $ip);
				break;
				
			case "02":
				break;
			
			default:
				echo " this is wrong answer!\n";
				break;
			}
			
			return $answer;
		}
		
		
		public function init_msg($msg_id,$type, $vid, $opt, $domain, $ip)
		{
			$tar = $this->tar;
			$fp = fsockopen ("unix:///tmp/$tar.sock", NULL ,&$errno, &$errstr);  

			$msg = "";
			$msg = $this->make_msg($msg_id,$type, $vid, $opt, $domain, $ip);
			
			#echo "send:".$msg."\n";
			fwrite($fp,$msg);
			$answer = fread($fp,5);
			fclose($fp);
			return $answer;
		}
		
		public function send_errdst($ip,$view,$opt)
		{
			$tar = $this->tar;
			$fp = fsockopen ("unix:///tmp/$tar.sock", NULL ,&$errno, &$errstr);  

			$msg = "";
			$msg = $this->make_errdst(2,$ip,$view,$opt);

			fwrite($fp,$msg);
			$answer = fread($fp,5);
			fclose($fp);
			
			return $answer;
		}
		
		public function send_illreq($ip,$view,$opt)
		{
			$tar = $this->tar;
			$fp = fsockopen ("unix:///tmp/$tar.sock", NULL ,&$errno, &$errstr);  

			$msg = "";
			$msg = $this->make_illreq(4,$ip,$view,$opt);

			fwrite($fp,$msg);
			$answer = fread($fp,5);
			fclose($fp);
			
			return $answer;
		}
		
		public function init_pktlog($ip)
		{
			$tar = $this->tar;
			$fp = fsockopen ("unix:///tmp/$tar.sock", NULL ,&$errno, &$errstr);  

			$msg = "";
			$msg = $this->make_pktlog(3,$ip);

			fwrite($fp,$msg);
			$answer = fread($fp,5);
			fclose($fp);
			
			return $answer;
		}

		
		private function deal_mysql($type, $vid, $opt, $domain, $ip)
		{
			switch($type)
			{
			case '1':
				switch($opt)
				{
				case '1':
					$this->create_domain($domain, $vid, $ip, "general");
					break;
				case '2':
					$this->insert_domain($domain, $vid, $ip, "general");
					break;
				case '3':
					$this->remove_domain($domain, $vid, $ip, "general");
					break;
				case '4':
					$this->modify_domain($domain, $vid, $ip, "general");
					break;
				case '5':
					$this->restart_ip($domain,$vid, $ip, "general");
					break;
				case '6':
					$this->stop_ip($domain,$vid, $ip, "general");
					break;
				case '8':
					$this->drop_domain($domain,$vid,"general");
					break;
				default:
					break;
				}
				break;
			
			case '2':
				switch($opt)
				{
				case '1':
					$this->create_domain($domain, $vid, $ip, "extend");
					break;
				case '2':
					$this->insert_domain($domain, $vid, $ip, "extend");
					break;
				case '3':
					$this->remove_domain($domain, $vid, $ip, "extend");
					break;
				case '4':
					$this->modify_domain($domain, $vid, $ip, "extend");
					break;
				case '5':
					$this->restart_ip($domain, $vid, $ip, "extend");
					break;
				case '6':
					$this->stop_ip($domain, $vid, $ip, "extend");
					break;
				case '8':
					$this->drop_domain($domain,$vid,"extend");
					break;
				default:
					break;
				}
				break;
				
				
			case '3':
				switch($opt)
				{
				case '1':
					$this->create_view($vid, $ip);
					break;
				case '2':
					$this->drop_view($vid);	
					break;
				case '3':
					$this->stop_view($vid);	
					break;
				case '4':
					$this->restart_view($vid);	
					break;
				case '5':
					$this->insert_viewip($vid, $ip);
					break;
				case '6':
					$this->remove_viewip($vid, $ip);
					break;
				case '7':
					$this->modify_viewip($vid, $ip);
					break;
				case '8':
					$this->restart_viewip($vid, $ip);
					break;
				case '9':
					$this->stop_viewip($vid, $ip);
					break;
				default:
					break;
				}
				break;
			case '4':
				switch($opt)
				{
				case '1':
					$this->create_mask($domain, $vid, $ip);
					break;
				case '2':
					$this->stop_mask($domain, $vid, $ip);
					break;
				case '3':
					$this->restart_mask($domain, $vid, $ip);
					break;
				case '4':
					$this->drop_mask($domain, $vid, $ip);
					break;
				default:
					break;
				}
				break;
			default:
				break;
				
			}
		
			return;
		}
		
		/* ----------------------------------------------------------------------------------------------------------------------- */
		
		
		private function create_domain($domain, $vid, $ips, $type)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "insert into `view_domain` values('','$domain', '$vid','$type','true')";
			$db->query($sql);
			
			//print $sql;
			
			$sql = '';
			$sql = "select `id` from `view_domain` where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
			$result = $db->query($sql);
			
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
			}
			
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "insert into `view_domain_ip` values('$id','$ip','true',CURRENT_TIMESTAMP)";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		
		private function modify_domain($domain, $vid,$ips,$type)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "insert into `view_domain` values('','$domain', '$vid','$type','true')";
			$db->query($sql);
			
			//print $sql;
			
			$sql = '';
			$sql = "select `id` from `view_domain` where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
			$result = $db->query($sql);
			
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
			}
			
			#print $id."\n";
			$sql = '';
			$sql = "update `view_domain_ip` set `ip` = '$ips[1]' where `ip` = '$ips[0]' and `domain_id` = '$id'";
			$db->query($sql);
			
			#print "$sql\n";
		
			unset($db);
			return 0;
		}
		
		
		private function drop_domain($domain, $vid, $type)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			
			$sql = "select `id` from `view_domain` where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
			$result = $db->query($sql);
			
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
			}
			
			$sql = '';
			$sql = "delete from `view_domain` where `id` = '$id'";
			$db->query($sql);
			
	
			$sql = '';
			$sql = "delete from `view_domain_ip` where `domain_id` = '$id'";
			$db->query($sql);
		
		
			unset($db);
			return 0;
		}
		
		private function insert_domain($domain, $vid, $ips, $type)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "insert into `view_domain` values('','$domain', '$vid','$type','true')";
			$db->query($sql);
			
			$sql = '';
			
			$sql = "select `id` from `view_domain`  where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
			$result = $db->query($sql);
			
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
			}
			
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "insert into `view_domain_ip` values('$id','$ip','true',CURRENT_TIMESTAMP)";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		
		private function remove_domain($domain, $vid, $ips, $type)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "select `id` from `view_domain`  where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
			$result = $db->query($sql);
			
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
			}
			
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "delete from `view_domain_ip` where `domain_id` = '$id' and `ip` = '$ip'";
				$db->query($sql);
			}
			
			$sql = '';
			$sql = "select count(*) from `view_domain_ip`  where `domain_id` = '$id'";
			$result = $db->query($sql);
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$cnt = $row[0];
			}
			
			if ($cnt == 0)
			{
				$sql = '';
				$sql = "delete from `view_domain` where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		
		private function drop_domain_all($domain,$type)
		{
			$views = array();
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "select * from `view_domain` where `domain` = '$domain' and `type` = '$type'";
			$result = $db->query($sql);
			#print_r($sql);
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
			
				#print_r($row);
				$view_id = $row[2];
				$views[] = $view_id;
				
			}
			unset($db);
			
			foreach ($views as $key => $view)
			$this->drop_domain($domain,$view,$type);
			
			
			return 0;
		}
		
		private function restart_ip($domain, $vid, $ips, $type)
		{
			$views = array();
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "select `id` from `view_domain`  where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
			#print_r($sql);
			$result = $db->query($sql);
			
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
			}
			
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "update `view_domain_ip` set `status` = 'true' where `ip` = '$ip' and `domain_id` = '$id'";
				#print_r($sql);
				$db->query($sql);
			}
			
			unset($db);
			return 0;
		}
		
		
		private function stop_ip($domain, $vid, $ips, $type)
		{
			$views = array();
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "select `id` from `view_domain`  where `domain` = '$domain' and `view_id` = '$vid' and `type` = '$type'";
			$result = $db->query($sql);
			
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
			}
			
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "update `view_domain_ip` set `status` = 'false' where `ip` = '$ip' and `domain_id` = '$id'";
				$db->query($sql);
			}
			
			unset($db);
			return 0;
		}
		
		/* ----------------------------------------------------------------------------------------------------------------------- */
		
		
		private function create_view($vid, $ips)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "insert into `view_index` values('$vid',' ','true',' ')";
			#print_r($sql);
			$db->query($sql);
				
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "insert into `view_index_ip` values('$vid','$ip','true',CURRENT_TIMESTAMP)";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		private function drop_view($vid)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = '';
			$sql = "delete from `view_index` where `id` = '$vid'";
			$db->query($sql);
			
	
			$sql = '';
			$sql = "delete from `view_index_ip` where `index_id` = '$vid'";
			$db->query($sql);
			
			/*
			$sql = '';
			$sql = "select * from `view_domain` where `view_id` = '$vid'";
			$result = $db->query($sql);
			$num_rows = mysql_num_rows($result);
			if ($num_rows == NULL){return -1;}
			while( ($row = mysql_fetch_array($result)) ) 
			{
				$id = $row[0];
				$sql = '';
				$sql = "delete  from `view_domain_ip` where `domain_id` = '$id'";
				$db->query($sql);
				
			}
			*/
		
			$sql = '';
			$sql = "delete  from `view_domain` where `view_id` = '$vid'";
			$db->query($sql);
		
			$sql = '';
			$sql = "delete  from `view_mask` where `viewid` = '$vid'";
			$db->query($sql);
			
			unset($db);
			return 0;
		}
		
		private function stop_view($vid)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = '';
			$sql = "update `view_index` set `status` = 'false' where `id` = '$vid'";
			$db->query($sql);
		
			$sql = '';
			$sql = "update `view_mask` set `status` = 'false' where `viewid` = '$vid'";
			$db->query($sql); 
			
			unset($db);
			return 0;
		}
		
		private function restart_view($vid)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = '';
			$sql = "update `view_index` set `status` = 'true' where `id` = '$vid'";
			$db->query($sql);
			
			
			unset($db);
			return 0;
		}
		
		private function insert_viewip($vid, $ips)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "insert into `view_index_ip` values('$vid','$ip','true',CURRENT_TIMESTAMP)";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		private function remove_viewip($vid, $ips)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
		
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "delete from `view_index_ip` where `index_id` = '$vid' and `ip` = '$ip'";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		private function stop_viewip($vid, $ips)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
		
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "update `view_index_ip` set `status` = 'false' where `index_id` = '$vid' and `ip` = '$ip'";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		private function restart_viewip($vid, $ips)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
		
			foreach($ips as $ip)
			{
				//$ip = inet_ntop($ip);
				$sql = '';
				$sql = "update `view_index_ip` set `status` = 'true' where `index_id` = '$vid' and `ip` = '$ip'";
				$db->query($sql);
			}
		
			unset($db);
			return 0;
		}
		
		
		private function modify_viewip($vid, $ips)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
		

				//$ip = inet_ntop($ip);
			$sql = '';
			$sql = "update `view_index_ip` set `ip` = '$ips[1]' where `index_id` = '$vid' and `ip` = '$ips[0]'";
			$db->query($sql);
	
		
			unset($db);
			return 0;
		}
		
		/* ----------------------------------------------------------------------------------------------------------------------- */
		
		private function create_mask($domain, $vid)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "insert into `view_mask` values('','$domain','$vid','true')";
			$db->query($sql);
			
			//print $sql;
			unset($db);
			return 0;
		}
		private function restart_mask($domain, $vid)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "update `view_mask` set `status` = 'true' where `network` = '$domain' and `viewid` = '$vid'";
			$db->query($sql);
			
			unset($db);
			return 0;
		}
		
		private function stop_mask($domain, $vid)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "update `view_mask` set `status` = 'false' where `network` = '$domain' and `viewid` = '$vid'";
			$db->query($sql);
			
			unset($db);
			return 0;
		}
		
		private function drop_mask($domain, $vid)
		{
			$db = new Mysql();
			$db -> opendb("NewDns", "utf8");
			
			$sql = "delete from `view_mask` where `network` = '$domain' and `viewid` = '$vid'";
			$db->query($sql);
			
			unset($db);
			return 0;
		}

		/* ----------------------------------------------------------------------------------------------------------------------- */
		
		private function make_msg($msg_id, $type, $vid, $opt, $domain, $ip)
		{
			$msg = "";

			$domain_len = strlen($domain);
			$msg = pack("SCSCS",$msg_id,$type,$vid,$opt,$domain_len);
			$domain_len = sprintf("%03d",$domain_len);

			if ($domain_len != 0)
			{
				$msg .= $domain;
			}
			

 
			$ip_num = count($ip);
			$msg .=  pack("S",$ip_num);
			
			/*
			print_r($ip);
			echo "type:".$type."\n";*/
			if ($ip_num != 0)
			{
				foreach($ip as $value)
				{
					$value = trim($value);
					$msg .= inet_pton($value);
				}
		
			}
		
			return $msg;
	
		}
		
		/* ----------------------------------------------------------------------------------------------------------------------- */
		private function make_errdst($msg_id,$ip,$view,$opt)
		{
			$msg = "";
			$msg = pack("SCC",$msg_id,$opt,$view);
			if ($ip != NULL)
			{
				$ip = trim($ip);
				$msg .= inet_pton($ip);
			}
	
			return $msg;
	
		}
		
		/* ----------------------------------------------------------------------------------------------------------------------- */
		private function make_illreq($msg_id,$ip,$view,$opt)
		{
			$msg = "";
			$msg = pack("SCC",$msg_id,$opt,$view);
			if ($ip != NULL)
			{
				$ip = trim($ip);
				$msg .= inet_pton($ip);
			}
	
			return $msg;
	
		}
		
		/* ----------------------------------------------------------------------------------------------------------------------- */
		
		private function make_pktlog($msg_id, $ip)
		{
			if ($ip == NULL)
			{
				echo "wrong input!\n";
				return "0";
			}	
 
			$msg = "";
			$msg = pack("S",$msg_id);
			$ip = trim($ip);
			$msg .= inet_pton($ip);
	
			return $msg;
	
		}
	 
	}
?>

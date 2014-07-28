<?php
	/*
	$ip = array();
	$ip[] = inet_pton("192.68.22.101");
	$ip[] = inet_pton("192.68.12.123");
	$ip[] = inet_pton("192.68.55.70");
	$dns = new dns_ev_msg();
	$dns->init_target("dns");
	$answer = $dns->send_msg(1,2,3,"www.rjkj.com",$ip);
   
	echo "answer:".$answer."\n";
   
   */
   
	class dns_msg
	{
		private $tar = "dns";

		public function init_target($tar)
		{
			$this->tar = $tar;
		}
		
		
		public function send_msg($type, $vid, $opt, $domain, $ip)
		{
			$tar = $this->tar;
			$fp = fsockopen ("unix:///tmp/$tar.sock", NULL ,&$errno, &$errstr);  

			$msg = "";
			$msg = $this->make_msg($type, $vid, $opt, $domain, $ip);
			
			fwrite($fp,$msg);
			$answer = fread($fp,5);
			fclose($fp);
			
			return $answer;
		}

		static function make_msg($type, $vid, $opt, $domain, $ip)
		{
			if ($type > 4 || $type < 1 || $opt > 4 || $opt < 1 || $vid < 0 || $vid > 99)
			{
				return "0";
			}	
 
			$msg = "";
			$vid = sprintf("%02d",$vid);
			$msg = "$type$vid$opt";
			$domain_len = strlen($domain);
			$domain_len = sprintf("%03d",$domain_len);
			$msg .= $domain_len;
			if ($domain_len != 0)
			{
				$msg .= $domain;
			}
			echo $msg."\n";
 
			$ip_num = count($ip);
			$ip_num = sprintf("%03d",$ip_num);
			$msg .= $ip_num;
			if ($ip_num != 0)
			{
				foreach($ip as $value)
				{
					$msg .= $value;
				}
		
			}
	
			echo $msg."\n";
			echo strlen($msg)."\n";
		
			return $msg;
	
		}

	}
?>

<?php
			if (isset($argv[1]))
			{
				$tar = $argv[1];
			}
			else
			{
				return;
			}

			$i = 0;
			for ($i =0 ; $i <1;$i ++)
			{
			$fp = fsockopen ("10.10.12.66", 12345 ,&$errno, &$errstr);  

			$ip = $tar;
			$msg = "";
			$msg = pack("S",6);
			$msg.=$ip;
			#$domain_len = sprintf("%03d",$domain_len);
			
			//echo "send:".$msg."\n";
			fwrite($fp,$msg);
			//$answer = fread($fp,5);
			fclose($fp);
			}

?>


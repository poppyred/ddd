<?php
function url_exists($url)   
{  
   $head = get_headers($url);  
   return $head[0];  
}


$ch = curl_init('http://dnspod.cn');
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
curl_exec($ch);
echo curl_getinfo($ch, CURLINFO_HTTP_CODE); // 200
curl_close($ch);

/*
$url = "http://www.baids1u.com";
$ret = url_exists($url);
$ret = explode(' ',$ret);
print_r($ret);
*/


?>

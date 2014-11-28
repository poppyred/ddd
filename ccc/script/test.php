<?php
$handle = exec("nslookup -type=ns baidu.com", $result, $output);
print_r($result);
print_r("\r\n");
print_r($output);
?>

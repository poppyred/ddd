<?php

$ret = file_get_contents("http://api.efly.cc/eflydns/addDomain.php?opt=insert&user=yanfa@efly.cc&domain=dns21.org");
$ret = json_decode($ret,true);
print_r($ret);
?>


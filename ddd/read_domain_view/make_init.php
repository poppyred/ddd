<?php
$path = dirname(__FILE__);
define ("DOMAIN_FILENAMEs","{$path}/domain_*.txt");
define ("INDEX_FILENAMEs","{$path}/index_*.txt");
define ("MASK_FILENAMEs","{$path}/mask_*.txt");
define ("CACHE_FILENAMEs","{$path}/cache_*.txt");

system("rm -rf ".DOMAIN_FILENAMEs);
system("rm -rf ".INDEX_FILENAMEs);
system("rm -rf ".MASK_FILENAMEs);
system("rm -rf ".CACHE_FILENAMEs);
?>

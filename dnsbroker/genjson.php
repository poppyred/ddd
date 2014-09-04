<?php
print("http://121.201.11.6/dnspro/dnsbroker/\n");
$x = array("data"=>array("type"=>"record", "opt"=>"get", 'data'=>array("sid"=>"node_mgr_primary_zs_bgp")));
print json_encode($x);
print("\n");
$x = array( '1'=>array('ret'=>0, 'result'=>'task id 1 succ', 'error'=>''), 
			'2'=>array('ret'=>0, 'result'=>'task id 2 succ', 'error'=>''),
			'3'=>array('ret'=>0, 'result'=>'task id 3 succ', 'error'=>''));
$x = array("data"=>array("type"=>"record", "opt"=>"set", 'data'=>array("sid"=>"node_mgr_primary_zs_bgp", "tasks"=>$x)));
print json_encode($x);
print("\n");

?>

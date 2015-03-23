<?php
$post = json_decode($_POST['data'], TRUE);
#print_r($post);

$post_type = $post['type'];
$post_opt = $post['opt'];
$post_data = $post['data'];

$mod_file = 'mod_'.$post_type.'.php';
clearstatcache();
if( file_exists($mod_file) ) {
	include $mod_file;
	do_post($post);
} else {
	$ret = array('ret' => 1 , 'error' => "module $mod_file not found", 'result' => array() );
	print_r(json_encode($ret));
	exit;
}



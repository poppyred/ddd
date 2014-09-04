<?php
require_once('common.php');
require_once('log.php');

function do_post($post) 
{
	dnspro_start_log();
	$post_opt = $post['opt'];
	dnspro_log(LOG_DEBUG, serialize($post));
	dnspro_close_log();
	#print_r($post);
	switch($post_opt)
	{
		case 'add': do_add_domain($post); break;
		case 'del': do_del_domain($post); break;
		case 'set': do_set_domain($post); break;
		case 'get': do_get_domain($post); break;
	}
}

function do_add_domain($post) 
{
	//nothing to do
	exit;
}

function do_del_domain($post) 
{
	$post_type = $post['type'];
	$post_opt = $post['opt'];
	$post_data = $post['data'];
	$post_user = $post['user'];
	$userid = $post_user['cid'];

	if( isset( $post_data['name'] ) )
	{
		//web post del A record task
		create_common_task($post, $userid, $post_type, $post_opt, json_encode($post_data));
		exit;
	}
}

function do_set_domain($post) 
{
	#print_r($post);
	$post_type = $post['type'];
	$post_opt = $post['opt'];
	$post_data = $post['data'];

	if( isset($post['user']) ) {
		//web post set record task
		$userid = $post['user']['cid'];
		create_common_task($post, $userid, $post_type, $post_opt, json_encode($post_data));
		exit;
	}

	if( isset($post_data['sid']) ) {
		//nodemgr post task result
		do_nodemgr_set_common_task($post);
		exit;
	}
}

function do_get_domain($post) 
{
	#print_r($post);
	$post_type = $post['type'];
	$post_opt = $post['opt'];
	$post_data = $post['data'];

	if( isset($post_data['sid']) ) {
		//nodemg get task list
		do_nodemgr_get_common_task($post);
		exit;
	}
}





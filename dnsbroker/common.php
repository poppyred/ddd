<?php
require_once('db.class.php');
require_once('log.php');

function create_common_task($post, $userid, $task_type, $task_opt, $task_data) 
{
	#print_r($post);
	$db = new DBObj();
	$db->conn(DBS_MAIN);
	$db->select_db(DB_DNSPRO);
	$sql = "SELECT * FROM `server_list` where type = 'node_mgr';";
	$result = $db->query($sql);
	$sids = array();
	while( ($row = mysql_fetch_array($result)) ) {
		$sid = $row['sid'];
		$sids[$sid] = $sid;
	}
	mysql_free_result($result);

	$timeout = 30;
	$retry = 1;
	$status = 'init';

	$sql = "insert into `task_list`(`cid`, `type`, `opt`, `timeout`, `retry`, `data`, `status`, `starttime`) 
			values($userid, '$task_type', '$task_opt', $timeout, $retry, '$task_data', '$status', now())";
	$db->query($sql);
	if( $db->affected_rows() != 1 ) {
		do_fail(1, 'insert into task_list fail '.$sql);
	}
	$taskid = $db->get_last_id();

	$sql = '';
	foreach( $sids as $sid ) {
		$sql = "insert into `task_feedback`(`taskid`, `sid`, `timeout`, `retry`, `tried`, `status`, `starttime`) 
            	values($taskid, '$sid', $timeout, $retry, 0, '$status', now());";
		$db->query($sql);
	}
	if( $db->affected_rows() <= 0 ) {
        do_fail(1, 'insert into task_feedback fail '.$sql);
    }

	$ret = array('ret' => 0, 'error' => '', 'result' => array() );
	print_r(json_encode($ret));
}

function do_nodemgr_set_common_task($post) 
{
	//print_r($post);
	$post_type = $post['type'];
	$post_opt = $post['opt'];
	$post_data = $post['data'];

	$sid = $post_data['sid'];
	$tasks = $post_data['tasks'];
	//print_r($sid);print_r($tasks);

	$db = new DBObj();
	$db->conn(DBS_MAIN);
	$db->select_db(DB_DNSPRO);
	$sql = '';
	foreach( $tasks as $taskid => $taskinfo) {
		$status = $taskinfo['ret'] == 0 ? 'finish' : 'init';
		$sql = "update `task_feedback` set 
				`tried` = `tried` + 1,
				`ret` = $taskinfo[ret], 
				`result` = '$taskinfo[result]', 
				`error` = '$taskinfo[error]', 
				`status` = '$status',
				`finishtime` = now() 
				where `taskid` = $taskid and `sid` = '$sid';";

		$db->query($sql);
	}
	//check task_list task was all finish?
	$taskids = array();
	$sql = "select `id` from `task_list` where `status` = 'init';"; 
	$result = $db->query($sql);
	while( ($row = mysql_fetch_array($result)) ) {
		$taskid = $row['id'];
		$taskids[$taskid] = $taskid;
	}
	mysql_free_result($result);
	foreach( $taskids as $taskid ) {
		$sql = "select count(*) from `task_feedback` where `taskid` = $taskid and `status` != 'finish';";
		$result = $db->query($sql);
		$row = mysql_fetch_array($result);
		mysql_free_result($result);
		if( $row[0] == 0 ) {
			$sql = "update `task_list` set `status` = 'finish', `finishtime` = now() where `id` = $taskid;";
			$db->query($sql);
		}
	}

	$ret = array('ret' => 0, 'error' => '', 'result' => '' );
	print_r(json_encode($ret));
}

function do_nodemgr_get_common_task($post) 
{
	#print_r($post);
	$post_type = $post['type'];
	$post_opt = $post['opt'];
	$post_data = $post['data'];

	$sid = $post_data['sid'];

	$db = new DBObj();
	$db->conn(DBS_MAIN);
	$db->select_db(DB_DNSPRO);
	$sql = "select * from `task_list` where 
			`type` = '$post_type' and 
			`status` = 'init' and 
			`id` in (SELECT taskid FROM `task_feedback` where 
				`sid` = '$sid' and `status` = 'init'
				and `task_feedback`.`tried` <= `task_feedback`.`retry`)";
	$result = $db->query($sql);
	$tasks = array();
	while( ($row = mysql_fetch_array($result)) ) {
		$task_id = $row['id'];
		$task_type = $row['type'];
		$task_opt = $row['opt'];
		$task_data = $row['data'];
		$tasks[$task_id] = array('type' => $task_type, 'opt' => $task_opt, 'data' => $task_data);
	}
	mysql_free_result($result);

	$ret = array('ret' => 0, 'error' => '', 'result' => $tasks );
	print_r(json_encode($ret));
}

function do_fail($ret, $error)
{
    $ret = array('ret' => $ret, 'error' => $error, 'result' => array() );
    print_r(json_encode($ret));
	exit;
}




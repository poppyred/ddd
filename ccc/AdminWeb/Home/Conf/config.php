<?php
return array(
	//'配置项'=>'配置值'
	'URL_MODEL'	=>	2,
	'APP_DEBUG' 	=>  	true,
	'DB_TYPE'	=>	'mysql',
	'DB_HOST'	=>	'121.201.12.60',
	'DB_NAME'	=>	'dnspro',
	'DB_USER'	=>	'root',
	'DB_PWD'	=>	'rjkj@rjkj',
	'DB_PORT'	=>	'3306',
	'DB_PREFIX'     =>      '',    //数据库表前缀
	'SERVICE_URL'   =>      'http://121.201.11.7:8080', //自定义配置参数项
	'INTERFACE_URL' =>      'http://121.201.12.60',
	'NS_CHECH_URL'  =>      'http://121.201.12.61',
	'API_URL' => 'http://api.efly.cc/eflydns',
	
	'DB_NEWS'	=>      array(
		'DB_TYPE'=>'mysql',
		'DB_USER'=>'root',
		'DB_PWD'=>'rjkj@rjkj',
		'DB_HOST'=>'121.201.12.60',
		'DB_PORT'=>'3306',
		'DB_PREFIX'=>'',
		'DB_NAME'=>'dnspro_client'
	)
);

?>

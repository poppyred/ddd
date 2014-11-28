<?php

/*mysql 操作类*/
/*author:hyb*/

class MySQL 
{
    private $host;//服务器
    private $dbname;//数据库名
    private $db;//数据库连接句柄
    private $link;//连接名
    public  $errmsg;//错误信息
    
    public function __construct()
	{
		$this -> open();
    }
	
	//应用析构函数自动释放连接资源
    public function __destruct()
	{ 
		mysql_close($this -> link);
	}
    
    //连接主机
    private function open()
	{	
		$mysql_arr = array();
		$mysql_arr = $this -> set_host();
				
		$this -> link = mysql_connect($mysql_arr[0],$mysql_arr[1],$mysql_arr[2]) or 
	    		  							die (mysql_error());
    }
    
    //设置数据库服务器
    private function set_host()
	{
		$host_arr = array();

		$host_arr[0] = "121.201.12.60";
		//$host_arr[0] = "192.168.22.126";
		$host_arr[1] = "root";
		$host_arr[2] = "rjkj@rjkj";

		return $host_arr;
    }
    
    public function to2DArray($result)
	{
		$_2DArray = Array();
		$arr = new ArrayObject($_2DArray);
		while($row = mysql_fetch_array($result))
		{
			$arr -> append($row);
		}
		return $arr ;
    }
    
    //连接数据库
    public function opendb($database,$charset)
	{
		$this -> dbname = $database;
		mysql_query("set names ".$charset);//设置字符集
		$this -> db = mysql_select_db( $this -> dbname, $this -> link );
		if (!$this -> db)
		{
			$this -> errmsg = "连接数据库错误！";
		}
    }
    
    public function query($sql) 
	{
		$result = mysql_query($sql);
		if ( !$result )
		{
			$this -> errmsg = "运行错误!";
		}
		return $result;
    }
    
    //错误方法吸收
    public function __call($n,$v)
	{
		return "不存在".$n."()方法";
    }
}
?>

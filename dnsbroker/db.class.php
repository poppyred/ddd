<?php
define ("DB_DNSPRO", "dnspro");
define ("DBS_MAIN", "maindb");

class DBObj
{
	public $link = null;

	private $_dbinfo = array(
						DBS_MAIN => array('ip' => 'localhost', 'port' => 3306, 'user' => 'root', 'passwd' => 'rjkj@rjkj' )
						); 

	private $_dbserver = null;
	
	function __construct() 
	{

	}

	function __destruct() 
	{
		if( $this->link && ! is_null($this->link) ) {
			mysql_close($this->link);		
		}
	}
	
	function conn($dbserver)
	{
		$this->_dbserver = $dbserver;
		$ip = $this->_dbinfo[$this->_dbserver]['ip'];
		$port = $this->_dbinfo[$this->_dbserver]['port'];
		$server = "$ip:$port";
  		$this->link = @mysql_pconnect($server,
									$this->_dbinfo[$this->_dbserver]['user'],
									$this->_dbinfo[$this->_dbserver]['passwd']);
  		return $this->link;
	}

	function select_db($dbname)
	{
		return mysql_select_db($dbname, $this->link);
	}
	
	function query($string)
	{
		if( ! $this->link ) {
			return false;
		}
		return @mysql_query($string, $this->link);
	}
	
	function get_last_id()
	{
		return @mysql_insert_id($this->link);
	}
	
	function affected_rows()
	{
		return @mysql_affected_rows($this->link);
	}
	
	function error()
	{
		return mysql_error($this->link);
	}
}





<?php

class mysqlconnection
{
	private $connection;
	
	private $server;
	private $username;
	private $password;

	public $connectable;

	function __construct($server,$username,$password)
	{
		$this->server = $server;
		$this->username = $username;
		$this->password = $password;
		$this->connectable = $this->connect();
	}
	
	public function connect()
	{
		if($this->server == "")
		{
			throw new Exception("database server can no be null");
		}
		else
		{
			$this->connection = mysql_pconnect($this->server,$this->username,$this->password);
			if(!$this->connection)
			{
				//throw new Exception('Could not connect to server '.$this->server.':'.mysql_error());
				echo 'Could not connect to server '.$this->server.':'.mysql_error()."\n";
				return false;
			}
		}
		return true;
	}


	public function useDB($db)
	{
		if(!$this->connection)
		{
			return;
		}
		$result = mysql_select_db($db,$this->connection ); 
		if(!$result)
		{
			throw new Exception("unable to use database ".$db.":".mysql_error());
		}
	}

	public function query($statement)
	{
		if(!$this->connection)
		{
			return;
		}

		$result =  mysql_query($statement,$this->connection);
		if($result)
		{
			return $result;
		}
		else
		{
			throw new Exception("mysql query error:\n".$statement."\n".mysql_error());
		}
	}
}
?>

<?php

error_reporting(0);

Class MyMongo
{
	#private $host;//服务器
    private $conn;//数据库名
    private $db;//数据库连接句柄
    private $collection;//连接名
	
   # public  $collection;//错误信息
    
    public function __construct($target)
	{
		$this->connect($target);
    }
	
	public function __destruct()
	{ 
		$this->conn->close();
		#echo "close mongoDB!\n";
	}
	
	public function connect($target)
	{
		$this->conn = new Mongo($target);
		$this->db = $this->conn->dnspro;
		$this->collection = $this->db->reqcnt;
		
		$regex = new MongoRegex("/eflypro.com/");
	}
   
   	public function find($query)
	{
		return $this->collection->find($query);
    }
	
	public function update($query)
	{
		return $this->collection->update($query);
    }
	
	public function remove($query)
	{
		return $this->collection->remove($query);
    }
   
   
   /*当天流量*/
   public function get_today_reqcnt($domain)
   {
   
		$ret = array();
		$i = 0;
		for ($i = 0; $i < 24; $i++)
		{
			$ret[$i."时"] = 0;
		}
		
		$now = date("Y-m-d");
		$now = split('-',$now);
		
		$year = $now[0];
		$month = $now[1];
		$day = $now[2];
		
		$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month,'day'=>$day));

		foreach ($cursor as $elements) 
		{
			#print_r($elements);
	  
			#$domain = $elements['domain'];
			$month = $elements['month'];
			$day = $elements['day'];
			$hour = $elements['hour'];
			$req = $elements['req'];
			#$domain = $elements['domain'];
			
			$h = sprintf("%d", $hour);
			$h.='时';
			
			$ret[$h] +=$req;
		}
		
		#print_r($ret);
		return $ret;
   }
   
   /*本月流量*/
   public function get_month_reqcnt($domain)
   {
   
		$ret = array();
		$i = 0;
		
		$now = date("Y-m-d");
		$now = split('-',$now);
		
		$year = $now[0];
		$month = $now[1];
		$day = $now[2];
		
		if ($month == '01' || $month == '03' || $month == '05' || $month == '07' || $month == '08' || $month == '10' || $month == '12')
		{
			$max = 31;
		}
		else if ($month == '02')
		{
			$max = 28;
		}
		else
		{
			$max = 30;
		}
		
		for ($i = 1; $i <= $max; $i++)
		{
			$d = sprintf("%02d", $i);
			$ret[$month."-".$d] = 0;
		}
		
		$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month));

		foreach ($cursor as $elements) 
		{
			#print_r($elements);
	  
			#$domain = $elements['domain'];
			$month = $elements['month'];
			$day = $elements['day'];
			$hour = $elements['hour'];
			$req = $elements['req'];
			#$domain = $elements['domain'];
			
			$ret[$month."-".$day] +=$req;
		}
		
		#print_r($ret);
		return $ret;
   }
   
   
   /*最近七天流量*/
   public function get_rweek_reqcnt($domain)
   {
   
		$days = array();
		$months = array();
		$years = array();
		
		$ret = array();
		$i = 0;
		
		$now = date("Y-m-d");
		$now = split('-',$now);

		$year = $now[0];
		
		$begin = date('Y-m-d',time()-86400*6);
		$begin = split('-',$now);
		$byear = $begin[0];
		
		if ($byear != $year)
		{
			$years[] = $year;
			$years[] = $byear;
		}
		else
		{
			$years[] = $year;
		}
		
		
		

		for ($i = 6; $i >= 0; $i--)
		{
			$time = date('m-d',time()-86400*$i);
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$month = $time[0];
			$day = $time[1];
			
			$days[] = $day;
			if (count($months) == 0)
			{
				$months[] = $month;
			}
			else
			{
				foreach ($months as $temp)
				{
					if ($month == $temp)
					{
						continue;
					}
					else
					{
						$months[] = $month;
					}
				}
			}
		
		}
		
		
		/*有一个隐藏bug，跨年*/
		$qdays = array('$in'=>$days);
		$qmonths = array('$in'=>$months);
		$qyears = array('$in'=>$years);
		
		$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$qyears,'month'=>$qmonths,'day'=>$qdays));

		foreach ($cursor as $elements) 
		{
			#print_r($elements);
	  
			#$domain = $elements['domain'];
			$month = $elements['month'];
			$day = $elements['day'];
			$hour = $elements['hour'];
			$req = $elements['req'];
			#$domain = $elements['domain'];
			
			if (isset($ret[$month."-".$day]))
			{
				$ret[$month."-".$day] +=$req;
			}
		}
		#exit();
		#print_r($ret);
		return $ret;
   }
   
   
   /*最近一个月流量*/
   public function get_rmonth_reqcnt($domain)
   {
   
		$days = array();
		$months = array();
		$years = array();
		
		$ret = array();
		$i = 0;
		
		$now = date("Y-m-d");
		$now = split('-',$now);

		$year = $now[0];
		
		$begin = date('Y-m-d',time()-86400*30);
		$begin = split('-',$now);
		$byear = $begin[0];
		
		if ($byear != $year)
		{
			$years[] = $year;
			$years[] = $byear;
		}
		else
		{
			$years[] = $year;
		}
		
		
		

		for ($i = 30; $i >= 0; $i--)
		{
			$time = date('m-d',time()-86400*$i);
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$month = $time[0];
			$day = $time[1];
			
			$days[] = $day;
			if (count($months) == 0)
			{
				$months[] = $month;
			}
			else
			{
				foreach ($months as $temp)
				{
					if ($month == $temp)
					{
						continue;
					}
					else
					{
						$months[] = $month;
					}
				}
			}
		
		}
		
		
		/*有一个隐藏bug，跨年*/
		$qdays = array('$in'=>$days);
		$qmonths = array('$in'=>$months);
		$qyears = array('$in'=>$years);
		
		$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$qyears,'month'=>$qmonths,'day'=>$qdays));

		foreach ($cursor as $elements) 
		{
			#print_r($elements);
	  
			#$domain = $elements['domain'];
			$month = $elements['month'];
			$day = $elements['day'];
			$hour = $elements['hour'];
			$req = $elements['req'];
			#$domain = $elements['domain'];
			
			if (isset($ret[$month."-".$day]))
			{
				$ret[$month."-".$day] +=$req;
			}
		}
		#exit();
		#print_r($ret);
		return $ret;
   }
   
   
    /*最近一个季度流量*/
	public function get_rseason_reqcnt($domain)
	{   	
	
		$ret = array();

		for ($i = 2; $i > 0; $i--)
		{
			$time = date("Y-m",strtotime("-$i month"));	
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$year = $time[0];
			$month = $time[1];
			
			$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month));

			foreach ($cursor as $elements) 
			{
				#print_r($elements);
	  
				#$domain = $elements['domain'];
				$month = $elements['month'];
				$day = $elements['day'];
				$hour = $elements['hour'];
				$req = $elements['req'];
				#$domain = $elements['domain'];
			
	
				$ret[$year."-".$month] +=$req;

			}	
		
		}
		
			$time = date("Y-m");	
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$year = $time[0];
			$month = $time[1];
			
			$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month));

			foreach ($cursor as $elements) 
			{
				#print_r($elements);
	  
				#$domain = $elements['domain'];
				$month = $elements['month'];
				$day = $elements['day'];
				$hour = $elements['hour'];
				$req = $elements['req'];
				#$domain = $elements['domain'];
		
				$ret[$year."-".$month] +=$req;
			}	
		#print_r($ret);
		
		return $ret;
   }
   
   
    /*最近半年流量*/
	public function get_rhalf_reqcnt($domain)
	{   	
	
		$ret = array();
	
		for ($i = 5; $i > 0; $i--)
		{
			$time = date("Y-m",strtotime("-$i month"));	
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$year = $time[0];
			$month = $time[1];
			
			$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month));

			foreach ($cursor as $elements) 
			{
				#print_r($elements);
	  
				#$domain = $elements['domain'];
				$month = $elements['month'];
				$day = $elements['day'];
				$hour = $elements['hour'];
				$req = $elements['req'];
				#$domain = $elements['domain'];
			
	
				$ret[$year."-".$month] +=$req;

			}	
		
		}
		
			$time = date("Y-m");	
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$year = $time[0];
			$month = $time[1];
			
			$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month));

			foreach ($cursor as $elements) 
			{
				##print_r($elements);
	  
				#$domain = $elements['domain'];
				$month = $elements['month'];
				$day = $elements['day'];
				$hour = $elements['hour'];
				$req = $elements['req'];
				#$domain = $elements['domain'];
		
				$ret[$year."-".$month] +=$req;
			}	
		#print_r($ret);
		return $ret;
   }
   
    /*最近一年流量*/
	public function get_ryear_reqcnt($domain)
	{   	
	
		$ret = array();
	
		for ($i = 11; $i > 0; $i--)
		{
			$time = date("Y-m",strtotime("-$i month"));	
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$year = $time[0];
			$month = $time[1];
			
			$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month));

			foreach ($cursor as $elements) 
			{
				#print_r($elements);
	  
				#$domain = $elements['domain'];
				$month = $elements['month'];
				$day = $elements['day'];
				$hour = $elements['hour'];
				$req = $elements['req'];
				#$domain = $elements['domain'];
			
	
				$ret[$year."-".$month] +=$req;

			}	
		
		}
		
			$time = date("Y-m");	
			$ret[$time] = 0;
			
			$time = split('-',$time);
			$year = $time[0];
			$month = $time[1];
			
			$cursor = $this->collection->find(array('domain'=>$domain,'year'=>$year,'month'=>$month));

			foreach ($cursor as $elements) 
			{
				#print_r($elements);
	  
				#$domain = $elements['domain'];
				$month = $elements['month'];
				$day = $elements['day'];
				$hour = $elements['hour'];
				$req = $elements['req'];
				#$domain = $elements['domain'];
		
				$ret[$year."-".$month] +=$req;
			}	
		#print_r($ret);
		return $ret;
   }
   
}


?>

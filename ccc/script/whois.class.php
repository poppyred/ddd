<?php
ini_set('date.timezone','Asia/Shanghai');

class Whois
{
	private $domain;
	private $context;
	private $updatetime;
	private $endtime;
	private $begintime;
	private $status = array();
	private $nameserver = array();
	
	public function __construct($init)
	{
		$this -> domain = $init;
    }
	
	public function whois_query() 
	{
 
		// fix the domain name:	
		$domain = strtolower(trim($this->domain));
		$domain = preg_replace('/^http:\/\//i', '', $domain);
		$domain = preg_replace('/^www\./i', '', $domain);
		$domain = explode('/', $domain);
		$domain = trim($domain[0]);
 
		// split the TLD from domain name
		$_domain = explode('.', $domain);
		$lst = count($_domain)-1;
		$ext = $_domain[$lst];
 
		// You find resources and lists 
		// like these on wikipedia: 
		//
		// http://de.wikipedia.org/wiki/Whois
		//
		
		$servers = array(
			"biz" => "whois.neulevel.biz",
			"com" => "whois.internic.net",
			"us" => "whois.nic.us",
			"coop" => "whois.nic.coop",
			"info" => "whois.nic.info",
			"name" => "whois.nic.name",
			"net" => "whois.internic.net",
			"gov" => "whois.nic.gov",
			"edu" => "whois.internic.net",
			"mil" => "rs.internic.net",
			"int" => "whois.iana.org",
			"ac" => "whois.nic.ac",
			"ae" => "whois.uaenic.ae",
			"at" => "whois.ripe.net",
			"au" => "whois.aunic.net",
			"be" => "whois.dns.be",
			"bg" => "whois.ripe.net",
			"br" => "whois.registro.br",
			"bz" => "whois.belizenic.bz",
			"ca" => "whois.cira.ca",
			"cc" => "whois.nic.cc",
			"ch" => "whois.nic.ch",
			"cl" => "whois.nic.cl",
			"cn" => "whois.cnnic.net.cn",
			"cz" => "whois.nic.cz",
			"de" => "whois.nic.de",
			"fr" => "whois.nic.fr",
			"hu" => "whois.nic.hu",
			"ie" => "whois.domainregistry.ie",
			"il" => "whois.isoc.org.il",
			"in" => "whois.ncst.ernet.in",
			"ir" => "whois.nic.ir",
			"mc" => "whois.ripe.net",
			"to" => "whois.tonic.to",
			"tv" => "whois.tv",
			"ru" => "whois.ripn.net",
			"org" => "whois.pir.org",
			"aero" => "whois.information.aero",
			"nl" => "whois.domain-registry.nl"
		);
 
		if (!isset($servers[$ext]))
		{
			die('Error: No matching nic server found!');
		}
 
		$nic_server = $servers[$ext];
	
		if ($ext == 'com' || $ext == 'net')
		{
			$domain = "=".$domain;
		}
 
		$output = '';
    // connect to whois server:
		if ($conn = fsockopen ($nic_server, 43)) 
		{
			fputs($conn, $domain."\r\n");
			while(!feof($conn)) {
				$output .= fgets($conn,128);
			}
			fclose($conn);
		}
		else
		{
			die('Error: Could not connect to ' . $nic_server . '!'); 
		}
 
		$this->context = $output;
		
		$ret = trim($output);
		$array = array_unique(explode(',', str_replace("\n",",",$ret)));
		foreach($array as $content)
		{
			substr($content,3);
			if ($temp = strstr($content,'Updated Date'))
			{
				$temp = trim($temp);
				$temp = substr($temp,14);
				#echo $content;
				$this -> updatetime = $temp;
				#print_r($temp);
			}
			
			
			if ($temp = strstr($content,'Name Server'))
			{
				$temp = trim($temp);
				$temp = substr($temp,13);
				#echo $content;
				$this -> nameserver[] = $temp;
				#print_r($this -> nameserver);
			}
			
			if ($temp = strstr($content,'Creation Date'))
			{
				$temp = trim($temp);
				$temp = substr($temp,15);
				#echo $content;
				$this -> begintime = $temp;

			}
			
			if ($temp = strstr($content,'Registration Date'))
			{
				$temp = trim($temp);
				$temp = substr($temp,19);
				#echo $content;
				$this -> begintime = $temp;

			}
			
			
			if ($temp = strstr($content,'Expiration Date'))
			{
				$temp = trim($temp);
				$temp = substr($temp,17);
				#echo $content;
				$this -> endtime = $temp;

			}
			
			if ($temp = strstr($content,'Registry Expiry Date'))
			{
				$temp = trim($temp);
				$temp = substr($temp,22);
				#echo $content;
				$this -> endtime = $temp;

			}
			
			
			if ($temp = strstr($content,'Status'))
			{
				$temp = trim($temp);
				$temp = substr($temp,8);
				#echo $content;
				$this -> status[] = $temp;

			}
		}
		
		
	}
	
	
	public function whois_get_updatetime()
	{		
		return $this -> updatetime;
	}
	
	
	public function whois_get_nameserver()
	{
		
		return $this -> nameserver;
	}
	
	public function whois_get_context()
	{	
		return $this->context;
	}
		
	public function whois_get_begintime()
	{
		
		return $this ->begintime;
	}
	
	public function whois_get_endtime()
	{	
		return $this->endtime;
	}
	
	public function whois_get_status()
	{
		return $this->status;
	}
	
}

?>

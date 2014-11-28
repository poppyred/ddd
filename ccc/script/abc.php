<?php

ini_set('date.timezone','Asia/Shanghai');
class SearchDomain
{
        var $domain="";
        function SetDomain($udomain)
        {
                $this->domain = $udomain;
        }
        //
        // 获取whois并分析域名状态
        // ok 未被注册
        // 非空值 过期时间
        // 空值 未知
        //
        function GetInfo()
        {
                /*
                $dinfo = trim($this->GetWhois());
                if($dinfo=="") return "";
                if(eregi("no match",$dinfo)) return "ok";
        //return $rs;
        */
        $wl = "";
                $w_server = $this->GetServer();
                if($w_server=="") return "";
                $fp = fsockopen($w_server, 43, $errno, $errstr, 30);
                if(!$fp)
                {
                        echo $errstr;
                        return "";
                }
                  $out = $this->domain."\r\n";
              $out .= "Connection: Close\r\n\r\n";
              fputs($fp, $out);
              while (!feof($fp))
              {
                $wl = fgets($fp, 255);
                if(eregi("no match",$wl))
                {
                        fclose($fp);
                        return "ok";
                }
                if(eregi("Expiration Date",$wl))
                {
                        $lines = split(":",$wl);
                        $t = trim($lines[1]);
                        $ts = split(" ",$t);
                        $t = $ts[0];
                        if(ereg("[^0-9-]",$t))
                        {
                                $ts = split("-",$t);
                                $t = $ts[2]."-".$this->MonthToNum($ts[1])."-".$ts[0];
                        }
                        fclose($fp);
                        return $t;
                }
              }
              fclose($fp);
              return "";
          }
        //
        //获得域名的整个whois信息
        //
        function GetWhois()
        {
                $wh = "";
                $w_server = $this->GetServer();
                if($w_server=="") return "";
                $fp = fsockopen($w_server, 43, $errno, $errstr, 30);
                if(!$fp)
                {
                        echo $errstr;
                        return "";
                }
                  $out = "=".$this->domain."\r\n";
              $out .= "Connection: Close\r\n\r\n";
              fputs($fp, $out);
              while (!feof($fp))
              {
                $wh .= nl2br(fgets($fp, 255));
              }
              fclose($fp);
              return $wh;
          }
          //
          //输出当前域名的状态信息
          //
          function PrintSta()
          {
                  $rs = $this->GetInfo();
                  if($rs=="ok") echo $this->domain." 未注册！<br/>\r\n";
                  else if($rs=="") echo "无法查询 ".$this->domain." 状态！<br/>\r\n";
                  else echo $this->domain." 已注册，到期时间：$rs<br/>\r\n";
          }
          //
          //获得 whois 查询服务器
          //
          function GetServer()
          {
                  $udomain=substr($this->domain,-3);
                  switch($udomain)
                  {
                    case "com":
                              $w_server="whois.internic.net";
                              break;
                    case "net":
                              $w_server="whois.internic.net";
                              break;
                        case "org":
                              $w_server="whois.pir.org";
                                break;
                    case "nfo":
                              $w_server="whois.afilias.info";
                              break;
                    case "biz":
                              $w_server="whois.biz";
                              break;
                    case ".cc":
                              $w_server="whois.nic.cc";
                              break;
                    case "edu":
                              $w_server="whois.educause.net";
                              break;
                    case "gov":
                              $w_server="whois.nic.gov";
                              break;
                    case ".cn":
                              $w_server="whois.cnnic.net.cn";
                              break;
                    default:
                                $w_server="";
                  }
                  return $w_server;
        }
        //
        //英语的月份转为数字
        //
        function MonthToNum($m)
        {
                $m = strtolower($m);
                for($i=1;$i<=12;$i++)
                {
                        $tt = mktime(0,0,0,$i+1,0,2005);
                        if($m==strtolower(strftime("%b",$tt)))
                        {
                                if($i>9) return $i-1;
                                else return "0".$i-1;
                        }
                }
        }
}


$sd = new SearchDomain();
$sd->SetDomain("163.com");


$rs = $sd->GetWhois();
print_r($rs);

if($rs=="ok") echo $sd->domain." 未注册！<br/>\r\n";
else if($rs=="") echo "无法查询 ".$sd->domain." 状态！<br/>\r\n";
else echo $sd->domain." 已注册，到期时间：$rs<br/>\r\n";

//获得域名的详细whois信息
//echo $sd->GetWhois();

?>

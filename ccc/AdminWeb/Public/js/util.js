
//检测ip
function CheckIp(addr){
    var reg = /^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])(\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])){3}$/;
    if(addr.match(reg)){
        return true;
    }else{
        return false;
    }
}
//检测是否域名
function IsURL(str_url){
        var strRegex = "^((https|http|ftp|rtsp|mms)?://)"
        + "?(([0-9a-z_!~*'().&=+$%-]+: )?[0-9a-z_!~*'().&=+$%-]+@)?" //ftp的user@
        + "(([0-9]{1,3}\.){3}[0-9]{1,3}" // IP形式的URL- 199.194.52.184
        + "|" // 允许IP和DOMAIN（域名）
        + "([0-9a-z_!~*'()-]+\.)*" // 域名- www.
        + "([0-9a-z][0-9a-z-]{0,61})?[0-9a-z]\." // 二级域名
        + "[a-z]{2,6})" // first level domain- .com or .museum
        + "(:[0-9]{1,4})?" // 端口- :80
        + "((/?)|" // a slash isn't required if there is no file name
        + "(/[0-9a-z_!~*'().;?:@&=+$,%#-]+)+/?)$";
        var re=new RegExp(strRegex);
		var gg = /[`?#~$%^&*()_+=|\{\};\[\];\"',<>]/g;
        if (re.test(str_url)&&(gg.test(str_url)==false)){
            return (true);
        }else{
            return (false);
        }
}
function checkPhone(phone){
	var reg = /^(13[0-9]|15[0|3|6|7|8|9]|18[8|9])\d{8}$/;
	if(!reg.test(phone)){
		return false;	
	}
	return true;
}
function checkName(name){
	var reg = /^[\u4E00-\u9FA5]+$/
	if(!reg.test(name)){
		return false;	
	}
	return true;
}
//检验域名
/*function checkDomain(domain){
	var patrn=/^[0-9a-zA-Z]+[0-9a-zA-Z\.-]*\.[a-zA-Z]{2,4}$/;
	if (!patrn.test(domain)) return false;
	return true;
}*/

function checkDomain(dm){
	var re = /^([\w-]+\.)+((com)|(net)|(org)|(gov\.cn)|(info)|(cc)|(com\.cn)|(net\.cn)|(org\.cn)|(name)|(biz)|(tv)|(cn)|(mobi)|(name)|(sh)|(ac)|(io)|(tw)|(com\.tw)|(hk)|(com\.hk)|(ws)|(travel)|(us)|(tm)|(la)|(me\.uk)|(org\.uk)|(ltd\.uk)|(plc\.uk)|(in)|(eu)|(it)|(jp))$/;
	if(!re.test(dm)){
		return false;
	}
	return true;
}

function checkIpv6(ipv6){
	var matchStr = "((([0-9a-f]{1,4}:){7}([0-9a-f]{1,4}|:))|(([0-9a-f]{1,4}:){6}(:[0-9a-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9a-f]{1,4}:){5}(((:[0-9a-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9a-f]{1,4}:){4}(((:[0-9a-f]{1,4}){1,3})|((:[0-9a-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9a-f]{1,4}:){3}(((:[0-9a-f]{1,4}){1,4})|((:[0-9a-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9a-f]{1,4}:){2}(((:[0-9a-f]{1,4}){1,5})|((:[0-9a-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9a-f]{1,4}:){1}(((:[0-9a-f]{1,4}){1,6})|((:[0-9a-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9a-f]{1,4}){1,7})|((:[0-9a-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$";
	if (!ipv6.match(matchStr)) {
		return false;
	}
	return true;
}

function checkMask(mask){ 
    obj=mask; 
    var exp=/^(254|252|248|240|224|192|128|0)\.0\.0\.0|255\.(254|252|248|240|224|192|128|0)\.0\.0|255\.255\.(254|252|248|240|224|192|128|0)\.0|255\.255\.255\.(254|252|248|240|224|192|128|0)$/; 
    var reg = obj.match(exp); 
    if(reg==null) 
    { 
         return false; //"非法"
    } 
     else 
    { 
         return true; //"合法"
    } 
}
function checkemail(val){
	var reg =/^\w+((-\w+)|(\.\w+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z0-9]+$/;
		if(!reg.test(val)){
			return false;
		}
	return true;
}


var stopPropagation = function(e) {
	if(e && e.stopPropagation){         //W3C取消冒泡事件         
		e.stopPropagation();     
	}else{         //IE取消冒泡事件         
		window.event.cancelBubble = true;     
	} 
};
function GetDay(){   
	var today=new Date();      
	var yesterday_milliseconds=today.getTime()-1000*60*60*24;    

	var yesterday = new Date();      
	yesterday.setTime(yesterday_milliseconds);      
		
	var strYear=yesterday.getFullYear(); 

	var strDay=yesterday.getDate();   
	var strMonth=yesterday.getMonth()+1; 

	if(strMonth<10){   
		strMonth="0"+strMonth;   
	}
	if(strDay<10){   
		strDay="0"+strDay;   
	}
	return strYesterday=strYear+"-"+strMonth+"-"+strDay;
}
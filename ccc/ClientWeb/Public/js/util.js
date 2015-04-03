//提示消息内容
var info_val = '<div style="position:relative;"><div class="patch-left"></div><div class="patch-right"></div><div class="expand-box J_expandBox" style="width:938px;"><b class="expand-box-arr" style="left: 107px;"><b class="expand-box-arr-in">◆</b></b><div class="help-tips-box"><div class="J_helpRecordType"style="display: none;"><p class="usual">要将域名指向空间商提供的IP地址，请选择「A记录」，要将域名指向另一个域名，请选择「CNAME记录」。</p><dl class="all-list"><dt>A记录：</dt><dd>将域名指向一个IP地址<span class="ui-color-grey">（例如：10.10.10.10）</span>，需要增加A记录</dd><dt>CNAME记录：</dt><dd>如果将域名指向一个域名，实现与被指向域名相同的访问效果，需要增加CNAME记录</dd><dt>MX记录：</dt><dd>建立电子邮箱服务，将指向邮件服务器地址，需要设置MX记录</dd><dt>NS记录：</dt><dd>域名解析服务器记录，如果要将子域名指定某个域名服务器来解析，需要设置NS记录</dd><dt>TXT记录：</dt><dd>可任意填写<span class="ui-color-grey">（可为空）</span>，通常用做SPF记录<span class="ui-color-grey">（反垃圾邮件）</span>使用</dd><dt>AAAA记录：</dt><dd>将主机名<span class="ui-color-grey">（或域名）</span>指向一个IPv6地址<span class="ui-color-grey">（例如：ff03:0:0:0:0:0:0:c1）</span>，需要添加AAAA记录</dd><dt><!--显性-->URL转发：</dt><dd>将域名指向一个http（s)协议地址，访问域名时，自动跳转至目标地址<span class="ui-color-grey">（例如：将www.net.cn显性转发到www.hichina.com后，访问www.net.cn时，地址栏显示的地址为：www.hichina.com）</span>。</dd><!--dt>隐性URL：</dt><dd>与显性URL类似，但隐性转发会隐藏真实的目标地址<span class="ui-color-grey">（例如：将www.net.cn隐性转发到www.hichina.com后，访问www.net.cn时，地址栏显示的地址仍然为：www.net.cn）</span>。</dd></dl--></div><div class="J_helpRR_A_MX_AAAA"style="display: none;"><p class="usual">要将域名解析为www.example.com，在主机记录处填写www即可。</p><p>主机记录就是域名前缀，常见用法有：</p><dl class="all-list"><dt>www：</dt><dd>将域名example.com解析为www.example.com；</dd><dt>@：</dt><dd>直接解析域名，使主机记录为空，解析后的域名为example.com<span class="ui-color-grey">（若不填写主机记录自动填充@）</span>；</dd><dt>mail：</dt><dd>通常被用作邮箱服务器前缀，解析后的域名为mail.example.com；</dd><dt>*：</dt><dd>泛解析，所有子域名均被解析到同一地址（除单独设置的子域名解析）。</dd></dl></div><div class="J_helpRR_CNAME"style="display: none;"><p class="usual">要将域名解析为www.example.com，在主机记录处填写www即可。</p><p>主机记录就是域名前缀，常见用法有：</p><dl class="all-list"><dt>www：</dt><dd>将域名example.com解析为www.example.com；</dd><dt>mail：</dt><dd>通常被用作邮箱服务器前缀，解析后的域名为mail.example.com；</dd><dt>*：</dt><dd>泛解析，所有子域名均被解析到同一地址<span class="ui-color-grey">（除单独设置的子域名解析）</span>。</dd></dl><p>【注意】CNAME记录的主机记录不能为空。</p></div><div class="J_helpRR_NS"style="display: none;"><p class="usual">要将域名解析为www.example.com，在主机记录处填写www即可。</p><p>主机记录就是域名前缀，常见用法有：</p><dl class="all-list"><dt>www：</dt><dd>将域名example.com解析为www.example.com；</dd><dt>mail：</dt><dd>通常被用作邮箱服务器前缀，解析后的域名为mail.example.com。</dd></dl><p>【注意】NS记录的主机记录不能为空，且NS记录不支持泛解析<span class="ui-color-grey">（泛解析：将所有子域名解析到同一地址）</span>。</p></div><div class="J_helpRR_TXT"style="display: none;"><p class="usual">要将域名解析为www.example.com，在主机记录处填写www即可。</p><p>主机记录就是域名前缀，常见用法有：</p><dl class="all-list"><dt>www：</dt><dd>将域名example.com解析为www.example.com；</dd><dt>mail：</dt><dd>通常被用作邮箱服务器前缀，解析后的域名为mail.example.com；</dd></dl><p>【注意】TXT记录不支持泛解析<span class="ui-color-grey">（泛解析：将所有子域名解析到同一地址）</span>。</p></div><div class="J_helpRR_SRV"style="display: none;"><p class="usual">要将域名解析为www.example.com，在主机记录处填写www即可。</p><p>主机记录就是域名前缀，常见用法有：</p><dl class="all-list"><dt>www：</dt><dd>将域名example.com解析为www.example.com；</dd><dt>mail：</dt><dd>通常被用作邮箱服务器前缀，解析后的域名为mail.example.com；</dd></dl><p>【注意】SRV记录不支持泛解析<span class="ui-color-grey">（泛解析：将所有子域名解析到同一地址）</span>。</p></div><div class="J_helpRR_URL1"style="display: none;"><p class="usual">要将域名解析为www.example.com，在主机记录处填写www即可。</p><p>主机记录就是域名前缀，常见用法有：</p><dl class="all-list"><dt>www：</dt><dd>将域名example.com解析为www.example.com；</dd><dt>mail：</dt><dd>通常被用作邮箱服务器前缀，解析后的域名为mail.example.com；</dd></dl><p>【注意】显性URL不支持泛解析<span class="ui-color-grey">（泛解析：将所有子域名解析到同一地址）</span>。</p></div><div class="J_helpRR_URL0"style="display: none;"><p class="usual">要将域名解析为www.example.com，在主机记录处填写www即可。</p><p>主机记录就是域名前缀，常见用法有：</p><dl class="all-list"><dt>www：</dt><dd>将域名example.com解析为www.example.com；</dd><dt>mail：</dt><dd>通常被用作邮箱服务器前缀，解析后的域名为mail.example.com；</dd></dl><p>【注意】隐性URL不支持泛解析<span class="ui-color-grey">（泛解析：将所有子域名解析到同一地址）</span>。</p></div><div class="J_helpRoute"style="display: none;"><p class="usual">若您对解析线路、智能解析不了解或不想设置，建议您选择「默认」线路。<a href="javascript:void(0);"target="_blank">什么是智能解析？</a></p><p>选择与您的空间或服务器所在线路相同的解析线路<span class="ui-color-grey">（例如：联通）</span>可以提升该线路<span class="ui-color-grey">（联通）</span>用户访问目标地址的速度，但必须同时设置一个默认线路的解析，否则会导致部分用户无法访问该目标地址。</p><dl class="all-list"><dt>* 默认：</dt><dd>若您未设置特定解析线路，则所有线路用户均访问该目标地址。<br>若设置了特定解析线路<span class="ui-color-grey">（例如：联通）</span>，则特定线路用户访问特定目标地址，其他线路用户仍然访问该<span class="ui-color-grey">（默认）</span>目标地址；</dd><dt>联通：</dt><dd>单独为用【联通线路】访问的用户指定目标地址，其他用户仍然访问默认的目标地址；</dd><dt>电信：</dt><dd>单独为用【电信线路】访问的用户指定目标地址，其他用户仍然访问默认的目标地址；</dd><dt>移动：</dt><dd>单独为用【移动线路】访问的用户指定目标地址，其他用户仍然访问默认的目标地址；</dd><dt>教育网：</dt><dd>单独为用【教育网线路】访问的用户指定目标地址，其他用户仍然访问默认的目标地址；</dd><dt>海外：</dt><dd>单独为用【海外线路】访问的用户指定目标地址，其他用户仍然访问默认的目标地址。</dd></dl></div><div class="J_helpRoute_URL"style="display: none;"><p>URL转发无需选择解析线路，使用默认即可。</p></div><div class="J_helpValue_A"style="display: none;"><p><b>A记录值：</b></p><p>请填写您的服务器IP地址<span class="ui-color-grey">（可以为IPv4地址或者IPv6地址，例如：11.11.11.11）</span>，若不清楚IP，请您咨询您的网络资源提供商。</p></div><div class="J_helpValue_CNAME"style="display: none;"><p><b>CNAME记录值：</b></p><p>请填写空间服务商提供给您的域名<span class="ui-color-grey">（例如：hichina.com）</span>。</p></div><div class="J_helpValue_MX"style="display: none;"><p><b>MX记录值：</b></p><p>请填写邮件服务商提供给您的域名<span class="ui-color-grey">（例如：hichina.com）</span>。</p></div><div class="J_helpValue_NS"style="display: none;"><p><b>NS记录值：</b></p><p>NS向下授权，请填写DNS域名<span class="ui-color-grey">（例如：example.hichina.com）</span>。</p></div><div class="J_helpValue_TXT"style="display: none;"><p><b>TXT记录值：</b></p><p>可以随意填写文本<span class="ui-color-grey">（长度不能超过253字符）</span>，不可为空，一般用作企业邮箱的反垃圾邮件设置。</p></div><div class="J_helpValue_AAAA"style="display: none;"><p><b>AAAA记录值：</b></p><p>请填写一个IPv6地址<span class="ui-color-grey">（例如：ff03:0:0:0:0:0:0:c1）</span>。</p></div><div class="J_helpValue_SRV"style="display: none;"><p><b>SRV记录值：</b></p><p>格式为：优先级、空格、权重、空格、端口号、空格、目标地址，例如：3 0 2176 example-server.2.google.com。</p></div><div class="J_helpValue_URL1"style="display: none;"><p><b>显性URL记录值：</b></p><p>请填写要跳转到的网址：</p><dl class="all-list"><dt>示例1：</dt><dd>www.net.cn<dt>示例2：</dt><dd>www.net.com</dl><p><span class="ui-color-red">【注意】根据工信部关于域名跳转服务的政策要求，URL转发功能目前只支持网站有备案号且接入商是万网的域名转发需求，网站无备案号或接入商不是万网的域名转发需求暂不支持。</span></p></div><div class="J_helpValue_URL0"style="display: none;"><p><b><!--隐性-->URL记录值：</b></p><p>请填写要跳转到的网址：</p><dl class="all-list"><dt>示例1：</dt><dd>www.net.cn<dt>示例2：</dt><dd>www.net.com</dl><p><span class="ui-color-red">【注意】根据工信部关于域名跳转服务的政策要求，URL转发功能目前只支持网站有备案号且接入商是万网的域名转发需求，网站无备案号或接入商不是万网的域名转发需求暂不支持。</span></p></div><div class="J_helpMX"style="display: none;"><p>用来指定邮件服务器接收邮件优先顺序，1-10，数值越小优先级越高。</p></div><div class="J_helpTTL"style="display: none;"><p class="usual">最常用的TTL值为10分钟，我们已为您默认选择，不用修改。</p><p>请输入0到1440的数字。</p><p>TTL指各地DNS缓存您域名记录信息的时间。</p></div></div><a class="btn-never-display J_btnNeverDisplay"href="javascript:void(0);" onclick="notshowInfo(this)">不再提示</a></div></div>',
//解析备注tr内容
tr_desc='<tr style="display:none" class="edit_desc"><td colspan="9" class="expand-outer fix"><div style="position: relative;"><div class="patch-left"></div><div class="patch-right"></div><div class="expand-box"><b class="expand-box-arr">◆<b class="expand-box-arr-in">◆</b></b><div class="remark-box fix J_remarkBox"><label class="lbl">备注：</label><textarea class="ui-textarea J_remarkTextarea" rows="3" maxlength="200" onKeyUp="wordsLimit(this)"></textarea><div class="btm"><span class="info J_countInfo">最多还可写<b>200</b>个字</span><button class="ui-btn-yellow btn-add J_btnSubmitRemark" onClick="saveEditDesc(this)">保存</button><button class="ui-btn-grey btn-cancle J_btnCancleRemark" onClick="cancelEditDesc(this)">取消</button></div></div></div></div></td></tr>';

function bindDomainDesc(val){
	return '<td colspan="9" class="expand-outer fix"><div style="position:relative"><div class="patch-left"></div><div class="patch-right"></div><div class="expand-box"><b class="expand-box-arr">◆<b class="expand-box-arr-in">◆</b></b><div class="remark-box fix J_remarkBox"><label class="lbl">备注：</label><textarea class="ui-textarea J_remarkTextarea" rows="3" maxlength="200" onKeyUp="wordsLimit(this)">'+val+'</textarea><div class="btm"><span class="info J_countInfo">最多还可写<b>200</b>个字</span> <button class="ui-btn-blue btn-add J_btnSubmitRemark" onclick="saveEditDesc(this)">保存</button> <button class="ui-btn-grey btn-cancle J_btnCancleRemark" onclick="cancelEditDesc(this)">取消</button></div></div></div></div></td>';
}

function bindZoneDesc(val){
	return '<tr style="display:none"><td colspan="5" class="expand-outer fix"><div style="position:relative"><div class="patch-left"></div><div class="patch-right"></div><div class="expand-box"><b class="expand-box-arr">◆<b class="expand-box-arr-in">◆</b></b><div class="remark-box fix J_remarkBox"><label class="lbl">备注：</label><textarea class="ui-textarea J_remarkTextarea" rows="3" maxlength="200" onKeyUp="wordsLimit(this)">'+val+'</textarea><div class="btm"><span class="info J_countInfo">最多还可写<b>200</b>个字</span> <button class="ui-btn-blue btn-add J_btnSubmitRemark" onClick="saveDomain(this)">保存</button> <button class="ui-btn-grey btn-cancle J_btnCancleRemark" onClick="cancelDomain(this)">取消</button></div></div></div></div></td></tr>';
}
String.prototype.trim = function () {
	return this .replace(/^\s\s*/, '' ).replace(/\s\s*$/, '' );
}
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

//检验域名
/*function checkDomain(domain){
	var patrn=/^[0-9a-zA-Z]+[0-9a-zA-Z\.-]*\.[a-zA-Z]{2,4}$/;
	if (!patrn.test(domain)) return false;
	return true;
}*/

function checkDomain(dm){
	var re = /^([\w-]+\.)+((com)|(net)|(org)|(gov\.cn)|(info)|(cc)|(com\.cn)|(net\.cn)|(org\.cn)|(name)|(biz)|(tv)|(cn)|(mobi)|(name)|(sh)|(ac)|(io)|(tw)|(com\.tw)|(hk)|(com\.hk)|(ws)|(travel)|(us)|(tm)|(la)|(me\.uk)|(org\.uk)|(ltd\.uk)|(plc\.uk)|(in)|(eu)|(it)|(jp)|(coop)|(edu)|(mil)|(int)|(ae)|(at)|(au)|(be)|(bg)|(br)|(bz)|(ca)|(ch)|(cl)|(cz)|(de)|(fr)|(hu)|(ie)|(il)|(ir)|(mc)|(to)|(ru)|(aero)|(nl))$/;
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
function checkemail(val){
	var reg =/^\w+((-\w+)|(\.\w+))*\@[A-Za-z0-9]+((\.|-)[A-Za-z0-9]+)*\.[A-Za-z0-9]+$/;
		if(!reg.test(val)){
			return false;
		}
	return true;
}
function messageBox(id,str){
	$("#"+id).children().html(str);
	$("#"+id).slideDown();				
	setTimeout(function(){
		$("#"+id).slideUp();
	},3000);
}

//IP转成整型
function _ip2int(ip) 
{
    var num = 0;
    ip = ip.split(".");
    num = Number(ip[0]) * 256 * 256 * 256 + Number(ip[1]) * 256 * 256 + Number(ip[2]) * 256 + Number(ip[3]);
    num = num >>> 0;
    return num;
}
//整型解析为IP地址
function _int2iP(num) 
{
    var str;
    var tt = new Array();
    tt[0] = (num >>> 24) >>> 0;
    tt[1] = ((num << 8) >>> 24) >>> 0;
    tt[2] = (num << 16) >>> 24;
    tt[3] = (num << 24) >>> 24;
    str = String(tt[0]) + "." + String(tt[1]) + "." + String(tt[2]) + "." + String(tt[3]);
    return str;
}
function showInfo(str){
	$("#regist-error").css('display','block');
	$("#error_msg").html(str);
}
/***********************注册验证****************************/

function checkEmail(){
	var val = $("#fm-regist-id-email").val();
	if(typeof(val)=="undefined" || val==""){
		return false;
	}
	if(!checkemail(val)){
		showInfo('请输入正确的邮箱格式，例如：zhangsan@163.com');
		return false;
	}
	$.ajax({
		url:APP+'/Index/checkUser',
		type:'post',
		async:false,
		data:{'eamil':val},
		success:function(data){
			if(data.info=='error'){
				showInfo('['+val+']  账户已存在，<a href="login.html">登陆</a> | <a href="forget.html">忘记密码？</a>');
				return false;
			}
		},
		error:function(data){
			showInfo(data.statusText);
		}
	});
	return true;
}
function checkYsPwd(){
	var val = $("#fm-regist-password-ys").val();
	//var tem=/^[^_][A-Za-z]*[a-z0-9_]*$/;
	if(typeof(val)=="undefined" || val==""){
		return false;
	}
	if(val.length<6){
		showInfo('密码不能小于六位字符');
		return false;
	}
	return true;
}
function checkPwd(){
	var val = $("#fm-regist-password").val();
	//var tem=/^[^_][A-Za-z]*[a-z0-9_]*$/;
	if(typeof(val)=="undefined" || val==""){
		return false;
	}
	/*if(!tem.test(val)){
		showInfo('密码只能为数字、字母组合组成');
		return false;
	}*/
	if(val.length<6){
		showInfo('密码不能小于六位字符');
		return false;
	}
	return true;
}
function checkRePwd(){
	var val = $("#fm-regist-repassword").val(),tem=$("#fm-regist-password").val();
	if(typeof(val)=="undefined" || val==""){
		return false;
	}
	if(val!=tem || val.length!=tem.length){
		showInfo('两次密码输入不一致');
		return false;
	}
	return true;
}
function checkPwdBar(val){
	if(val.length<=6){
		$("#bar_ruo").css('background','red');
		$("#bar_zhong").css('background','#99BEFF');
		$("#bar_qiang").css('background','#99BEFF');
	}
	if(val.length>6 && val.length<12){
		$("#bar_ruo").css('background','red');
		$("#bar_zhong").css('background','red');
		$("#bar_qiang").css('background','#99BEFF');
	}
	if(val.length>=12){
		$("#bar_ruo").css('background','red');
		$("#bar_zhong").css('background','red');
		$("#bar_qiang").css('background','red');
	}
}
function checkCode(){
	var val = $("#J-checkcode").val();
	if(typeof(val)=="undefined" || val==""){
		return false;
	}
	return true;
}

String.prototype.getQuery = function (name) {
	var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)", "i");
	var r = window.location.search.substr(1).match(reg);
	if (r != null) return unescape(r[2]); return null;
}

<?php if (!defined('THINK_PATH')) exit();?><!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta content="" name="keywords">
	<meta content="" name="description">
    <link rel="shortcut icon" href="__ROOT__/Public/img/eflydns.ico" />
	<link rel="stylesheet" href="__ROOT__/Public/css/core.css">
	<link rel="stylesheet" href="__ROOT__/Public/css/reg.css">
    <script type="text/javascript">var APP = "__APP__";</script>
	<script type="text/javascript" src="__ROOT__/Public/js/jquery-1.8.3.min.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/common.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/util.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/jquery.mailAutoComplete-3.1.js"></script>
	<title>EflyDNS - 修改密码</title>
	<script type="text/javascript">
		//如果是内部页，父级页面刷新,登录
		
		$(function(){
			var msg = $("#error-msg").html();
			if(msg){
				  $("#regist-error").show();
			}
		});
		function keyEmail(){
			$("#username").mailAutoComplete({
				boxClass: "out_box", //外部box样式     
				listClass: "list_box", //默认的列表样式     
				focusClass: "focus_box", //列表选样式中     
				markCalss: "mark_box", //高亮样式     
				autoClass: true,     
				textHint: true, //提示文字自动隐藏 
			}); 	
		}
	</script>
	<script type="text/javascript">
	function forgetPWD(){
		if(checkEmail2() && checkCode()){
			var mail = $("#fm-regist-id-email").val(),
				code = $("#J-checkcode").val();
				$.ajax({
					url:'__APP__/Index/forget',
					type:'post',
					data:{'mail':mail,'code':code},
					success:function(data){
						if(data.info=='error'){
							showInfo(data.data);	
						}else{
							//showInfo(data.data);
							//setTimeout(function(){
								window.location.href='__APP__/Index/updatepwd?mail='+mail;
							//},2000);
						}
					},
					error:function(data){
						showInfo(data.statusText);		
					}
				});
		}else{
			showInfo('请完整填写表单！');
		}
	}
	function checkEmail2(){
		var val = $("#fm-regist-id-email").val();
		if(typeof(val)=="undefined" || val==""){
			return false;
		}
		if(!checkemail(val)){
			showInfo('请输入正确的邮箱格式，例如：zhangsan@163.com');
			return false;
		}
		
		return true;
	}
	var timerc=60;
	function countdown(){
        if(timerc > 0){ 
            --timerc;
			$("#time_a").hide();
            $("#timedesc").css('color','red').html(timerc+"秒后可重发"); 
            t = setTimeout("countdown()", 1000);
        };
		if(timerc==0){
			clearTimeout(t);
			$("#time_a").show();
            $("#timedesc").html(""); 
		}
    };
	function changeVerify(){		
		var mail = $("#fm-regist-id-email").val();
		if(!mail){
			$("#username").focus();
			$("#msg-username").show();
			return false;
		}
		if(!isEmail(mail)){
			$("#username").focus();
			$("#msg-username").html("<span class='noempty'>*</span>输入正确的邮箱地址");
			$("#msg-username").show();
			return false;
		}
		timerc=60;
		countdown();
		$.ajax({
			type:'post',
			url:'__APP__/Index/sendCode',
			data:{'mail':$("#fm-regist-id-email").val()},
			success:function(data){
				showInfo(""+data.data);
			},
			error:function(data){
				showInfo(data.statusText);
			}
		});
	}
	</script>
</head>
<body class="regist">
        <div id="header-container">
		<div id="header" class="clr">
			<h1 id="logo">
				<a href="http://www.eflydns.com" title="DNSPro" ><img class="main lf" src="__ROOT__/Public/img/DNSPro_w.png" alt="DNSPro"></a>
				<span class="header-title" style="display:block;">修改密码</span>
			</h1>
		</div>
	</div>
	
	<div id="content-container">
    <div id="content" class="grid780 clr">
	
        <div id="regist-module" class="clr" >
            <div id="regist-wrap" style="border:none;">
                 <form id="regist" name="regist" method="post">
                    <div id="regist-error" class="form-error notice notice-error" style="display:none">
                        <span class="icon-notice icon-error"></span>
                        <span id="error_msg" class="notice-descript"><?php echo ($error); ?></span>
                    </div>
                    <table id="regist-form" class="form">
                        <tbody><tr>
                            <td class="fm-label">
                                <div class="fm-label-wrap fm-relative">
                                    <span class="icon icon-regist-email"></span>
                                    <label for="username">
                                        电子邮箱:</label>
                                </div>
                            </td>
                            <td class="fm-field" width="300">
                                <div class="fm-field-wrap" style="z-index:2;">
                                    <input type="text" id="fm-regist-id-email" name="username" onclick="onBlue();" onFocus="keyEmail()" tabindex="1" placeholder="输入电子邮箱" class="fm-text fm-validator" value="<?php echo ($mail); ?>">
                                    <div id="msg-username" class="fm-validator-result"><span class="noempty">*</span>输入邮箱地址</div>
                                </div>
                            </td>
                        </tr>
                        <tr>
                            <td class="fm-label">
                                <label for="fm-regist-aliyun-checkcode">
                                    验证码:</label>
                            </td>
                            <td class="fm-field">
                                <div class="fm-field-wrap">
                                    <input id="J-checkcode" name="checkcode" onclick="onBlue();" class="fm-text fm-checkcode fm-validator" tabindex="2" type="text" maxlength="6" autocomplete="off" value="" name="_fm-a-_0-im" style="width:80px!important;">
                                    <a href="javascript:void(0);" id="time_a" style="display:block;height:30px;line-height:30px;width:180px; margin-left:30px; text-align:center;" onclick="changeVerify(this);" class="J-changeCheckcode">获取验证码</a><span id="timedesc" style="height:30px;line-height:30px;width:180px;margin-left:20px;"></span>
                                    <div id="msg-code" class="fm-validator-result"><span class="noempty">*</span>输入验证码</div>
                                </div>
                        </td></tr>
                        <tr>
                            <td></td>
                            <td class="fm-field">
                                <div class="fm-field-wrap">
                                    <input type="button" id="fm-regist-submit" onClick="forgetPWD()" value="下一步" name="event_submit_do_join" tabindex="4" class="fm-button fm-submit">
                                </div>
                            </td>
                        </tr>
                    </tbody></table>
                </form>
            </div>
            <!--div id="regist-extra">
                    <h3>免费注册</h3>
                    <p>&nbsp;</p>
                    <a href="__APP__/Index/register">注册个人账号</a>
                    <p>&nbsp;</p>
                    <p>如果您是企业用户，请</p>
                    <a href="__APP__/Index/cregister">注册企业账号</a>
                    <p>&nbsp;</p>
                    <a href="#">了解更多>></a>
              </div-->
          </div>
    </div>
    </div>
    
    <!-- footer -->
	<style>
 #feedback {
   position:fixed;
   right:20px;
   /*top:92%;*/
   bottom: 14.5%;
   height:30px;
   width:100px;
   color:#fff;
   font-size:12px;
   line-height: 20px;
   background-color:#0066CB;
   border:1px solid #0099FF;
   text-align: center;
   z-index:16;
 }

 #feedback:hover {
   background-color: #0099FF;
 }

 #feedback a{
   line-height:30px;
   text-decoration:none;
   color:#fff;
 }

 .fix_bottom {
	position: absolute;
	bottom: 0;
	width: 100%;
 }
</style>

<div id="feedback" style="cursor:pointer;" onclick="location='tencent://message/?uin=1207197276';">
<a href="javascript:void(0);">客服咨询</a>
</div>
<div class="tc-page-foot">     
    <div class="foot_c1">
        <a href="http://www.eflypro.com/index.php/Aboutus/" target="_blank" class="link_gray">关于我们</a><!--span class="s">|</span>	      
        <a href="javascript:void(0);" target="_blank" class="link_gray">eflyDNS解析服务协议</a-->
        &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
        <span class="ui-color-grey">Copyright © 2014 广东睿江科技有限公司 版权所有</span>	  
    </div>
</div>
    
</body>
</html>
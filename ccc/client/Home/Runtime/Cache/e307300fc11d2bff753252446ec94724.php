<?php if (!defined('THINK_PATH')) exit();?><!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta content="" name="keywords">
	<meta content="" name="description">
	<title>EflyDNS - 账户管理</title>
    <link rel="shortcut icon" href="__ROOT__/Public/img/eflydns.ico" />
	<link rel="stylesheet" href="__ROOT__/Public/css/core.css">
	<link rel="stylesheet" href="__ROOT__/Public/css/base.css">
	<link rel="stylesheet" href="__ROOT__/Public/css/dns.css">
	<link rel="stylesheet" href="__ROOT__/Public/css/artDialog.css">
	<script type="text/javascript" src="__ROOT__/Public/js/jquery-1.8.3.min.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/jquery.mailAutoComplete-3.1.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/util.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/layer/layer.min.js"></script>    
	<style type="text/css">     
		.out_box{border:1px solid #ccc; background:#fff; font-size:16px;}     
		.list_box{border-bottom:1px solid #eee; padding:0 5px; cursor:pointer;}     
		.focus_box{background:#f0f3f9;}     
		.mark_box{color:#c00;}     
	</style> 
	<script type="text/javascript">
		function updatepwd(){
			if(checkPwd() && checkYsPwd() && checkRePwd()){
				var mail = $("#fm-regist-id-email").val(),
					oldpwd = $("#fm-regist-password-ys").val(),
					newpwd = $("#fm-regist-password").val();
				$.ajax({
					url:'__APP__/Index/userUpdatepwd',
					type:'post',
					data:{'mail':mail,'oldpwd':oldpwd,'newpwd':newpwd},
					success:function(data){
						if(data.info=='error'){
							showInfo(data.data);	
						}else{
							showInfo(data.data);
							setTimeout(function(){
								window.location.href='__APP__/Domain/domainList';
							},2000);
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
	</script>
</head>
<body class="page-dns">
	<div class="J_pageWrapper">
		<!-- header -->
		<div class="tc-page-head"> 	
	<div class="inner"> 		
		<h1 class="logo">
			<a href="http://www.eflydns.com/"><img class="main lf" src="__ROOT__/Public/img/DNSPro_w.png" alt="DNSPro"></a>
		</h1>
		<div class="info">
			<p class="user">您好，<?php echo ($_SESSION['user']); ?>
				<span class="s">|</span>
				<a href="javascript:void(0);" style="margin-left:-10px;padding: 5px 15px 5px 15px; background:url(__ROOT__/Public/img/down_arrow.png) no-repeat; background-position:40px 8px;" id="userinfo">账户</a>
                <!--span class="s">|</span>
				<a href="__APP__/Index/login/">退出</a><!--span class="s">|</span>	
				<a href="javascript:void(0);">购物车[0件]</a-->
			</p>
			<p class="nav">
				<span class="b active"></span><a href="__APP__/Domain/domainList/">域名解析</a>
				<!--span class="b"></span><a href="__APP__/Monitor/">域名监控</a>
				<span class="b"></span><a href="__APP__/Shop/">域名商城</a-->
				<span class="b"></span><a href="http://www.eflydns.com/index/Help/help/" target="_blank">帮助中心</a>
			</p>
		</div>        
        <div class="userset" id="userset">
            <ul>
                <li style=" border-top:none;">
                	<a href="__APP__/Index/user/"><i style="background:url(__ROOT__/Public/img/set.png) no-repeat;"></i>账户设置</a>
                </li>
                <!--li>
                	<a href="javascript:void(0);"><i style="background:url(__ROOT__/Public/img/service.png) no-repeat;"></i>我的服务</a>
                </li>
                <li>
                	<a href="javascript:void(0);"><i style="background:url(__ROOT__/Public/img/user.png) no-repeat;"></i>切换账户</a>
                </li-->
                <li>
                	<a href="__APP__/Index/login/"><i style="background:url(__ROOT__/Public/img/out.png) no-repeat;"></i>退出</a>
                </li>
            </ul>
        </div>
	</div>
</div> 
<script type="text/javascript">	
		
	$(function(e){
		
		$(".info .user a").hover(function(){
			$(this).css({'background':'#fff url(__ROOT__/Public/img/down_arrow1.jpg) no-repeat','color':'#666','background-position':'40px 8px'});
			$("#userset").show();
			
		},function(){
			//$(this).css({'background':'#1c8fdf url(__ROOT__/Public/img/down_arrow.png) no-repeat','color':'#fff','background-position':'40px 8px'});
		});
		
		$('html').bind('click',function(e){
			if($("#userset").is(":visible")){
				$("#userset").slideToggle();
				$(".info .user a").css({'background':'#1c8fdf url(__ROOT__/Public/img/down_arrow.png) no-repeat','color':'#fff','background-position':'40px 8px'});	
			}
		});
		
		$("#userset ul").mouseleave(function(){
			$(".info .user a").css({'background':'#1c8fdf url(__ROOT__/Public/img/down_arrow.png) no-repeat','color':'#fff','background-position':'40px 8px'});	
			$("#userset").hide();
		});
	});
	var stopPropagation = function(e) {
		if(e && e.stopPropagation){         //W3C取消冒泡事件         
			e.stopPropagation();     
		}else{         //IE取消冒泡事件         
			window.event.cancelBubble = true;     
		} 
	};
</script>
 
		<div class="head-info">
			<div class="head-path">
				<a class="home" href="__APP__/Domain/domainList" ></a><span class="s"></span>
				<a href="__APP__/Index/user">账户管理</a><span class="s"></span>
				<strong class="current">用户信息</strong>
			</div>
		</div> 
		<div class="J_singleTop">
			<p class="domain-title">
				<span class="dtype icon-null"></span>
				<strong id="cur_zone" class="dname"></strong>
				<!--span class="tab nocur"><a href="__APP__/Index/alarm">告警通知</a></span-->
				<!--span class="tab nocur"><a href="__APP__/Index/safe">账号安全</a></span-->
				<span class="tab "><a href="__APP__/Index/user">用户信息</a></span>
			</p>
		</div>
		<div class="table-box" style="margin-top:7px;">
        	<input id="fm-regist-id-email" value="<?php echo ($mail); ?>" type="hidden" />
			<table cellpadding="0" cellspacing="0" width="100%" id="mytab" style="font-size:14px;">
				<thead>
					<tr id="tr_title">
						<th style="text-align:left;font-size:13px;color:#404040;height:30px;" colspan="2">
                        	<div id="regist-error" class="form-error notice notice-error" style="display:none;width:440px;margin-left:100px">
                                <span class="icon-notice icon-error"></span>
                                <span class="notice-descript" id="error_msg"></span>
                            </div>
                        </th>
						<th style="height:30px;">						
						</th>
					</tr>
				</thead> 	
				<tbody id="myData">
					<tr class="data" style="height:55px;">
					    <td class="laber" style="text-align:center;width:150px;">原始密码</td>
					    <td class="edit">
                        	<input type="password" id="fm-regist-password-ys" style="width:285px;" name="_fm-a-_0-p" tabindex="3" class="fm-text fm-password fm-validator" autocomplete="off" onkeyup="checkPwdBar(this.value)" onBlur="checkYsPwd()"/>
                        </td>
					    <td class="info" width="50px"></td>
					</tr>
                    <tr class="data" style="height:55px;">
					    <td class="laber" style="text-align:center;width:150px;">登录密码</td>
					    <td class="edit">
                        	<div class="fm-field-wrap">
                                    <input type="password" id="fm-regist-password" style="width:285px;" name="_fm-a-_0-p" tabindex="3" class="fm-text fm-password fm-validator" autocomplete="off" onkeyup="checkPwdBar(this.value)" onBlur="checkPwd()"/>
                                       <div id="fm-regist-password-pstrength" class="pstrength-container">
                                           <div id="fm-regist-password-pstrength-bar" class="pstrength-bar clr">
                                           <div class="pstrength-bar-weak" id="bar_ruo">弱</div>
                                           <div class="pstrength-bar-middle" id="bar_zhong">中</div>
                                           <div class="pstrength-bar-strong" id="bar_qiang">强</div>
                                       </div>
                                    </div><div class="fm-validator-result"></div></div>
                        </td>
					    <td class="info"></td>
					</tr>
                    <tr class="data" style="height:55px;">
					    <td class="laber" style="text-align:center;width:150px;">确认密码</td>
					    <td class="edit">
                        	<input type="password" id="fm-regist-repassword" style="width:285px;" name="_fm-a-_0-p" tabindex="3" class="fm-text fm-password fm-validator" autocomplete="off" onkeyup="checkPwdBar(this.value)" onBlur="checkRePwd()"/>
                        </td>
					    <td class="info"></td>
					</tr>
                    <tr style="height:55px;" align="right">                    	
                        <td colspan="2">
                        	<input type="button" id="fm-regist-submit" onClick="updatepwd()" value="保存" name="event_submit_do_join" tabindex="4" class="fm-button fm-submit">
                            <input type="button" id="fm-regist-submit" onClick="history.go(-1)" value="返回" name="event_submit_do_join" tabindex="4" class="fm-button fm-submit">
                        </td>
					    <td ></td>
					</tr>
				</tbody> 
			</table>
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
	</div>
</body>
</html>
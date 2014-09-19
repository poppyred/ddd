<?php if (!defined('THINK_PATH')) exit();?><!DOCTYPE html>
<html>
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta content="" name="keywords">
	<meta content="" name="description">
	<title>EflyDNS - 操作日志</title>
    <link rel="shortcut icon" href="__ROOT__/Public/img/eflydns.ico" />
	<link rel="stylesheet" href="__ROOT__/Public/css/base.css">
	<link rel="stylesheet" href="__ROOT__/Public/css/dns.css">
</head>
<body class="page-logs" style="">
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
				<a class="home" href="__APP__/Domain/domainList"></a><span class="s"></span>
				<a href="__APP__/Domain/domainList">域名列表</a><span class="s"></span>
				<strong class="current">域名操作日志</strong> 
			</div>
		</div> 
		<div class="table-box">
			<div class="toolbar fix"> 	
				<div class="lf">
					<input type="text" class="ui-ipt-txt ipt-date" readonly="" placeholder="开始时间(非必填)" id=""><span class="w">到</span>
					<input type="text" class="ui-ipt-txt ipt-date" readonly="" placeholder="结束时间(非必填)" id=""> 		
					<input class="ui-ipt-txt ui-ipt-search J_iptSearchLog" submit-with=".J_btnQuerySubmit" type="text" placeholder="关键字(如：域名、IP。非必填)">
					<button class="ui-btn-grey btn-s-l J_btnQuerySubmit">查询</button><button class="ui-btn-grey btn-s-l J_btnResetForm">重置</button> 	
				</div> 
			</div>
			<table class="ui-table" cellpadding="0" cellspacing="0" width="100%"> 	
				<thead> 		
					<tr><th class="log-date">操作时间</th> 		
					    <th>域名</th> 		
					    <th>操作行为</th> 		
					    <th>操作者IP</th> 	
					</tr>
				</thead> 	
				<tbody>   
					<tr class="even"> 	
						<td class="log-date">2014-04-04 09:04:02</td> 	
						<td class="log-domain">keyanwei.com.cn</td> 	
						<td class="log-action"><span>修改星标</span></td> 	
						<td class="log-ip">119.145.147.18</td> 
					</tr>   
					<tr> 	
						<td class="log-date">2014-04-04 09:03:59</td> 	
						<td class="log-domain">keyanwei.com.cn</td> 	
						<td class="log-action"><span>修改星标</span></td> 	
						<td class="log-ip">119.145.147.18</td> 
					</tr>   
					<tr class="even"s> 	
						<td class="log-date">2014-04-04 09:03:58</td> 	
						<td class="log-domain">keyanwei.com.cn</td> 	
						<td class="log-action"><span>修改星标</span></td> 	
						<td class="log-ip">119.145.147.18</td> 
					</tr>  
				</tbody> 	
				<tfoot> 		
					<tr> 			
						<td colspan="4" class="J_pageNumBox"><div class="ui-page-num"><a href="javascript:void(0);" class="on">1</a><button class="ui-btn-grey btn-prev btn-off J_pagePrev">&lt;上一页</button><button class="ui-btn-grey btn-next btn-off J_pageNext">下一页&gt;</button>跳转至<input type="text" class="ui-ipt-txt ipt-go-page J_iptPageNavTo"><span class="btn-submit-go"></span>页 </div></td> 		
					</tr> 	
				</tfoot> 
			</table>
		</div> 
		<!-- footer -->
		<style>
 #feedback {
   position:fixed;
   right:20px;
   /*top:92%;*/
   bottom: 44.5%;
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

<div id="feedback" style="cursor:pointer;" onclick="location='tencent://message/?uin=1207197276&Site=wendns.com&Menu=yes';">
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
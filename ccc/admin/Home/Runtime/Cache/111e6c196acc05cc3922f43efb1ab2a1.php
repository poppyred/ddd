<?php if (!defined('THINK_PATH')) exit();?>﻿<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Language" content="zh-CN">
<meta name="Keywords" content="DNSPro">
<meta name="Description" content="DNSPro DNS 域名解析">
<title>DNSPro管理后台</title>
<link type="text/css" rel="stylesheet" href="__ROOT__/Public/css/style.css">
<script type="text/javascript" src="__ROOT__/Public/js/jquery-1.8.1.min.js"></script>
<script type="text/javascript" src="__ROOT__/Public/js/jquery.SuperSlide.2.1.js"></script>
<!-- Le HTML5 shim, for IE6-8 support of HTML elements -->
<!--[if lt IE 9]>
    <script src="./js/html5.js"></script>
<![endif]-->
</head>
<body>

<div id="topBar">
	<div class="siteWidth">
		<span><a style="text-decoration:none;"><a href="#">收藏页面</a></span>
		<ul>
			<li><em>您的域名出现访问超时，请留意！</em>DNSpro监控到您的域名出现故障，请检查</li>
			<li><em>DNSPro退出个人用户免费套餐啦，欢迎免费试用！</em>试用对象：小型站点，企业网站，个人用户等</li>
		</ul>
	</div>
</div><div class="clear"></div>
<script type="text/javascript">jQuery("#topBar").slide({ mainCell:"ul",autoPlay:true,effect:"topLoop" });</script>


<!-- 头部 S -->
<div id="header">
	<div class="logoBar">
		<h1>DNSPro管理平台</h1>
	</div>

	<!-- navBar -->
	<div class="navBar">
		<ul class="nav clearfix">
			<li class="m">
				<h3><a target="_self" href="__APP__/Clientmgr/">客户管理</a></h3>
			</li>
			<li class="s">|</li>
			<li class="m">
				<h3><a target="_self" href="__APP__/Domainmgr/">域名管理</a></h3>
			</li>
			<li class="s">|</li>
			<li class="m">
				<h3><a target="_self" href="#">套餐管理</a></h3>
			</li>
			<li class="s">|</li>
			<li class="m">
				<h3><a target="_self" href="__APP__/Runmgr/">运行管理</a></h3>
			</li>
			<li class="s">|</li>
			<li class="m on">
				<h3><a target="_self" href="__APP__/System/">控制面板</a></h3>
			</li>
			<div>
				<span class='name'><?php echo ($_SESSION['user']); ?></span>
				<span class='notification'>|</span>
				<a class='account' href="__APP__/Index/"><span>退出</span></a>
			</div>
		</ul>
	</div>
</div><!-- 头部 e --><div class="clear"></div>

<div class="content"><!-- 内容 -->
	<!-- left菜单 S -->
	<div id="sideMenu" class="side">
		<div class="hd">
			<h3>服务器管理</h3>
		</div>
		<div class="bd">
			<ul>
				<li><a style="color:#3CADED; text-decoration:underline;" href="__APP__/System">服务器列表</a></li>
				<li><a href="__APP__/System/addServer">添加服务器</a></li>
			</ul>
		</div>
		<div class="hd">
			<h3>系统用户管理</h3>
		</div>
		<div class="bd">
			<ul>
				<li><a href="__APP__/System/user">系统用户列表</a></li>
				<li><a href="__APP__/System/addUser">添加系统用户</a></li>
			</ul>
		</div>
		<div class="hd">
			<h3>任务下发管理</h3>
		</div>
		<div class="bd">
			<ul>
				<li><a href="__APP__/System/task">任务下发记录</a></li>
				<li><a href="#"></a></li>
				<li><a href="#"></a></li>
				<li><a href="#"></a></li>
			</ul>
		</div>
		<div class="hd">
			<h3>其他功能</h3>
		</div>
		<div class="bd">
			<ul>
				<li><a href="#"></a></li>
				<li><a href="#"></a></li>
			</ul>
		</div>
	</div>
	<script type="text/javascript">jQuery("#sideMenu").slide({ titCell:".hd", targetCell:".bd", effect:"slideDown", trigger:"click" });</script>
	<!-- left菜单 E -->
	
	<!-- mainContent s -->
	<div class="mainContent">
		
		<!-- Tab切换 S -->
		<div class="slideTxtBox">
			<div class="hd">
				<h2 style="float:left;">服务器列表</h2>
			</div>
			<div class="bd">
				<ul>
					<div class="table-toolbar">
						<div class="lf">
							服务器类型：
							<select id="sel_type" class="ui-select" onchange="selectType();">
								<option value="all">全部</option>
								<?php if(is_array($tlist)): $i = 0; $__LIST__ = $tlist;if( count($__LIST__)==0 ) : echo "" ;else: foreach($__LIST__ as $key=>$t_vo): $mod = ($i % 2 );++$i; if($t_vo["type"] == $type): ?><option selected><?php echo ($t_vo["type"]); ?></option>
										<?php else: ?><option><?php echo ($t_vo["type"]); ?></option><?php endif; endforeach; endif; else: echo "" ;endif; ?>
							</select>
							<span style="margin-left:30px;">服务器子类：</span>
							<select id="sel_subtype" class="ui-select">
								<option value="all">全部</option>
								<?php if(is_array($ctlist)): $i = 0; $__LIST__ = $ctlist;if( count($__LIST__)==0 ) : echo "" ;else: foreach($__LIST__ as $key=>$ct_vo): $mod = ($i % 2 );++$i;?><option><?php echo ($ct_vo["subtype"]); ?></option><?php endforeach; endif; else: echo "" ;endif; ?>
							</select>
						</div>
						<div class="rf"> 
							<!--a class="icon icon-export" href="optLog.html">解析记录日志</a-->
							<!--a class="icon icon-log" href="javascript:void(0);" target="_blank">查看报警信息</a> 
							<input class="ui-ipt-txt ui-ipt-search " submit-with=".J_btnSubmitSearch" type="text" placeholder="请输入关键字词">
							<button class="ui-btn-grey">搜索</button-->
						</div> 
					</div>
					<table class="ui-table" cellpadding="0" cellspacing="0" width="100%" id="mytab">
						<thead>
						<tr id="tr_title">
							<th class="chk">编号</th>
							<th style="text-align:left;"><span class="el">服务器编码</span></th>
							<th><span class="el">IP地址</span></th>
							<th><span class="el">状态</span></th>
							<th><span class="el">备注信息</span></th>
							<th>操作&nbsp;</th>
						</tr>
						</thead> 	
						<tbody id="myData">
							<?php if(is_array($slist)): $i = 0; $__LIST__ = $slist;if( count($__LIST__)==0 ) : echo "" ;else: foreach($__LIST__ as $key=>$s_vo): $mod = ($i % 2 );++$i;?><tr class="data"> 	 	
								<td class="chk"><?php echo ($s_vo["id"]); ?></td>
								<td class="val" style="text-align:left;">
									<a title="" href="__APP__/System/editServer?id=<?php echo ($s_vo["id"]); ?>"><?php echo ($s_vo["sid"]); ?></a>
								</td>
								<td class="iop"><?php echo ($s_vo["ip"]); ?></td>
								<td class="iop"><?php echo ($s_vo["status"]); ?></td> 
								<td class="rr" style="text-align:left;width:240px;"><?php echo ($s_vo["desc"]); ?></td>
								<td class="iop">
									<?php if($s_vo["status"] == 'true'): ?><a title="停用服务器" href="javascript:void(0);" onclick="stopServer(<?php echo ($s_vo["id"]); ?>,this);">停用</a>
										<?php else: ?><a title="启用服务器" href="javascript:void(0);" onclick="startServer(<?php echo ($s_vo["id"]); ?>,this);">启用</a><?php endif; ?>
									<a title="删除" href="javascript:void(0);" onclick="deleteDns(<?php echo ($s_vo["id"]); ?>,this)">删除</a>
								</td>
							</tr><?php endforeach; endif; else: echo "" ;endif; ?>
						</tbody> 
					</table>
				</ul>
			</div>
		</div>
		<script type="text/javascript">jQuery(".slideTxtBox").slide();</script>
		<!-- Tab切换 E -->
	</div>
</div>

	<div class="footer">
		<span class="ui-color-grey">Copyright © 2014 DNSPro 版权所有</span>	  
	</div>

<script type="text/javascript">
	function selectType(){
		var t = $("#sel_type").val();
		if(t=="all"){
			window.location.href = "__APP__/System/index";
		}else{
			window.location.href = "__APP__/System/index?t="+t;
		}
	}
	function stopServer(id,evt){
		if(confirm('确认停用该服务器吗？')){

			$.ajax({

				url: '__APP__/System/stopServer',

				type: "post",

				data:{'id':id},

				success: function (data) {

					//$("#save_msg").html(data.info);

					$($(evt).parent().parent().find("td").get(3)).html('false');
					$($(evt).parent()).html('<a title="启用服务器" href="javascript:void(0);" onclick="startServer('+id+',this);">启用</a> <a title="删除" href="javascript:void(0);" onclick="deleteDns('+id+',this)">删除</a>');
				},

				error: function (data) {

					alert(data.statusText);

				}

			});

		}
	}
	function startServer(id,evt){
		$.ajax({

			url: '__APP__/System/startServer',

			type: "post",

			data:{'id':id},

			success: function (data) {

				$($(evt).parent().parent().find("td").get(3)).html('true');
				$($(evt).parent()).html('<a title="停用服务器" href="javascript:void(0);" onclick="stopServer('+id+',this);">停用</a> <a title="删除" href="javascript:void(0);" onclick="deleteDns('+id+',this)">删除</a>');
			},

			error: function (data) {

				alert(data.statusText);

			}

		});

	}
	//这种导航效果相当于Tab切换，用titCell和mainCell 处理
	jQuery(".navBar").slide({ 
		titCell:".nav .m", // 鼠标触发对象
		mainCell:".subNav", // 切换对象包裹层
		delayTime:0, // 效果时间
		triggerTime:0, //鼠标延迟触发时间
		returnDefault:true  //返回默认状态
	});
</script> 
</body>
</html>
<?php if (!defined('THINK_PATH')) exit();?><!DOCTYPE html>
<html><head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta content="" name="keywords">
	<meta content="" name="description">
	<title>EflyDNS - 解析统计</title>
    <link rel="shortcut icon" href="__ROOT__/Public/img/eflydns.ico" />
	<link rel="stylesheet" href="__ROOT__/Public/css/base.css">
	<link rel="stylesheet" href="__ROOT__/Public/css/dns.css">
	<script type="text/javascript" src="__ROOT__/Public/js/jquery-1.8.3.min.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/util.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/layer/layer.min.js"></script>
	<script type="text/javascript" src="__ROOT__/Public/js/highcharts/highcharts.js"></script>
	<script type="text/javascript" >
		$(function(){
			$('#chart').highcharts({
				title: {
				    text: '域名请求量统计图'//,
				    //x: -20 //center
				},
				subtitle: {
				    text: '今天请求总量：暂无数据',//1245次（统计结果延迟1小时）',
				    x: -16
				},
				xAxis: {
				    categories: ['14:50', '14:55', '15:00', '15:05', '15:10', '15:15','15:20', '15:25', '15:30', '15:35', '15:40', '15:45']
				},
				yAxis: {
				    title: {
					text: '请求次数'
				    },
				    min: 0,
				    plotLines: [{
					value: 0,
					width: 1,
					color: '#808080'
				    }]
				},
				tooltip: {
				    valueSuffix: '次',
				    shared: true
				},
				legend: {
				    enabled: true
				},
				series: [{
					  name: '访问量',
				    //data: [7.0, 6.9, 9.5, 14.5, 18.2, 21.5, 25.2, 26.5, 23.3, 18.3, 13.9, 9.6]
				}/*, {
				      //name: 'mail.test1.com',
				      //data: [0.2, 0.8, 5.7, 11.3, 17.0, 22.0, 24.8, 24.1, 20.1, 14.1, 8.6, 2.5]
				}, {
				      //name: 'img.test1.com',
				      //data: [0.9, 0.6, 3.5, 8.4, 13.5, 17.0, 18.6, 17.9, 14.3, 9.0, 3.9, 1.0]
				}*/]
			});
			
			$('#container').highcharts({
			    chart: {
				type: 'area'
			    },
			    title: {
				text: '子域名请求比重'
			    },
			    subtitle: {
				text: '（统计结果延迟一小时）暂无数据'
			    },
			    xAxis: {
				categories: ['14:50', '14:55', '15:00', '15:05', '15:10', '15:15','15:20', '15:25', '15:30', '15:35', '15:40', '15:45'],
				tickmarkPlacement: 'on',
				title: {
				    enabled: false
				}
			    },
			    yAxis: {
				title: {
				    text: '百分比'
				}
			    },
			    tooltip: {
				pointFormat: '<span style="color:{series.color}">{series.name}</span>: <b>{point.percentage:.1f}%</b> ({point.y:,.0f} 次)<br/>',
				shared: true
			    },
			    plotOptions: {
				area: {
				    stacking: 'percent',
				    lineColor: '#ffffff',
				    lineWidth: 1,
				    marker: {
					lineWidth: 1,
					lineColor: '#ffffff'
				    }
				}
			    },
			    series: [{
				      name: '访问量',
				      //data: [7.0, 6.9, 9.5, 14.5, 18.2, 21.5, 25.2, 26.5, 23.3, 18.3, 13.9, 9.6]
				}/*, {
				      //name: 'mail.test1.com',
				      //data: [2.1, 0.8, 5.7, 11.3, 17.0, 22.0, 24.8, 24.1, 20.1, 14.1, 8.6, 2.5]
				}, {
				      //name: 'img.test1.com',
				      //data: [0.9, 0.6, 3.5, 8.4, 13.5, 17.0, 18.6, 17.9, 14.3, 9.0, 3.9, 1.0]
				}*/]
			});
		});

		//字数限制
		function wordsLimit(obj){
			$(obj).next().find('b').html(200-$(obj).val().length);
		}
		function bindRoute(route){
			var val;
			if(typeof(route)=='undefined' || route==""){
				return "默认";
			}else{
				return val;
			}
		}
		//全选、取消全选
		function checkAll(obj){
			$('#myData :checkbox').each(function(){
				this.checked = obj.checked;
			});
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

		<input type="hidden" value="<?php echo ($zone); ?>" id="txt_zone"/>
		<input type="hidden" value="<?php echo ($is_new); ?>" id="is_first"/>
		<div class="head-info">
			<div class="head-path">
				<a class="home" href="__APP__/Domain/domainList" ></a><span class="s"></span>
				<a href="__APP__/Domain/domainList">域名列表</a><span class="s"></span>
				<strong class="current"><?php echo ($zone); ?> 的解析统计</strong>
			</div>
		</div> 
		<div class="J_singleTop">
			<p class="domain-title">
				<!--if condition="$is_on eq 0">
                	<span class="dtype icon-pause"></span>
					<else /-->
                    <?php if($level != 0): ?><span class="dtype icon-vip"></span>
                        <?php else: ?><span class="dtype icon-novip"></span><?php endif; ?>
                <!--<a href="__APP__/Domain/domainRoute?d=<?php echo ($zone); ?>">自定义路线</a>-->
				<strong id="cur_zone" class="dname"><?php echo ($zone); ?></strong>
				<span class="tab nocur"><a href="javascript:void(0);">自定义路线</a></span>
				<span class="tab "><a href="__APP__/Domain/domainRpt?d=<?php echo ($zone); ?>">解析统计</a></span>
				<span class="tab nocur"><a href="__APP__/Domain/domainSet?d=<?php echo ($zone); ?>">域名设置</a></span>
				<span class="tab nocur"><a href="__APP__/Domain/detail?d=<?php echo ($zone); ?>">记录管理</a></span>
			</p>
		</div> 
		
		<div class="J_infoBox" style="display:none;">
		    <div class="dns-tip-box fix J_dnsTipBox">
			<div class="lf tips">
			    <p>
				<b class="ui-color-red">
				    域名解析记录生效的时间分为以下三种情况：
				    <a class="ui-help" href="javascript:void(0);" title="先设置解析记录，后修改DNS，可使您的域名解析平滑过渡，访问无断点。"></a>
				</b>
			    <p class="dot">
			      新增解析记录，我们是实时生效的。
			    </p>
			    <p class="dot">
			      修改域名记录，DNSPro解析权威服务器上是实时生效的，最终生效时间取决于各地运营商的缓存DNS刷新时间；一般可认为修改记录后生效时间为您域名记录之前设置的TTL时间，所以还请耐心等待。
			    </p>
			    <p class="dot">
			      新修改域名DNS指向DNSPro解析的DNS后，我们的DNS服务器的生效时间是实时的，但因各地ISP服务商刷新域名DNS的时间不一致，所以导致解析在全球生效一般需要0--72小时，所以还请您耐心的等待。
			    </p>
			    
			</div>
			<div class="lf helps">
			    <p class="how"><a href="" target="_blank">如何修改DNS？</a></p>
			    <p class="when"><a href="" target="_blank">设置解析后多长时间生效？</a></p>
			    <p class="qa"><a href="" target="_blank">解析常见问题？</a></p>
			</div>
			<a class="btn-close J_btnClose" href="javascript:void(0);" onClick="closeDiv();" >close</a>
		    </div>
		</div>
		
		<div class="table-box" style="margin-bottom:100px;">
			<table cellpadding="0" cellspacing="0" width="100%" >
				<thead>
					<tr id="tr_title">
						<th style="text-align:right;width:88px;white-space:nowrap;">查询类型</th> 
						<th style="text-align:left;">
						<select class="ui-select" style="margin-left:10px;">
							<option selected value="all">按域名</option>
							<option value="recent">按子域名</option>
						</select>
						</th>
						<th style="text-align:right;width:88px;white-space:nowrap;">查询时间</th> 
						<th style="text-align:left;">
						<select class="ui-select" style="margin-left:10px;">
							<option selected value="all">今天</option>
							<option value="recent">本月</option>
							<option value="recent">最近一周</option>
							<option value="recent">最近一月</option>
							<option value="recent">最近一季</option>
							<option value="recent">最近半年</option>
							<option value="recent">最近一年</option>
						</select>
						</th>
						<th style="text-align:right;width:88px;white-space:nowrap;">查询域名</th> 
						<th style="text-align:left;">
						<select class="ui-select" style="margin-left:10px;width:200px;">
							<option selected value="all"><?php echo ($zone); ?></option>
						</select>
						</th>
						<th style="width:220px;text-align:left;">
							<button class="ui-btn-blue" onclick="addDNS('icon-manage')">查询</button>
						</th>
					</tr>
				</thead> 	
				<tbody id="myData">
					<tr class="">
					    <td colspan="7">
						<div id="chart" style="height:320px;margin-top:10px;"></div>
					    </td>
					</tr>
					<tr class="">
					    <td colspan="7">
						<div id="container" style="height:320px;margin-top:10px;"></div>
					    </td>
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
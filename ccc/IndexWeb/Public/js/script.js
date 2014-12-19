$(function(){

	var tophtml="<div id=\"izl_rmenu\" class=\"izl-rmenu\"><a href=\"http://wpa.qq.com/msgrd?v=3&uin=4000662212&site=qq&menu=yes\" title='点击咨询' target='_blank' class=\"btn btn-qq\"></a><div class=\"btn btn-wx\"><img class=\"pic\" src='"+ ROOT +"/Public/images/weixin_.png' onclick=\"window.open(\'http://www.efly.cc')\"/></div><div class=\"btn btn-phone\"><div class=\"phone\">400-066-2212 (7 * 24)</div></div></div>";
	$("#top").html(tophtml);
	$("#izl_rmenu").each(function(){
		$(this).find(".btn-wx").mouseenter(function(){
			$(this).find(".pic").fadeIn("fast");
		});
		$(this).find(".btn-wx").mouseleave(function(){
			$(this).find(".pic").fadeOut("fast");
		});
		$(this).find(".btn-phone").mouseenter(function(){
			$(this).find(".phone").fadeIn("fast");
		});
		$(this).find(".btn-phone").mouseleave(function(){
			$(this).find(".phone").fadeOut("fast");
		});
		/*$(this).find(".btn-top").click(function(){
			$("html, body").animate({
				"scroll-top":0
			},"fast");
		});*/
	});
	var lastRmenuStatus=false;
	/*$(window).scroll(function(){//bug
		var _top=$(window).scrollTop();
		if(_top>200){
			$("#izl_rmenu").data("expanded",true);
		}else{
			$("#izl_rmenu").data("expanded",false);
		}
		if($("#izl_rmenu").data("expanded")!=lastRmenuStatus){
			lastRmenuStatus=$("#izl_rmenu").data("expanded");
			if(lastRmenuStatus){
				$("#izl_rmenu .btn-top").slideDown();
			}else{
				$("#izl_rmenu .btn-top").slideUp();
			}
		}
	});*/
});
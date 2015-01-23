
//分页的显示与隐藏
$("#xifenye").click(function(a){
	$("#uljia").empty();
	$("#xab").toggle();
	//显示出的一共多少页
	var uljia=$("#uljia");
	var page=parseInt($("#xiye").html());//获取当前的页数
	var pages=parseInt($("#mo").html());//获取当前的总页数
	for(var i=1;i<=pages;i++)
	{
		var H="<li  onclick='fl("+i+","+pages+",this)'>"+i+"</li>";//添加一共多少页和点击事件
		
		uljia.append(H);
	}
	scrolltop(page);
})
//点击分页显示的方法
function fl(p1,p2,obj){
	//var p=p1;
	$("#xiye").empty();
	$("#xiye").html(p1);//给显示的页数赋值
	selectByPage($("#txt_zone").val(),p1,$("#orderBy").val());
	stopPropagation(obj);
}
//分页的的上一页和下一页
function topclick(){
	var v=document.getElementById("xiye");
	var num=v.innerHTML;
	if(num>1){		
		num--;
		v.innerHTML=num;
		var hei=25*num-25;
		$("#xab").scrollTop(hei);		
		selectByPage($("#txt_zone").val(),num,$("#orderBy").val());
	}	
}
function downclick(){
	var pages=parseInt($("#mo").html());//获取当前的总页数
	var v=$("#xiye");
	var num=parseInt(v.html());
	if(num < pages){		
		num = ++num;
		v.html(num);
		scrolltop(num);
		selectByPage($("#txt_zone").val(),num,$("#orderBy").val());
	}
}
function selectByPage(val,page,ord){
	var url = window.location.href;
	switch(true){
		case url.indexOf('Domain/detail')!=-1:		
		window.location.href = APP + "/Domain/detail?d=" + val + "&p=" + page + "&o=" + ord + "&t=" + $("#sel_type").val() + "&h=" + $("#searchTxt").val() + "&v=" + $("#sel_view").val();
		break;
		case url.indexOf('ReverseDomain/index')!=-1:		
		window.location.href = APP + "/ReverseDomain/index?p=" + page;
		break;
		case url.indexOf('Optlog/index')!=-1:		
		window.location.href = APP + "/Optlog/index?zone=" + url.getQuery('zone') + "&p=" + page + "&startTime=" + $("#startTime").val() + "&endTime=" + $("#endTime").val() + "&val=" + $("#txt_val").val();
		break;
	}
	
}
//分页的的首页和未页
$("#first").bind("click",function(){
	var v=document.getElementById("xiye");
	v.innerHTML=1;
	scrolltop(v.innerHTML);
	selectByPage($("#txt_zone").val(),1,$("#orderBy").val());
});
$("#last").bind("click",function(){
	var v=document.getElementById("xiye");
	var l=document.getElementById("mo");
	v.innerHTML=l.innerHTML;
	scrolltop(v.innerHTML);	
	selectByPage($("#txt_zone").val(),$("#mo").html(),$("#orderBy").val());
});
//滚动条
function scrolltop(top){
	var hei=25*top-25;
	$("#xab").scrollTop(hei);
}
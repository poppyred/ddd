<?php if (!defined('THINK_PATH')) exit();?><!doctype html>

<!--[if lt IE 7]> <html class="no-js lt-ie9 lt-ie8 lt-ie7" lang="zh_CN"> <![endif]-->
<!--[if IE 7]>    <html class="no-js lt-ie9 lt-ie8" lang="zh_CN"> <![endif]-->
<!--[if IE 8]>    <html class="no-js lt-ie9" lang="zh_CN"> <![endif]-->
<!--[if gt IE 8]><!-->
<html class="no-js" lang="zh-CN">
<!--<![endif]-->
<head>

	<meta charset="utf-8">
	<meta http-equiv="X-UA-Compatible" content="IE=Edge,chrome=1">
	<meta name="renderer" content="webkit">

	<title>EflyDNS - 速度最快的智能DNS解析服务商,免费DNS,安全,可靠,智能划分</title>
	<meta name="description" content="EflyDNS - 速度最快的智能DNS解析服务商,免费DNS,安全,可靠,智能划分">
	<link rel="shortcut icon" href="__ROOT__/Public/images/eflydns.ico" />

	<link href="__ROOT__/Public/css/outter-global.css" media="screen" rel="stylesheet" type="text/css" />

	<script src="__ROOT__/Public/js/sea.js" id="seajsnode"></script>

	<!--[if lt IE 9]>
	<script type="text/javascript" src="__ROOT__/Public/js/html5.js"></script>
	<![endif]--> 
<link href="__ROOT__/Public/css/detail.css" rel="stylesheet" type="text/css" />

<style type="text/css">
    /*
    .more_info{cursor:pointer}
    .hide_info{display:none;}
    .data_info{display:none;}
    */

    .hid{
        display:none;
    }
    .clear{
        clear:both;
    }
    /* begin flashbox */
    .flash-message{
        padding:5px;
        font-size:12px;
        font-weight:bold;
        color:#333;
        height:20px;
        /* float:left; */
        position:relative;
        top:0px;
        left:400px;
        width:200px;
    }
    .flash-message span{
        background:#ffffbc;
        padding:4px;
        padding-left:20px;
        padding-right:20px;
        border-radius:3px;
    }
    .ajaxstatus{
        padding:10px;
        padding-top:2px;
        padding-bottom:0px;
        font-weight:bold;
        color:#333;
        height:20px;
        position:absolute;
        display:inline-block;
        top:0px;
        left:0px;
        margin:0px;
        /* float:left; */
        color:black;
        font-size:13px;
        font-weight:bold;
        -webkit-border-radius:2px 2px;
        -moz-border-radius:2px 2px;
        border-radius:2px 2px;
    }
    .ajaxstatus img{
        display:none;
    }
    .ajaxstatus.loading{
        background-color:#FE8
    }
    .ajaxstatus.error{
        color:white;
        background-color:#B00
    }
    .ajaxstatus.error img{
        display:inline-block;
    }
    .ajaxstatus.error em{
        font-style:normal;
    }
    .ajaxstatus .close{
        position:relative;
        right:-1px;
        top:1px;
        cursor:pointer;
    }
    .ajaxstatus.success{
        color:white;
        background-color:#349900;
    }
    /* end flashbox */

    .defaulttable{
        border:1px solid #d0d0d2;
        width:84%;
        box-shadow:0px 1px 1px #ddd;
        background-color:#fff;
        margin:30px 97px;
    }
    .defaulttable thead{
        background-color:#f7f7f7;
        height:38px;
    }
    .defaulttable thead th{
        font-weight:bold;
        text-align:left;
    }
    .defaulttable th,
    .defaulttable td{
        padding:10px;
        border-bottom:1px solid #d0d0d2;
    }

    .defaulttable tr:hover{
        background-color:#f1f1f1;
    }

    /*begin domaindiag
    * ==========================*/
    .diag {
        min-height:200px;
        font-size:14px;
    }
    .diag .inputs{
        height:60px;
        width:700px;
        margin:0 auto 20px auto;
    }
    .diag .line{
        border-bottom:1px solid #eee;
    }
    .diag .inputs input{
        height:30px;
        width:500px;
        font-size:16px;
        margin-right:20px;
    }
    .diag .inputs button{
        height:40px;
        font-size:16px;
    }
    .small .inputs{
        margin:0 0 0 97px;
    } 
    .small .inputs input{
        height: 20px;
        width: 200px;
        font-size: 14px;
    }
    .domain_status{
        height:50px;
        width:500px;
        margin:30px 0 0 25px;
    }
    .small .inputs button{
        height: 30px;
        font-size: 14px;
    }
    .diag .result ul li{
        line-height:2em;
    }
    .diag .desc{
        height:50px;
        width:500px;
        margin:30px 0 0 25px;
    }
    .diag .desc p{
        height:42px;
        line-height:42px;
        font-size:20px;
        font-weight:bold;
        margin:0 20px;
    }
    .diag .desc .ok{
        width:51px;
        height:42px;
    }
    .diag .desc .warn{
        width:51px;
        height:42px;
    }

    .diag .result td p{
        clear:both;
        height:30px;
        line-height:30px;
        float:none;
        overflow:hidden;
    }
    .diag #whoisresult p span{
        float:left;
        width:100px;
        line-height:30px;
    }
    .diag .whoisresultdesc{
        background-color:#f1f1f1;
        border:1px solid #efefef;
        padding:10px;
    } 
    .diag .http_result_desc{
        background-color:#f1f1f1;
        border:1px solid #efefef;
        padding:10px;
        margin-top:10px;
    } 
    .diag #ns-tip{
        background-color:#f1f1f1;
        border:1px solid #efefef;
        padding:10px;
        margin-top:10px;
    } 
    .diag .digtrace_result_desc{
        background-color:#f1f1f1;
        border:1px solid #efefef;
        padding:10px;
        margin-top:10px;
    } 
    .diag .httpresult dl{
        clear:both;
        margin:5px 0; 
        overflow:hidden;
    }
    .diag .httpresult dl dt{
        float:left;
        width:150px;
    }
    .diag .httpresult dl dd{
        float:left;
        width:200px;
        white-space:pre;
    }
    .diag .other_qtype{
        margin-top:5px;
    }
    .diag .dnsresult td p{
        font-weight:bold;
        border-bottom:1px solid #ddd;
    }
    .diag .result td ul li {
        clear:both;
    }
    .diag .result td ul li span{
        line-height:30px;
        width:250px;
        float:left;
    }
    .diag .diagcate{
        width:100px;
    }
    .diag .result td ul li div{
        float:left;
        width:300px;
        word-break:break-all;
    }
    .diag .defaulttable tr:hover{
        background-color:#fff;
    }
    .diag .nslookupresult i{
        margin-right:3px; 
        margin-top:-2px;
        width:20px;
        height:16px;
        float:left;
    }
    .diag .result .ok{
        width:16px;
        height:16px;
    }
    .diag .result .warn,.diag .result .error{
        width:16px;
        height:16px;
        }
    #help-inline{
        color: #B94A48;
        display:none;
    }
    .warning_info {
        color : #B94A48!important;
    }
    /*end domaindiag */  
    
    /*
    .ui-progress span.ui-labels {
        font-size: 0.8em;
        right: 0;
        line-height: 18px;
        padding-right: 12px;
        color: rgba(0,0,0,0.6);
        text-shadow: rgba(255,255,255, 0.45) 0 1px 0px;
        white-space: nowrap;
    }
    .ui-progress span.ui-label {
        font-size: 0.8em;
        position: absolute;
        right: 0;
        line-height: 18px;
        padding-right: 12px;
        color: rgba(0,0,0,0.6);
        text-shadow: rgba(255,255,255, 0.45) 0 1px 0px;
        white-space: nowrap;
    }
    @-webkit-keyframes animate-stripes {
        from {
            background-position: 0 0;
        }
    
        to {
            background-position: 44px 0;
        }
    }
    
    .ui-progress-bar {
        height: 20px;
        padding-right: 2px;
        background-color: #abb2bc;
        border-radius: 35px;
        -moz-border-radius: 35px;
        -webkit-border-radius: 35px;
        background: -webkit-gradient(linear, left bottom, left top, color-stop(0, #b6bcc6), color-stop(1, #9da5b0));
        background: -moz-linear-gradient(#9da5b0 0%, #b6bcc6 100%);
        -webkit-box-shadow: inset 0px 1px 2px 0px rgba(0, 0, 0, 0.5), 0px 1px 0px 0px #FFF;
        -moz-box-shadow: inset 0px 1px 2px 0px rgba(0, 0, 0, 0.5), 0px 1px 0px 0px #FFF;
        box-shadow: inset 0px 1px 2px 0px rgba(0, 0, 0, 0.5), 0px 1px 0px 0px #FFF;
    }
    
    .ui-progress {
        position: relative;
        display: block;
        height: 19px;
        -moz-border-radius: 35px;
        -webkit-border-radius: 35px;
        border-radius: 35px;
        -webkit-background-size: 44px 44px;
        background-color: #74d04c;
        background: -webkit-gradient(linear, 
                    left bottom, left top, 
                    color-stop(0, #74d04c), 
                    color-stop(1, #9bdd62));
    }
    */

    /*
    #tooltip_info .tooltip {
        position: absolute;
        z-index: 1030;
        display: block;
        font-zie: 11px;
        line-height: 1.4;
        float:left;
        opacity: 0;
        filter: alpha(apacity=0);
        visibility: visible;
    }
    #tooltip_info .tooltip.in {
        opacity: 0.8;
        filter: alpha(opacity=80);
    }
    #tooltip_info .tooltip.top {
        padding: 0 2px;
        margin-top: -55px;
        margin-left: 110px;
    }
    #tooltip_info .tooltip-inner {
        max-width: 250px;
        padding-left: 4px;
        padding-right: 4px;
        color: #ffffff;
        text-align: center;
        text-decoration: none;
        background-color: #000000;
        -webkit-border-radius: 4px;
           -moz-border-radius: 4px;
                border-radius: 4px;
    }
    #tooltip_info .tooltip-arrow {
        position: absolute;
        width: 0;
        height: 0;
        border-color: transparent;
        border-style: solid;
    }
    #tooltip_info .tooltip.top .tooltip-arrow {
        top: 50%;
        left: 0;
        margin-top: -5px;
    }
    .domain_status_icon {
        width: 22px!important;
        height: 20px;
        padding: 5px 3px;
    }
    #tooltip_where {
        margin-top: -27px;
        margin-left: 0px;
        width:400px
    }
    #dnssx_result {
        background-color: #F1F1F1;
        border: 1px solid #EFEFEF;
        padding: 30px;
        margin-top: 10px;
    }
    #dnssx_result p {
        font-weight: bold;
        border-bottom: 1px solid #DDD;
    }
    .icon_style {
        width: 18px;
        height: 18px;
        padding: 3px 2px;
    }
    */
   /* ==========================*/


    /* tooltip */
    .relate-info-show {
        position: absolute;
        padding: 10px;
        left: 10px;
        top: 10px;
        background: #fffae2;
        border: 1px solid #f4eed0;
        line-height: 18px;
        z-index: 10;
        width: 400px;
        color: #000;
    }


    /* progress bar */
    @-webkit-keyframes progress-bar-stripes {
        from {
            background-position: 40px 0
        }

        to {
            background-position: 0 0
        }
    }

    @-moz-keyframes progress-bar-stripes {
        from {
            background-position: 40px 0
        }

        to {
            background-position: 0 0
        }
    }

    @-ms-keyframes progress-bar-stripes {
        from {
            background-position: 40px 0
        }

        to {
            background-position: 0 0
        }
    }

    @-o-keyframes progress-bar-stripes {
        from {
            background-position: 0 0
        }

        to {
            background-position: 40px 0
        }
    }

    @keyframes progress-bar-stripes {
        from {
            background-position: 40px 0
        }

        to {
            background-position: 0 0
        }
    }

    .progress {
        height: 20px;
        /*margin-bottom: 20px;*/
        overflow: hidden;
        /*background-color: #f7f7f7;
        background-image: -moz-linear-gradient(top,#f5f5f5,#f9f9f9);
        background-image: -webkit-gradient(linear,0 0,0 100%,from(#f5f5f5),to(#f9f9f9));
        background-image: -webkit-linear-gradient(top,#f5f5f5,#f9f9f9);
        background-image: -o-linear-gradient(top,#f5f5f5,#f9f9f9);
        background-image: linear-gradient(to bottom,#f5f5f5,#f9f9f9);*/
        /*background-color: #d8d8d8;
        background-image: -moz-linear-gradient(top,#d5d5d5,#dbdbdb);
        background-image: -webkit-gradient(linear,0 0,0 100%,from(#f5f5f5),to(#dbdbdb));
        background-image: -webkit-linear-gradient(top,#f5f5f5,#dbdbdb);
        background-image: -o-linear-gradient(top,#f5f5f5,#dbdbdb);
        background-image: linear-gradient(to bottom,#f5f5f5,#dbdbdb);
        filter: progid:DXImageTransform.Microsoft.gradient(startColorstr='#d5d5d5',endColorstr='#dbdbdb',GradientType=0);*/

        background-color: #abb2bc;
        background-image: -moz-linear-gradient(top,#9da5b0,#b6bcc6);
        background-image: -webkit-gradient(linear,0 0,0 100%,from(#9da5b0),to(#b6bcc6));
        background-image: -webkit-linear-gradient(top,#9da5b0,#b6bcc6);
        background-image: -o-linear-gradient(top,#9da5b0,#b6bcc6);
        background-image: linear-gradient(to bottom,#9da5b0,#b6bcc6);
        filter: progid:DXImageTransform.Microsoft.gradient(startColorstr='#9da5b0',endColorstr='#b6bcc6',GradientType=0);

        -webkit-box-shadow: inset 0px 1px 2px 0px rgba(0, 0, 0, 0.5), 0px 1px 0px 0px #FFF;
        -moz-box-shadow: inset 0px 1px 2px 0px rgba(0, 0, 0, 0.5), 0px 1px 0px 0px #FFF;
        box-shadow: inset 0px 1px 2px 0px rgba(0, 0, 0, 0.5), 0px 1px 0px 0px #FFF;

        background-repeat: repeat-x;
        -webkit-border-radius: 4px;
        -moz-border-radius: 4px;
        border-radius: 4px;
        /*filter: progid:DXImageTransform.Microsoft.gradient(startColorstr='#fff5f5f5',endColorstr='#fff9f9f9',GradientType=0);*/
        /*-webkit-box-shadow: inset 0 1px 2px rgba(0,0,0,0.1);
        -moz-box-shadow: inset 0 1px 2px rgba(0,0,0,0.1);
        box-shadow: inset 0 1px 2px rgba(0,0,0,0.1);*/

        #d5d5d5,#DADADA)
    }

    .progress .bar {
        float: left;
        width: 0;
        height: 100%;
        font-size: 12px;
        color: #fff;
        text-align: center;
        text-shadow: 0 -1px 0 rgba(0,0,0,0.25);
        background-color: #0e90d2;
        background-image: -moz-linear-gradient(top,#149bdf,#0480be);
        background-image: -webkit-gradient(linear,0 0,0 100%,from(#149bdf),to(#0480be));
        background-image: -webkit-linear-gradient(top,#149bdf,#0480be);
        background-image: -o-linear-gradient(top,#149bdf,#0480be);
        background-image: linear-gradient(to bottom,#149bdf,#0480be);
        background-repeat: repeat-x;
        filter: progid:DXImageTransform.Microsoft.gradient(startColorstr='#ff149bdf',endColorstr='#ff0480be',GradientType=0);
        -webkit-box-shadow: inset 0 -1px 0 rgba(0,0,0,0.15);
        -moz-box-shadow: inset 0 -1px 0 rgba(0,0,0,0.15);
        box-shadow: inset 0 -1px 0 rgba(0,0,0,0.15);
        -webkit-box-sizing: border-box;
        -moz-box-sizing: border-box;
        box-sizing: border-box;
        -webkit-transition: width .6s ease;
        -moz-transition: width .6s ease;
        -o-transition: width .6s ease;
        transition: width .6s ease
    }

    .progress .bar+.bar {
        -webkit-box-shadow: inset 1px 0 0 rgba(0,0,0,0.15),inset 0 -1px 0 rgba(0,0,0,0.15);
        -moz-box-shadow: inset 1px 0 0 rgba(0,0,0,0.15),inset 0 -1px 0 rgba(0,0,0,0.15);
        box-shadow: inset 1px 0 0 rgba(0,0,0,0.15),inset 0 -1px 0 rgba(0,0,0,0.15)
    }

    .progress-striped .bar {
        background-color: #149bdf;
        background-image: -webkit-gradient(linear,0 100%,100% 0,color-stop(0.25,rgba(255,255,255,0.15)),color-stop(0.25,transparent),color-stop(0.5,transparent),color-stop(0.5,rgba(255,255,255,0.15)),color-stop(0.75,rgba(255,255,255,0.15)),color-stop(0.75,transparent),to(transparent));
        background-image: -webkit-linear-gradient(45deg,rgba(255,255,255,0.15) 25%,transparent 25%,transparent 50%,rgba(255,255,255,0.15) 50%,rgba(255,255,255,0.15) 75%,transparent 75%,transparent);
        background-image: -moz-linear-gradient(45deg,rgba(255,255,255,0.15) 25%,transparent 25%,transparent 50%,rgba(255,255,255,0.15) 50%,rgba(255,255,255,0.15) 75%,transparent 75%,transparent);
        background-image: -o-linear-gradient(45deg,rgba(255,255,255,0.15) 25%,transparent 25%,transparent 50%,rgba(255,255,255,0.15) 50%,rgba(255,255,255,0.15) 75%,transparent 75%,transparent);
        background-image: linear-gradient(45deg,rgba(255,255,255,0.15) 25%,transparent 25%,transparent 50%,rgba(255,255,255,0.15) 50%,rgba(255,255,255,0.15) 75%,transparent 75%,transparent);
        -webkit-background-size: 40px 40px;
        -moz-background-size: 40px 40px;
        -o-background-size: 40px 40px;
        background-size: 40px 40px
    }

    .progress.active .bar {
        -webkit-animation: progress-bar-stripes 2s linear infinite;
        -moz-animation: progress-bar-stripes 2s linear infinite;
        -ms-animation: progress-bar-stripes 2s linear infinite;
        -o-animation: progress-bar-stripes 2s linear infinite;
        animation: progress-bar-stripes 2s linear infinite
    }

    /* error page */
    /*body {background: #f9fee8;margin: 0; padding: 20px; text-align:center; font-family:Arial, Helvetica, sans-serif; font-size:14px; color:#666666;}*/
    .error_page {width: 600px; padding: 50px; margin: auto; text-align: center; display: none;}
    .error_page h1 {margin: 20px 0 0; font-size: 24px;}
    .error_page p {margin: 10px 0; padding: 0;text-align: center; width: 100%;}
    /*a {color: #9caa6d; text-decoration:none;}
    a:hover {color: #9caa6d; text-decoration:underline;}*/


</style>

</head>

<body>


  <!-- Topbar ================================================== -->
    <header style="[style]">

    <nav class="outter-nav">
      <a href="__APP__/Index/index" class="brand">
        <img src="__ROOT__/Public/images/EflyDNS.png" width="140" height="36" alt="" title="">
      </a>

      <ul role="navigation" class="support">
        <li class="index"><a href="__APP__/Index/index">首&nbsp;&nbsp;&nbsp;&nbsp;页</a></li>
        <li class="plan"><a href="__APP__/Product/product">购&nbsp;买&nbsp;套&nbsp;餐</a></li>
        <li class="support"><a href="__APP__/Help/help">帮&nbsp;助&nbsp;中&nbsp;心</a></li>
      </ul>

      <?php if(empty($_SESSION['user'])): ?><ul class="info" style="display:none;">
      <?php else: ?>
        <ul class="info"><?php endif; ?>
        <li>
            <span onclick="location='http://www.eflydns.com/client/Domain/domainList';">回到我的域名</span>
        </li>
      </ul>

      <?php if(empty($_SESSION['user'])): ?><ul class="account">
      <?php else: ?>
        <ul class="account" style="display:none;"><?php endif; ?>
        <li>
          <a href="http://www.eflydns.com/client/Index/register" class="signup">注册</a>
        </li>

        <li class='login_li' >
          <a href="http://www.eflydns.com/client/Index/login" id='login-widget'>登录</a>

         <div class='topbar-dropdown'>
            <form id="form-login-widget" class="form-stacked" method="POST" action='/Login'>
                <div class="clearfix">
                  <label for="email">E-Mail</label>
                  <div class="input">
                    <input class="large" id="email" name="email" size="30" type="text" />
                    <span class="help-inline"></span>
                  </div>
                </div><!-- /clearfix -->
                <div class="clearfix">
                  <label for="password">密码</label>
                  <div class="input">
                    <input class="large" id="password" name="password" size="30" type="password" />
                    <span class="help-inline"></span>
                  </div>
                </div><!-- /clearfix -->
                <div class="clearfix">
                  <div class="input remember ">
                    <label class="clearfix">
                      <input type="checkbox" name="remember" value="1" />
                      <span>一个月内自动登录</span>
                    </label>
                    <a href='#'>忘记密码</a>
                  </div>
                </div>
                <div class="">
                  <button type="submit" class='btn primary large' >登 录</button>
                </div>
            </form>
         </div>
        </li>

      </ul>
    </nav>

    <div class="bg"></div>
    
  </header>


<script>
  seajs.use(['jquery.js', 'cookie.js'], function($, cookie) {

      var ua = $.browser;
      if ( !(ua.msie && /^[67]/.test(ua.version)) ) {
        if (ua.msie && /^[9]/.test(ua.version)) {
            $('#email').keyup(function(e){
              if (e.keyCode==13) $('#password').focus();
            });
            $('#password').keyup(function(e){
              if (e.keyCode==13) $('#form-login-widget').submit();
            });
        }
        
        var $dropdown = $('.topbar-dropdown');
        var $login = $('#login-widget');
        var $form = $('#form-login-widget');

        $form.find('input, button').attr('tabindex', 1);

        //点击登陆按钮
        $login.click(function(e){
          //return;//不显示dropdown直接跳转

          e.preventDefault();
          e.stopPropagation();
          //button actived
          $(this).closest('.login_li').toggleClass('login-widget-show');
          //login in widget show up
          $dropdown.toggleClass('show');
          $form.find(':input[value=""]:first').select();
         
         if($(this).closest('.login_li').hasClass('login-widget-show')){
            $(this).css('color', '#000');
         }else{
            $(this).css('color', '#fff');
         }
         
        });


        //点击窗口其他地方关闭dropdown
        $(document).click(function(e) {
          var $target = $(e.target);
          if (!$target.closest('.login_li').length && $dropdown.hasClass('show')) {
            $login.closest('.login_li').removeClass('login-widget-show');
            $dropdown.removeClass('show');

            $login.css('color', '#fff');
          }
        });


        //提交登陆表单
        $form.submit(function(e) {
          e.preventDefault();
          $form.find('button[type=submit]').attr('disabled', true).addClass('disabled');
          $.ajax({
            url: '__APP__/Index/login',
            data: $form.serialize(),
            dataType: 'json',
            type: 'post',
            success: function(data) {
                //console.log(data);

                if (data.status == 'success') {
                  cookie.set('peach', $form.find('input[name=email]').val(), {
                    expires: 3650
                  });

                  //window.location.href = data.string + window.location.hash;

                  //$(".outter-nav .account").hide();
                  //$(".outter-nav .info").show();
                  window.location.href = 'http://www.eflydns.com/client/Domain/domainList';

                } else {
                  //cookie.set('loginError', data.string);

                  window.location.href = 'http://www.eflydns.com/client/Index/login';
                }
            }
          });

        });

        var last_login_email = cookie.get('peach');
        if (last_login_email) {
          $('input[name=email]').val(last_login_email);
        }

      } // end if ie6

  });

</script>



  <!-- Page ================================================== -->
  <div role="main">
    <div class='container pages support-pages'>
      <div class='guide'>
        <h1>域名健康诊断</h1>
        <p>为您提供全面的智能化域名诊断，域名健康情况一目了然。</p>
      </div>

      <div class='main-content'>
        <div class='wrapper'>
          <div class="row">

            <section class="sub-content">
                <nav>
                    <div style="margin-top:10px;margin-left:-10px;margin-bottom:10px">
                        <ul class="yarnball">
                            <li class="yarnlet first">
                                <a href="__URL__/help" style="z-index: 9;">
                                    <span class="left-yarn">
                                    </span>
                                    帮助中心
                                </a>
                            </li>
                            <li class="yarnlet ">
                                <a style="z-index: 8;">
                                    域名诊断
                                </a>
                            </li>
                        </ul>
                    </div>
                </nav>


              <div class="diag small">
                <div class="inputs clear">
                  <form>
                    <input type='text' id='domain_zhenduan' style="width: 300px;"
                      name='domain' value='<?php echo ($domain); ?>' placeholder='请输入要诊断的域名，例如：eflydns.com'
                      tabindex='1' />
                    <button type='submit' class="domaindiag btn primary"
                      id="domaindiag">域名诊断</button>
                    <div>
                    <div id='help-inline' style="margin-right: 10px;">请输入正确域名</div>
                    <!--
                      域名解析不到IP？试试
                      <a href="#">DNS检测工具</a>吧！
                    -->
                    </div>
                  </form>
                </div>

                <div class="line hid" style="display: block; "></div>
                <div class="desc hid">
                  <div class="msg_status"></div>
                </div>

                <table border="0" class="defaulttable result hid clear" style="display: none;">
                    <thead>
                        <tr>
                            <th></th>
                            <th>
                                项目
                            </th>
                            <th>
                                结果
                            </th>
                        </tr>
                    </thead>
                    <tbody class="info_zhenduan" id="info_zhenduan">
                        <tr class="domain_info">
                            <td class="ok">
                                <img src="__ROOT__/Public/images/help/ok.jpg">
                            </td>
                            <td class="diagcate">
                                最后更新时间
                            </td>
                            <td class="whois_info">
                                <div class="data_info">
                                </div>
                                <span class="warning_info"></span>
                            </td>
                        </tr>
                        <tr class="domain_info">
                            <td class="ok">
                                <img src="__ROOT__/Public/images/help/ok.jpg">
                            </td>
                            <td>
                                NS状态
                            </td>
                            <td>
                                <div class="data_info" id="ns-tip">
                                </div>
                                <span class="warning_info"></span>
                            </td>
                        </tr>
                        <tr class="domain_info">
                            <td class="ok">
                                <img src="__ROOT__/Public/images/help/ok.jpg">
                            </td>
                            <td>
                                注册日期
                            </td>
                            <td>
                                <div class="data_info">
                                </div>
                                <span class="warning_info"></span>
                            </td>
                        </tr>
                        <tr class="domain_info">
                            <td class="ok">
                                <img src="__ROOT__/Public/images/help/ok.jpg">
                            </td>
                            <td>
                                到期日期
                            </td>
                            <td>
                                <div class="data_info">
                                </div>
                                <span class="warning_info"></span>
                            </td>
                        </tr>
                        <tr class="domain_info">
                            <td class="ok">
                                <img src="__ROOT__/Public/images/help/ok.jpg">
                            </td>
                            <td>
                                域名时间
                            </td>
                            <td>
                                <div class="data_info">
                                    <div class="progress progress-striped active">
                                        <div class="bar" style="width: 20%; position: relative;">
                                            <span style="width: 140px;line-height: 20px;font-size: 12px;color: #fff;right: 5px;">
                                                离域名到期还有
                                                <b class="value">
                                                    336天
                                                </b>
                                            </span>
                                        </div>
                                    </div>
                                </div>
                                <span class="warning_info"></span>
                            </td>
                        </tr>
                        <tr class="domain_info">
                            <td class="ok">
                                <img src="__ROOT__/Public/images/help/ok.jpg">
                            </td>
                            <td>
                                域名状态
                            </td>
                            <td>
                                <div class="data_info">

                                </div>
                                <span class="warning_info"></span>
                            </td>
                        </tr>
                    </tbody>
                </table>

                <div style="color:#5e5e5e; border-bottom:1px solid #5e5e5e; width: 84%; margin: 0 auto;margin: 0 auto -5px; height: 25px; display:none;">详细英文注册信息如下</div>
                <div id="originInfo" style="color:#5e5e5e; width: 84%; margin: 0 auto 20px; display:none;"></div>

                <!--错误页面-->
                <div class="error_page">
                    <img alt="Kidmondo_face_sad" src="__ROOT__/Public/images/help/kidmondo_face_sad.gif">
                    <h1>
                        出错啦，请稍后重试
                    </h1>
                    <p>The page or journal you are looking for cannot be found.</p>
                </div>

                <!--
                <div id="ressurvey">
                  <div style="margin:0 0 10px 93px;">
                    <h4 style="display:inline">如果上面的诊断无法解决您的问题，您还可以</h4>
                    <a style="font-weight:bold" href='#'>通过向导解决问题</a>
                  </div>
                </div>
                -->

              </div>

            </section>

          </div>
          <!--end of a row-->
        </div>
        <!-- end of wrapper -->
      </div>
      <!-- end of main-content -->
    </div>
    <!-- END OF CONTAINER -->
  </div>
  <!-- END OF MAIN -->



  <!-- footer ================================================== -->
      <!-- <footer> -->
      <!--
      <nav>
        <ol class="breadcrumbs">
            <li class="home">首页</li>
        </ol>

        <div class="directorynav">
          <div id="dn-cola" class="column first">
            <h3>为什么使用 EflyDNS</h3>
            <ul>
              <li><a href="#">谁在使用 EflyDNS</a></li>
              <li><a href="#">快速入门</a></li>
            </ul>
          </div>
          <div id="dn-colb" class="column">
            <h3>产品介绍</h3>
            <ul>
              <li><a href="#">智能 DNS</a></li>
              <li><a href="#">D监控</a></li>
              <li><a href="#">D令牌</a></li>
              <li><a href="#">套餐服务</a></li>
              <li><a href="#">企业服务</a></li>
            </ul>
          </div>
          <div id="dn-colc" class="column">
            <a href="#"><h3>帮助中心</h3></a>
            <ul>
              <li><a href="#">常见问题</a></li>
              <li><a href="#">DNS 工具</a></li>
              <li><a href="#">服务器状态</a></li>
              <li><a href="#">本地DNS优化</a></li>
            </ul>
          </div>
          <div id="dn-cold" class="column">
            <h3>其他</h3>
            <ul>
              <li><a href="#">API</a></li>
              <li><a href="#">客户端</a></li>
              <li><a href="#">手机版</a></li>
              <li><a href="#">开源</a></li>
            </ul>
          </div>
          <div id="dn-cole" class="column">
            <h3>关于</h3>
            <ul>
              <li><a href="#">关于我们</a></li>
              <li><a href="#">官方博客</a></li>
              <li><a href="#">工作机会</a></li>
              <li><a href="#">合作伙伴</a></li>
              <li><a href="#">友情链接</a></li>
            </ul>
          </div>
          <div id="dn-colf" class="column last">
            <h3>联系我们</h3>
            <ul>
              <li>   
                <a href="#">80000000</a>
              </li>
              <li><a href="#">技术支持</a></li>
              <li><a href="#">投诉建议</a></li>
              <li class="weibo">
                 <a href="#"></a>  
                 <a href="#"></a> 
                 <a href="#"></a> 
                 <a href="#"></a> 
                 <a href="#"></a> 
              </li>
            </ul>
          </div>
          <div class="c"></div>
        </div>
      </nav>
      -->

      <!--
      <section class="links">
        <h3><a href="#">合作伙伴：</a></h3>
        <a href="#">站长之家</a>
        <a href="#">海波IDC</a>
        <a href="#">站长网</a>
        <a href="#">英拓网络</a>
        <a href="#">中客科技</a>
        <a href="#">BGP双线</a>
        <a href="#">零刻数据</a>
        <a href="#">亚洲诚信</a>
        <a href="#">炎黄网络</a>
        <a href="#">五月广告联盟</a>
        <a href="#">腾佑科技</a>
        <a href="#">维派科技</a>
        <a href="#">51IDC</a>
        <a href="#">虚拟主机</a>
        <a href="#">cnBeta</a>
      </section>
      -->
      
      <!--       
      <section class="copyright">
        <div><span style="font-family: arial;font-size: 14px;">©</span> 2007-2014 广东睿江科技有限公司版权所有</div>
        
        <div>&copy; 2014 <a href="#">Report Abuse</a> </div>
        <div>SSL Powered by <a href="#">TrustAsia</a></div>
        <div>• 昨天共提供了<a href="#">17,690,777,127</a>次查询服务 • 今年提供了3,406,230,418,485次查询服务</div>
        </table>
      </section> 
      -->
    <!-- </footer> -->

      <style>
        #footerdiv {
            width: 100 % ;
            min-width: 1000px;
            background-color: #2e2e2e;
            background-image: url(__ROOT__/Public/images/footbg.jpg);
            background-repeat: repeat-x;
            background-position: top;
            font-size: 12px;
            font-family: "Microsoft YaHei", Arial, Helvetica, sans-serif;
        }
        #footer {
            width: 1000px;
            margin: 0 auto;
            text-align: left;
            color: #999999;
            padding: 20px 0px 10px;
        }
        #footer #fleft {
            width: 600px;
            float: left;
            line-height: 25px;
        }
        #footer #fmiddle {
            width: 24px;
            float: right;
            line-height: 26px;
            padding-top: 10px;
            padding-right: 10px;
        }
        #footer #fright {
            margin-top: 7px;
            line-height: 18px;
            width: 300px;
            height: 36px;
            font-size: 13px;
            padding-left: 28px;
            float: right;
            background-image: url(__ROOT__/Public/images/tel.jpg);
            background-repeat: no-repeat;
            background-position: left;
        }
        #footer #fright #worktime {
            font-size: 11px;
        }
        #footer #fright li {
            float: left;
            padding-left: 10px;
            line-height: 18px;
        }
      </style>

      <footer>
        <section id="footerdiv">
          <div id="footer">
              <span id="fleft">
                  Copyright © 广东睿江科技有限公司 粤ICP备09026812号&nbsp;&nbsp;ISO9001国际标准质量管理体系认证
                  <br>
                  增值电信业务经营许可证(IDC、ISP) 编号：粤B1.B2-20070326
              </span>
              <span id="fright">
                  <ul>
                      <li>
                          全国客户服务热线
                          <br>
                          <font id="worktime">
                              TIME: 7*24 HOURS
                          </font>
                      </li>
                      <li>
                          <img src="__ROOT__/Public/images/telnum.jpg" width="164" height="32">
                      </li>
                  </ul>
              </span>
              <span id="fmiddle">
                  <a href="tencent://message/?uin=1207197276">
                      <img border="0" src="__ROOT__/Public/images/qq.jpg" alt="" width="24" height="26">
                  </a>
              </span>
              <div style="clear: both; font-size: 0px; line-height: 0px; height: 0px;"></div>
          </div>
        </section>
      </footer>


       <style>
         #feedback {
           position:fixed;
           right:20px;
           top:92%;
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

      <div id="feedback" style="cursor:pointer;[displayfb]" onclick="location='tencent://message/?uin=1207197276';">
        <a href="javascript:void(0);">客服咨询</a>
      </div>


      <script type="text/javascript">

        //获取浏览器窗口尺寸，通用！
        function findDimensions() //函数：获取尺寸
        {
          // 获取窗口宽度
          if (window.innerWidth)
            winWidth = window.innerWidth;
          else if ((document.body) && (document.body.clientWidth))
            winWidth = document.body.clientWidth;
          
          // 获取窗口高度
          if (window.innerHeight)
            winHeight = window.innerHeight;
          else if ((document.body) && (document.body.clientHeight))
            winHeight = document.body.clientHeight;
                   
          // 通过深入Document内部对body进行检测，获取窗口大小
          if (document.documentElement  && document.documentElement.clientHeight && document.documentElement.clientWidth)
          {
                winHeight = document.documentElement.clientHeight;
                winWidth = document.documentElement.clientWidth;
          }

          return {"winWidth": winWidth, "winHeight": winHeight};
        }

        var footer = document.getElementById('footerdiv');

        //底部自适应，保持在屏幕底部(不出滚动条时)，或在文档最底(出滚动条时)
        var intervalId = setInterval(function(){
          //console.log(new Date().getTime());

          var size = findDimensions();

          if(size.winHeight < document.body.offsetHeight){
            //出现滚动条
            footer.className = '';
            //clearInterval(intervalId);//出现滚动条后停止任务
          }else{
            footer.className = 'fix_bottom';
          }
        }, 300);

      </script>

      <!-- 百度统计 -->
      <div style="display:none;">
        <script type="text/javascript">
            var _bdhmProtocol = (("https:" == document.location.protocol) ? " https://" : " http://");
            document.write(unescape("%3Cscript src='" + _bdhmProtocol + "hm.baidu.com/h.js%3F4507d6bdc805c13e1bdbbe784201ee2f' type='text/javascript'%3E%3C/script%3E"));
        </script>
      </div>
      


  <script>
      seajs.use('jquery.js', function($){

          $(function(){
            /*
            $(".more_info").live("click",function(){
                $(this).hide();
                $(this).parents("td").find(".hide_info").show();
                $(this).parents("td").find(".data_info").show();
            });
            $(".hide_info").live("click",function(){
                $(this).hide();
                $(this).parents("td").find(".more_info").show();
                $(this).parents("td").find(".data_info").hide();
            });
            $("#progress_bar").live("mouseover",function(){
                document.getElementById("tooltip-box").style.display="block";
            });
            $("#progress_bar").live("mouseout",function(){
                document.getElementById("tooltip-box").style.display="none";
            });
            $(".dig_whois").live("click",function(){
                $("#whoisresult").show();
                $("#whoisresult").parents("td").find(".hide_info").show();
                $("#whoisresult").parents("td").find(".more_info").hide();
                $("#whoisresult").parents("td").focus();
            });
            $(".dig_record").live("click",function(){
                $("#record_result").show();
                $("#record_result").parents("td").find(".hide_info").show();
                $("#record_result").parents("td").find(".more_info").hide();
                $("#record_result").parents("td").focus();
            });
            */

            //tooltips
            $("img[src*='question-tip.png']").live({
                mousemove : function(e){
                    var x = e.pageX + 10;
                    var y = e.pageY + 5;

                    $(this).parent().next().css({
                        'left': x + 'px',
                        'top': y + 'px'
                    }).show();
                },
                mouseleave : function(){
                    $(this).parent().next().hide();
                }
            });

            $("#domaindiag").click(function(e){
                e.preventDefault();//阻止默认行为

                //隐藏信息
                $("#info_zhenduan").parent().hide();//基本信息
                $("#originInfo").hide().prev().hide();//原始信息
                $("div.error_page").hide();//错误页面

                if($("#help-inline").is(":visible")){
                    $("#help-inline").hide();
                }

                var domain = $("#domain_zhenduan").val().trim().toLocaleLowerCase();
                var doname = /(^([a-zA-Z0-9-\u4e00-\u9fa5]+\.)+([a-zA-Z-\u4e00-\u9fa5]+)$)/;
                var flag_domain = doname.test(domain);
                if(!flag_domain){
                    $("#help-inline").css("display", "inline");
                    $("#help-inline").show();
                    return;
                }


                //等待效果
                //var loading = '<div id="loading" style="width:200px; margin:20px auto 0; color:#5e5e5e;">结果加载中，请稍后。。。</div>';
                var loading = '<div id="loading" style="width:200px;margin:20px auto 0;"><img src="__ROOT__/Public/images/loading.gif"/></div>';
                if($('#loading').size() == 0){
                    $("#info_zhenduan").parent().before(loading);
                }

                $.ajax({
                    url : '__URL__/get_whois_info',
                    type: 'GET',
                    data: {'domain' : domain},
                    dataType: 'json',
                    success: function(data){
                        //console.log(data);

                        if(data.Ret){
                            //出错
                            $("div.error_page p").html(data.RetMsg);
                            $("div.error_page").show();
                            $("#info_zhenduan").parent().prev('#loading').remove();
                            return;
                        }

                        //最后更新时间
                        var $div = $("#info_zhenduan").find("tr").eq(0).children("td:last").find("div.data_info");
                        var html = '<p>' + data.Updated['time'] + '</p>';
                        $div.html(html);
                        $div.next().html(data.Updated['msg']);
                        if(data.Updated['msg']){
                            $div.parent().siblings(':first').children('img').attr('src', '__ROOT__/Public/images/help/warning.jpg');
                        }else{
                            $div.parent().siblings(':first').children('img').attr('src', '__ROOT__/Public/images/help/ok.jpg');
                        }

                        //NS状态
                        $div = $("#info_zhenduan").find("tr").eq(1).children("td:last").find("div.data_info");
                        html = '';
                        for(var k in data.Nameservers){
                            if(k !== 'msg'){
                                html += '<p>' + data.Nameservers[k] + '</p>';
                            }
                        }
                        $div.html(html);
                        $div.next().html(data.Nameservers['msg']);
                        if(data.Nameservers['msg']){
                            $div.parent().siblings(':first').children('img').attr('src', '__ROOT__/Public/images/help/warning.jpg');
                        }else{
                            $div.parent().siblings(':first').children('img').attr('src', '__ROOT__/Public/images/help/ok.jpg');
                        }

                        //注册日期
                        $div = $("#info_zhenduan").find("tr").eq(2).children("td:last").find("div.data_info");
                        html = '<p>' + data.BeginDate + '</p>';
                        $div.html(html);

                        //到期日期
                        $div = $("#info_zhenduan").find("tr").eq(3).children("td:last").find("div.data_info");
                        html = '<p>' + data.EndDate + '</p>';
                        $div.html(html);

                        //域名时间
                        $(".progress .bar").css('width', (data.TotalDays - data.LeaveDays) / data.TotalDays * 100 + '%');
                        $(".progress .bar").find('b').html(data.LeaveDays + '天');

                        //域名状态
                        $div = $("#info_zhenduan").find("tr").eq(5).children("td:last").find("div.data_info");
                        html = '';
                        for(var i = 0; i < data.StatusInfo.length; i++){
                            html += '<p>' + 
                                '<span style="margin-right:5px;">' + data.StatusInfo[i].desc + '(' +  data.StatusInfo[i].status + ')</span>' + 
                                '<img src="__ROOT__/Public/images/help/question-tip.png" alt="">' + 
                            '</p>' + 
                            '<div class="relate-info-show" style="display: none;">' + data.StatusInfo[i].tip + '</div>';
                        }

                        $div.html(html);

                        //原始信息
                        //$div = $("#info_zhenduan").find("tr").eq(6).children("td:last").find("div.data_info");
                        $("#originInfo").html(data.OriginalInfo);


                        //显示信息
                        $("#info_zhenduan").parent().show();//基本信息
                        $("#originInfo").show().prev().show();//原始信息
                        $("#info_zhenduan").parent().prev('#loading').remove();
                    }
                });

            });


            //如果有域名传入，直接查询
            if($("#domain_zhenduan").val()){
                $("#domaindiag").click();
            }
        });

      });
      
  </script>

</body>
</html>
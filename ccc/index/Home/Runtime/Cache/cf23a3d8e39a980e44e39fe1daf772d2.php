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

    <style>

      /* 购买流程 */
      .tab_wrapper {
        width: 1090px;
        margin: 0 auto;
      }
        .tabs {
            height: 48px;
        }
        .tabs li {
            float: left;
            width: 25%;
            border-bottom: solid 1px #eeeeee;
            position: relative;
            margin-left: -1px;
            z-index: 0;
            border-top: 1px solid #eeeeee;
            border-right: 1px solid #eeeeee;
            background-color: #fbfaf8;
        }
        .tabs li.selected {
            z-index: 9;
            border-right: 1px solid #eeeeee;
            border-top: 2px solid #00a2ca;
            border-bottom: 1px solid white;
            background-color: white;
        }
        .tabs li.selected a {
            color: #000;
        }
        
        .tabs li:hover {
            background-color: white;
            color: #4D97A2;
        }
        .tabs li a {
            padding-top: 14px;
            border-left: 1px solid #eeeeee !important;
            text-decoration: none;
            height: 32px;
            display: block;
            text-align: center;
            cursor: pointer;
            font-size: 14px;
        }
        .tabs-content {
            border: solid 1px #eeeeee;
            background-color: white;
            margin-top: -1px;
            padding-top: 1px;
        }
        .tabs-content div {
            text-align: center;
            padding: 30px 0;
            display: none;
        }
        .tabs-content div.selected {
            display: block;
        }

        .tabs-content div ul {
            width: 1030px;
            height: 42px;
            margin: 5px auto;
            padding-left: 75px;
        }

        .tabs-content div ul li {
            float: left;
            width: 122px;
            margin-right: 150px;
            text-align: left;
        }

        .tabs-content div ul li a {
            font-size: 14px;
        }

        .tabs-content div ul li a:hover {
          text-decoration: underline;
        }


        /* 购买流程end */


      /* 常见问题 */
      .page-conts .cont-item-3 {
          padding-top: 85px;
          padding-bottom: 50px;
          background: url(http://gtms01.alicdn.com/tps/i1/T150wXFmJlXXXS9Bc_-131-49.png) center 0 no-repeat
      }

      .page-stage {
          width: 996px;
          margin: 0 auto;
      }

      .fix {
          zoom: 1
      }

      .cont-item-3 .faq-item {
          width: 332px
      }

      .lf {
          float: left;
          display: inline
      }

      .cont-item-3 .faq-item {
          width: 320px
      }

      .cont-item-3 .faq-item .tit {
          font-size: 22px;
          color: #5E5E5E;
          font-weight: 300;
          text-align: center;
          padding-top: 140px;
          /*padding-bottom: 20px;*/
          background: url(__ROOT__/Public/images/help/icon4.png) -1000px -1000px no-repeat;

          margin-bottom: 10px;
          margin-top: 0;
      }

      .cont-item-3 .faq-item-1 .tit {
          background-position: 40px -18px;
      }

      .cont-item-3 .faq-item-2 .tit {
          background-position: -298px -18px;
      }

      .cont-item-3 .faq-item-3 .tit {
          background-position: -638px -18px;
      }

      .cont-item-3 .faq-item .list {
          border-right: dashed 1px #2f82d5;
          padding: 10px 10px 10px 75px;
          height: 116px
      }

      .cont-item-3 .faq-item .no-border {
          border-right: none 0
      }

      .cont-item-3 .faq-item .list li {
          line-height: 29px;
          height: 29px;
          padding-left: 15px;
          font-size: 14px;
          background: url(http://gtms03.alicdn.com/tps/i3/T10iqjFpVdXXctpfrr-4-4.gif) 0 center no-repeat
      }

      .cont-item-3 .faq-item .list a {
          color: #06c;
      }

      .cont-item-3 .faq-item .list a:hover {
          text-decoration: underline;
      }

      .cont-item-3 .btm-link {
          text-align: center;
          font-size: 16px;
          padding-top: 20px;
          padding-right: 50px
      }

      .cont-item-3 .btm-link a {
          color: #f60
      }
      /* 常见问题end */


      /* 自助服务new */
        ul.help_tools {

        }

      ul.help_tools li {
        float: left;
        /* margin: 0 31px; */
        border: 1px solid #e7e7e7;
        border-right: none;
        /*padding: 20px 30px;*/

        -webkit-box-shadow: 0px 3px 3px rgba(0, 0, 0, 0.1);
        -moz-box-shadow: 0px 3px 3px rgba(0, 0, 0, 0.1);
        box-shadow: 0px 3px 3px rgba(0, 0, 0, 0.1);

        *width: 270px;/*ie7*/
      }

      ul.help_tools li.last {
        border-right: 1px solid #e7e7e7;
      }

      ul.help_tools li a {
        display: block;
        padding: 35px 70px 25px;
      }

      ul.help_tools li a:hover {
        background: #43A4E9;
        text-decoration: none;

        -webkit-box-shadow: 0 0 10px rgba(67, 164, 233, 0.7);
        -moz-box-shadow: 0 0 10px rgba(67, 164, 233, 0.7);
        box-shadow: 0 0 10px rgba(67, 164, 233, 0.7);
        /*box-shadow: inset 0 -10px 5px rgba(255, 255, 255, 0.7);*/

        -webkit-transition: transform .25s ease-out;
        -moz-transition: transform .25s ease-out;
        transition: transform .25s ease-out;

        -webkit-transform: scale(1.07);
        -moz-transform: scale(1.07);
        transform: scale(1.07);

        /*-webkit-animation: myfirst 1s linear 0 infinite;
        animation: myfirst 1s linear 0 infinite;*/
      }

      /*@keyframes myfirst {
          0% {
            transform: scale(1);
          }
          50% {
            transform: scale(0.5);
          }
          100% {
            transform: scale(1.07);
          }
      }

      @-webkit-keyframes myfirst {
          0% {
            -webkit-transform: scale(1);
          }
          50% {
            -webkit-transform: scale(0.5);
          }
          100% {
            -webkit-transform: scale(1.07);
          }
      }*/

      ul.help_tools li a > div {
        display: block;
        height: 130px;
        width: 130px;
        margin: 0 auto;
      }

      ul.help_tools li a > p {
        margin: 20px auto;
        float: none;
        text-align: center;
        font-size: 22px;
        color: #5E5E5E;

        /*-webkit-transition: color .25s linear;
        -moz-transition: color .25s linear;
        transition: color .25s linear;*/
      }

      ul.help_tools li a.t1 > div {
        background: url(__ROOT__/Public/images/help/icon2.png) no-repeat -5px -5px;
      }

      ul.help_tools li a.t2 > div {
        background: url(__ROOT__/Public/images/help/icon2.png) no-repeat -5px -285px;
      }

      ul.help_tools li a.t3 > div {
        background: url(__ROOT__/Public/images/help/icon2.png) no-repeat -5px -145px;
      }

      ul.help_tools li a.t4 > div {
        background: url(__ROOT__/Public/images/help/icon2.png) no-repeat -5px -425px;
      }

      ul.help_tools li a:hover > p {
        color: #fff;
      }

      ul.help_tools li a.t1:hover > div {
        background-position: -145px -5px;
      }

      ul.help_tools li a.t2:hover > div {
        background-position: -145px -285px;
      }

      ul.help_tools li a.t3:hover > div {
        background-position: -145px -145px;
      }

      ul.help_tools li a.t4:hover > div {
        background-position: -145px -425px;
      }
      /* 自助服务end */

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


  <!-- Page
    ================================================== -->
  <div role="main">
    <div class='container pages support-pages'>
      <div class='guide'>
        <h1>客户服务中心</h1>
        <p>这里有最专业的服务团队，7×24小时响应故障排查及业务咨询服务，并提供自助诊断工具及详细用户帮助文档。</p>
        <img src="__ROOT__/Public/images/help/phone.png" width="100" height="100" style="position: absolute; top: -20px;left: 780px;" />
      </div>

      <div class='main-content'>
        <div class='wrapper'>

            <!-- 常用自助服务 -->
            <section class="sub-content">
                <h2 class="title">自助服务</h2>

                <nav class="content-box" style="width:1090px; height: 254px; margin-top: 0;">
                    <ul class="help_tools">
                        <li>
                          <a href="__URL__/tools" class="t1">
                            <div></div>
                            <p>域名诊断</p>
                          </a>
                        </li>
                        <li>
                          <a href="__URL__/manual?51" class="t2">
                            <div></div>
                            <p>API文档</p>
                          </a>
                        </li>
                        <li>
                            <a href="tencent://message/?uin=1207197276&Site=wendns.com&Menu=yes" class="t3">
                              <div></div>
                              <p>在线咨询</p>
                            </a>
                        </li>
                        <li class="last">
                          <a href="__URL__/manual?51" class="t4">
                              <div></div>
                              <p>用户手册</p>
                          </a>
                        </li>
                    </ul>
                </nav>
            </section>


            <section class="sub-content">
                <h2 class="title">快速上手</h2>

                <div class="tab_wrapper content-box">
                    <ul class="tabs">
                        <li class="selected"><a href="javascript:void(0)">简单四步，即可让您的域名提速</a></li>
                        <li style="display:none;"><a href="javascript:void(0)" >备案流程</a></li>
                        <li style="display:none;"><a href="javascript:void(0)" >备案流程</a></li>
                        <li style="display:none;"><a href="javascript:void(0)" >备案流程</a></li>
                    </ul>
                    <div class="tabs-content">
                        <div class="selected">
                            <img src="__ROOT__/Public/images/help/icon5.png">
                            <ul>
                                <li>
                                    <a href="__URL__/manual?51">
                                        登陆Eflydns官方网站，注册会员
                                    </a>
                                </li>
                                <li>
                                    <a href="__URL__/manual?52">
                                        注册成功后，在系统中添加您的域名和相应解析记录
                                    </a>
                                </li>
                                <li>
                                    <a href="__URL__/manual?53">
                                        登陆域名注册商，把DNS解析服务器地址指向Eflydns
                                    </a>
                                </li>
                                <li style="margin-right:0px;">
                                    <a href="__URL__/manual?54">
                                        静候 0-72 小时的全球生效时间
                                    </a>
                                </li>
                            </ul>
                        </div>
                        <div>2</div>
                        <div>3</div>
                        <div>4</div>
                    </div>
                </div>
            </section>


            <section class="sub-content">
              <h2 class="title">常见问题</h2>

              <div class="cont-item cont-item-3 content-box" style="display: block; height:385px;">
                <div class="page-stage fix">
                  <div class="faq-item faq-item-1 lf">
                    <h3 class="tit">使用配置</h3>
                    <ul class="list" style="padding-left:34px;">
                      <li><a href="__URL__/manual?22">EflyDNS的DNS解析地址是什么</a></li>
                      <li><a href="__URL__/manual?41">万网注册的域名如何使用EflyDNS解析</a></li>
                      <li><a href="__URL__/manual?42">新网注册的域名如何使用EflyDNS解析</a></li>
                      <li><a href="__URL__/manual?43">Godaddy注册的域名如何使用EflyDNS</a></li>
                      <li><a href="__URL__/manual?44">易名中国注册的域名如何使用EflyDNS</a></li>
                      <li><a href="__URL__/manual?45">琥珀网注册的域名如何使用EflyDNS</a></li>
                    </ul>
                  </div>
                  <div class="faq-item faq-item-2 lf">
                    <h3 class="tit">生效时间</h3>
                    <ul class="list">
                      <li><a href="__URL__/manual?24">修改后，解析何时生效</a></li>
                      <li><a href="__URL__/manual?25">导致解析不能生效的原因</a></li>
                    </ul>
                  </div>
                  <div class="faq-item faq-item-3 lf">
                    <h3 class="tit">产品相关</h3>
                    <ul class="list no-border">
                      <li><a href="__URL__/manual?21">什么是智能DNS解析</a></li>
                      <li><a href="__URL__/manual?10">智能DNS和普通DNS的区别</a></li>
                      <li><a href="__URL__/manual?23">EflyDNS支持哪些解析类型</a></li>
                    </ul>
                  </div>
                </div>

                <!--
                <div class="btm-link page-stage"><a href="http://www.net.cn/service/faq/yuming/url/201402/6305.html" target="_blank">更多常见问题&gt;&gt;</a></div>
                -->
            </section>
        

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
            $("ul.help_tools li a").hover(function(){
              var index = $("ul.help_tools li a").index(this);
              if(index === 1){
                index = 2;
              }else if(index === 2){
                index = 1;
              }

              //$(this).children("p").css('color', '#fff');
              $(this).children('div').css('background', 'url(__ROOT__/Public/images/help/icon2.png) no-repeat -145px ' + (-5 + index * -140) + 'px');
            }, function(){
              $(this).children("div").removeAttr('style');
              //$(this).children("p").removeAttr('style');
            });
            */

            $("div.tab_wrapper ul.tabs li").click(function(){
                var index = $("div.tab_wrapper ul.tabs li").index(this);

                $(this).addClass('selected').siblings().removeClass('selected');
                $('div.tab_wrapper .tabs-content div').eq(index).addClass('selected').siblings().removeClass('selected');
            });

          });

      });
      
    </script>

</body>
</html>
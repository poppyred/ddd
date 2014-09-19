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
  <link href="__ROOT__/Public/css/manual.css" rel="stylesheet" type="text/css" />

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
          <div class='row'>

            <section class="sub-content" style="margin-bottom: 15px;">
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
                                  用户手册
                              </a>
                          </li>
                      </ul>
                  </div>
              </nav>
            </section>

            <div class="help_box">
              <div class="help_box_left">
                  <div class="title">
                  </div>
                  <div class="guide1">

                      <div class="open">
                          <div class="title menulist">
                              <div class="cat" href="javascript:;">
                                  快速上手，仅需四步
                              </div>
                          </div>
                          <div class="content">
                              <ul>
                                  <li onclick="loadContent(this, 51);">
                                      <a href="javascript:void(0);">
                                          注册账户
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 52);">
                                      <a href="javascript:void(0);">
                                          添加记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 53);">
                                      <a href="javascript:void(0);">
                                          设置解析
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 54);">
                                      <a href="javascript:void(0);">
                                          静待生效
                                      </a>
                                  </li>
                              </ul>
                          </div>
                      </div>

                      <div class="open">
                          <div class="title menulist">
                              <div class="cat" href="javascript:;">
                                  各注册商设置解析方法
                              </div>
                          </div>
                          <div class="content">
                              <ul>
                                  <li onclick="loadContent(this, 41);">
                                      <a href="javascript:void(0);">
                                          万网注册的域名如何使用Eflydns
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 42);">
                                      <a href="javascript:void(0);">
                                          新网注册的域名如何使用Eflydns
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 43);">
                                      <a href="javascript:void(0);">
                                          Godaddy注册的域名如何使用Eflydns
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 44);">
                                      <a href="javascript:void(0);">
                                          易名中国注册的域名如何使用Eflydns
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 45);">
                                      <a href="javascript:void(0);">
                                          琥珀网注册的域名如何使用Eflydns
                                      </a>
                                  </li>
                              </ul>
                          </div>
                      </div>

                      <div class="open">
                          <div class="title menulist">
                              <div class="cat" href="javascript:;">
                                  记录添加
                              </div>
                          </div>
                          <div class="content">
                              <ul>
                                  <li onclick="loadContent(this, 31);">
                                      <a href="javascript:void(0);">
                                          添加A记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 32);">
                                      <a href="javascript:void(0);">
                                          添加AAAA记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 33);">
                                      <a href="javascript:void(0);">
                                          添加CNAME记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 34);">
                                      <a href="javascript:void(0);">
                                          添加MX记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 35);">
                                      <a href="javascript:void(0);">
                                          添加NS记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 36);">
                                      <a href="javascript:void(0);">
                                          添加TXT记录
                                      </a>
                                  </li>
                              </ul>
                          </div>
                      </div>

                      <div class="open">
                          <div class="title menulist">
                              <div class="cat" href="javascript:;">
                                  常见问题
                              </div>
                          </div>
                          <div class="content">
                              <ul>
                                  <li onclick="loadContent(this, 21);">
                                      <a href="javascript:void(0);">
                                          什么是智能DNS解析
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 22);">
                                      <a href="javascript:void(0);">
                                          EflyDNS的DNS解析地址是什么
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 23);">
                                      <a href="javascript:void(0);">
                                          EflyDNS支持哪些解析类型
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 24);">
                                      <a href="javascript:void(0);">
                                          修改后，解析何时生效
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 25);">
                                      <a href="javascript:void(0);">
                                          导致解析不能生效的原因
                                      </a>
                                  </li>
                              </ul>
                          </div>
                      </div>

                      <div class="open">
                          <div class="title menulist">
                              <div class="cat" href="javascript:;">
                                  基础知识
                              </div>
                          </div>
                          <div class="content">
                              <ul>
                                  <li onclick="loadContent(this, 1);">
                                      <a href="javascript:void(0);">
                                          什么是DNS
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 2);">
                                      <a href="javascript:void(0);">
                                          什么是A记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 3);">
                                      <a href="javascript:void(0);">
                                          什么是AAAA记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 4);">
                                      <a href="javascript:void(0);">
                                          什么是CNAME记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 5);">
                                      <a href="javascript:void(0);">
                                          什么是MX记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 6);">
                                      <a href="javascript:void(0);">
                                          什么是NS记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 7);">
                                      <a href="javascript:void(0);">
                                          什么是TXT记录
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 8);">
                                      <a href="javascript:void(0);">
                                          什么是TTL
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 9);">
                                      <a href="javascript:void(0);">
                                          域名的构成规则
                                      </a>
                                  </li>
                                  <li onclick="loadContent(this, 10);">
                                      <a href="javascript:void(0);">
                                          智能DNS和普通DNS的区别
                                      </a>
                                  </li>
                              </ul>
                          </div>
                      </div>
                      
                  </div>
              </div>

              <div class="help_box_right">
                  <div class="title"></div>
                  <button class="back_btn" onclick="goback();" onfocus="this.blur();">返回</button>
                  <div class="content"></div>
              </div>
          </div>

            
          </div>
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
        var Stack = function(){
            this.data = [];
            this.top = -1;
        };

        Stack.prototype = {
            init: function(){
                this.data = [];
                this.top = -1;
            }
            ,isEmpty: function(){
                return this.top < 0;
            }
            ,push: function(x){
                this.top += 1;
                this.data[this.top] = x;
            }
            ,pop: function(){
                if(this.isEmpty()){
                    return false;
                }

                var t = this.getTop();
                this.top -= 1;
                this.data.length -= 1;//删除元素
                return t;
            }
            ,getTop: function(){
                if(this.isEmpty()){
                    return false;
                }

                return this.data[this.top];
            }
        };

        var mark = new Stack();

        //var mark = [];//点击菜单项记录

        seajs.use('jquery.js', function($){

          $(function(){
              $(".menulist").click(function(){
                var classType = $(this).parent().attr("class");
                if(classType=="open"){
                  $(this).parent().attr("class", "close");
                }else{
                  $(this).parent().attr("class", "open");
                }

                //$(this).parent().siblings().attr("class", "close");
              });


              //点击子菜单项
              $(".help_box_left .guide1 .content li").live('click', function(e){
                $(".help_box_left .guide1 .content li").removeClass('on');
                $(this).addClass('on');

                //如自身未展开，则展开
                var menu = $(this).parent().parent().parent();
                if(menu.hasClass('close')){
                    menu.attr("class", "open");
                    //menu.attr("class", "open").siblings().attr("class", "close");
                }

                //回到顶部
                window.scrollTo(0, 0);

                //记录点击路径
                var id = parseInt($(this).attr('onclick').replace(');', '').split(',')[1].trim());
                if(mark.getTop() != id){
                    mark.push(id);
                }
                /*if(!mark.length || id != mark[mark.length - 1]){
                  mark.push(id);
                }*/
                //console.log(mark);
              });

              //初始展开
              clickItem(location.search ? location.search.substring(1) : 51);
          });

          //加载内容
          window.loadContent = function(obj, id){
              $(".help_box_right .content").load('q' + id, function(){
                  //$(this).find('.title').html($(obj).text());
                  $(this).siblings('.title').html($(obj).text());
              });
          };

          //点击子菜单项
          window.clickItem = function(id){
              $("li[onclick='loadContent(this, " + id + ");']").click();
          };

      });

      //返回
      function goback(){
        /*mark.pop();//弹出当前记录

        if(mark.length){
          clickItem(mark.pop());//再点击上一条
        }*/

        mark.pop();

        if(!mark.isEmpty()){
            clickItem(mark.pop());
        }else{
          location = '__URL__/help';//没上一页回到帮助中心
        }
      }
      
    </script>

</body>
</html>
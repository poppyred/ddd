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
        .page-stage {
            width: 990px;
            margin: 0 auto;
        }

        .cont-item-2 .sub-tit {
            font-size: 30px;
            color: #06c;
            padding-bottom: 30px;
            padding-top: 10px;
            margin: 0;
            font-weight: bold;
        }

        .cont-item-2 .sub-table {
            border: solid 1px #0066CC;
            border-left: none 0;
            font-size: 14px
        }

        .cont-item-2 .sub-table th {
            text-align: center;
            height: 56px;
            background: #06c;
            color: #fff;
            font-size: 18px
        }

        .cont-item-2 .sub-table img.i-yes {
            display: inline
        }

        .cont-item-2 .sub-table td {
            height: 46px;
            color: #333;
            padding-left: 20px;
            border-left: solid 1px #0066CC;
            border-top: solid 1px #b7d4f1
        }

        .cont-item-2 .sub-table .no-border-top td {
            border-top: none 0
        }

        .cont-item-2 .sub-table td.align-center {
            text-align: center
        }

        .cont-item-2 .sub-table td.lbl {
            padding-left: 25px;
            background: #0066CC;
        }

        .cont-item-2 .sub-table .q-ico {
            display: inline-block;
            /*padding-right: 25px;*/
            cursor: default;
            /*background: url(__ROOT__/Public/images/product/qicon.png) right center no-repeat;*/
        }

        /* 等宽表格 */
        .cont-item-2 .sub-table td {
            /*padding-right: 140px;*/
            width: 246px;
        }

        .cont-item-2 .sub-table td.lbl span {
            color: #fff;
        }

    </style>

</head>

<body>

  <!-- Topbar ================================================== -->
    <header style="[style]">

    <nav class="outter-nav">
      <a href="__APP__/Index/index" class="brand">
        <img src="__ROOT__/Public/images/EflyDNS.png" width="140" height="36" alt="" title="">
      </a>

      <ul role="navigation" class="plan">
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
        <h1>创新是产品生命的源动力</h1>
        <p>根据客户实际需求量身打造的一系列标准化功能，为您域名的每一次的访问保驾护航，满足您一切需求。</p>
        <img src="__ROOT__/Public/images/product/product.png" width="100" height="100" style="position: absolute; top: -16px;left: 780px;" />
      </div>

      <div class='main-content'>
        <div class='wrapper'>


          <div class="cont-item-2">
              <div class="page-stage">
                <h3 class="sub-tit">
                    EflyDNS产品特性
                </h3>
                <table class="sub-table" cellspacing="0" cellpadding="0" width="100%">
                    <tbody>
                        <tr class="no-border-top">
                            <td class="lbl">
                                <span class="q-ico" title="根据网站访问用户的IP所属运营商信息，智能分配对应的运营商线路，使访问更迅速，更通畅">
                                    智能线路细分
                                </span>
                            </td>
                            <td>
                                电信、联通、移动、教育网、长宽
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="支持中文域名的DNS解析服务">
                                    中文域名解析
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="可承受的最大DNS解析并发连接数">
                                    最大防护能力
                                </span>
                            </td>
                            <td>
                                3000000 qps
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="支持不限层级的子域名解析">
                                    子域名解析
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="从用户设置DNS解析记录到解析记录在递归服务器生效的时间">
                                    解析生效时间
                                </span>
                            </td>
                            <td>
                                小于等于5s
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="利用通配符* （星号）来做次级域名以实现所有的次级域名均指向同一IP地址">
                                    泛解析
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="对解析域名不同异常状态的监测和提醒，如域名即将到期、已到期提醒等">
                                    域名状态异常监测
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="将访问您当前域名的用户引导到您指定的另一个网址">
                                    URL转发
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="对多台服务器进行流量分发的负载均衡服务">
                                    负载均衡
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="某一个域名下可添加的解析条数">
                                    解析记录数
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    别名绑定
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    EDNS0/EDNS Client Subnet
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    域名到期提醒
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    DDNS（动态解析）
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    解析量统计
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    TTL
                                </span>
                            </td>
                            <td>
                                最小支持10分钟
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    域名星标
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    域名分组
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    邮件通知
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    专属售前经理
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                        <tr>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    专属售后经理
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                            <td class="lbl">
                                <span class="q-ico" title="">
                                    在线客服
                                </span>
                            </td>
                            <td>
                                <img src="__ROOT__/Public/images/product/ok.png" alt="支持" />
                            </td>
                        </tr>
                    </tbody>
                </table>
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
      

</body>

</html>
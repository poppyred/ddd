<!DOCTYPE html>
<html lang="en">



  <head>
    <meta charset="utf-8">
    <title>Efly DNS Proxy</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="">
    <meta name="author" content="">

    <!-- Le styles -->
    <link href="//cdnjs.bootcss.com/ajax/libs/twitter-bootstrap/2.3.1/css/bootstrap.min.css" rel="stylesheet">
    <style type="text/css">
      body {
        padding-top: 60px;
        padding-bottom: 40px;
      }
    </style>
    <link href="//cdnjs.bootcss.com/ajax/libs/twitter-bootstrap/2.3.1/css/bootstrap-responsive.min.css" rel="stylesheet">

 <style type="text/css">

 
	      body {
        padding-top: 20px;
        padding-bottom: 60px;
      }

      /* Custom container */
      .container {
        margin: 0 auto;
        max-width: 1000px;
      }
      .container > hr {
        margin: 60px 0;
      }
    /* GLOBAL STYLES
    -------------------------------------------------- */
    /* Padding below the footer and lighter body text */

    body {
      padding-bottom: 40px;
      color: #5a5a5a;
    }



    /* CUSTOMIZE THE NAVBAR
    -------------------------------------------------- */

 
      /* Customize the navbar links to be fill the entire space of the .navbar */
      .navbar .navbar-inner {
        padding: 0;
      }
      .navbar .nav {
        margin: 0;
        display: table;
        width: 100%;
      }
      .navbar .nav li {
        display: table-cell;
        width: 1%;
        float: none;
      }
      .navbar .nav li a {
        font-weight: bold;
        text-align: center;
        border-left: 1px solid rgba(255,255,255,.75);
        border-right: 1px solid rgba(0,0,0,.1);
      }
      .navbar .nav li:first-child a {
        border-left: 0;
        border-radius: 3px 0 0 3px;
      }
      .navbar .nav li:last-child a {
        border-right: 0;
        border-radius: 0 3px 3px 0;
      }



    /* CUSTOMIZE THE CAROUSEL
    -------------------------------------------------- */

    /* Carousel base class */
    .carousel {
      margin-bottom: 60px;
    }

    .carousel .container {
      position: relative;
      z-index: 9;
    }

    .carousel-control {
      height: 80px;
      margin-top: 0;
      font-size: 120px;
      text-shadow: 0 1px 1px rgba(0,0,0,.4);
      background-color: transparent;
      border: 0;
      z-index: 10;
    }

    .carousel .item {
      height: 600px;
    }
    .carousel img {
      position: absolute;
      top: 0;
      left: 0;
      min-width: 100%;
      height: 600px;
    }

    .carousel-caption {
      background-color: transparent;
      position: static;
      max-width: 550px;
      padding: 0 20px;
      margin-top: 200px;
    }
    .carousel-caption h1,
    .carousel-caption .lead {
      margin: 0;
      line-height: 1.25;
      color: #fff;
      text-shadow: 0 1px 1px rgba(0,0,0,.4);
    }
    .carousel-caption .btn {
      margin-top: 10px;
    }



    /* MARKETING CONTENT
    -------------------------------------------------- */

    /* Center align the text within the three columns below the carousel */
    .marketing .span4 {
      text-align: center;
    }
    .marketing h2 {
      font-weight: normal;
    }
    .marketing .span4 p {
      margin-left: 10px;
      margin-right: 10px;
    }


    /* Featurettes
    ------------------------- */

    .featurette-divider {
      margin: 80px 0; /* Space out the Bootstrap <hr> more */
    }
    .featurette {
      padding-top: 120px; /* Vertically center images part 1: add padding above and below text. */
      overflow: hidden; /* Vertically center images part 2: clear their floats. */
    }
    .featurette-image {
      margin-top: -120px; /* Vertically center images part 3: negative margin up the image the same amount of the padding to center it. */
    }

    /* Give some space on the sides of the floated elements so text doesn't run right into it. */
    .featurette-image.pull-left {
      margin-right: 40px;
    }
    .featurette-image.pull-right {
      margin-left: 40px;
    }

    /* Thin out the marketing headings */
    .featurette-heading {
      font-size: 50px;
      font-weight: 300;
      line-height: 1;
      letter-spacing: -1px;
    }



    /* RESPONSIVE CSS
    -------------------------------------------------- */

    @media (max-width: 979px) {

      .container.navbar-wrapper {
        margin-bottom: 0;
        width: auto;
      }
      .navbar-inner {
        border-radius: 0;
        margin: -20px 0;
      }

      .carousel .item {
        height: 500px;
      }
      .carousel img {
        width: auto;
        height: 500px;
      }

      .featurette {
        height: auto;
        padding: 0;
      }
      .featurette-image.pull-left,
      .featurette-image.pull-right {
        display: block;
        float: none;
        max-width: 40%;
        margin: 0 auto 20px;
      }
    }


    @media (max-width: 767px) {

      .navbar-inner {
        margin: -20px;
      }

      .carousel {
        margin-left: -20px;
        margin-right: -20px;
      }
      .carousel .container {

      }
      .carousel .item {
        height: 300px;
      }
      .carousel img {
        height: 300px;
      }
      .carousel-caption {
        width: 65%;
        padding: 0 70px;
        margin-top: 100px;
      }
      .carousel-caption h1 {
        font-size: 30px;
      }
      .carousel-caption .lead,
      .carousel-caption .btn {
        font-size: 18px;
      }

      .marketing .span4 + .span4 {
        margin-top: 40px;
      }

      .featurette-heading {
        font-size: 30px;
      }
      .featurette .lead {
        font-size: 18px;
        line-height: 1.5;
      }
	  
	      
    }
    </style>
  </head>
  
  
  <body>

		
      <div class="container">

      <div class="masthead">
        <h3 class="muted">Efly DNS Proxy</h3>
        <div class="navbar">
          <div class="navbar-inner">
            <div class="container">
              <ul class="nav">
                <li ><a href="index.php">产品主页</a></li>
				<li class="active"><a href="#">测试频道</a></li>
				<li class="dropdown">
                <a href="#" class="dropdown-toggle" data-toggle="dropdown">源码提供 <b class="caret"></b></a>
				
                <ul class="dropdown-menu">
                  <li class="nav-header">暂无</li>
                </ul>
				
              </li>
				
				
				<li class="dropdown">
                <a href="#" class="dropdown-toggle" data-toggle="dropdown">负责人 <b class="caret"></b></a>
				
                <ul class="dropdown-menu">
                  <li class="nav-header">项目策划</li>
                  <center><h5><li class = "text-center"><font color="#006666 "><strong>何作祥 <BR>Francis Ho</strong></font></li></h5></center>
				  <li class="nav-header">技术提供</li>
                  <center><h5><li class = "text-center"><font color="#006666 "><strong>黄衍博 <BR>Embert Wong</strong></font></li></h5></center>
				  <li class="nav-header">前端支持</li>
                  <center><h5><li class = "text-center"><font color="#006666 "><strong>欧吉增 <BR>Jizeng Ou</strong></font></li></h5></center>
                </ul>
				
              </li>
				<li class="dropdown">
                <a href="#" class="dropdown-toggle" data-toggle="dropdown">联系我们 <b class="caret"></b></a>
				
                <ul class="dropdown-menu">
				  <li class="nav-header">官方网站</li>
				  <center><h5><li><a onclick="window.open('http://www.efly.cc')">睿江科技有限公司 </a></li></h5></center>
				  <center><li class="divider"></li></center>
                  <li class="nav-header">邮件地址</li>
				  <center><a href="mailto:#">huangyb@efly.cc</a></center>
                </ul>
				
              </li>
              </ul>
            </div>
          </div>
        </div><!-- /.navbar -->
      </div>
	  </div>
	  
	 
	
	

    <script src="http://code.jquery.com/jquery.js"></script>
    <script src="js/bootstrap.min.js"></script>
	


	<div class="container" color = "blue">
	<form action="./test.php" method="post">
	<h1>DNS代理操作</h1>
	<P> 请选择要进行的操作类型</P>
	<select class = "span2" name="type">
		<option value = "1">域名劫持</option>
		<option value = "2">泛解析劫持</option>
		<option value = "3">视图功能</option>
		<option value = "4">掩码信息</option>
	</select>
	<select class = "span1" name="opt">
		<option value = "1">增添</option>
		<option value = "2">插入</option>
		<option value = "3">剔出</option>
		<option value = "4">删除</option>
	</select>
	<input class="span2" type="text" name="view" placeholder="视图ID">
	<input class="span3" type="text" name="domain" placeholder="域名/掩码">
	<input class="span3" type="text" name="ip" placeholder="IP地址">
	<center>
	<button type="submit" class="btn btn-primary span2 offset3"><i class="icon-ok icon-white"></i>  确认</button>
	<button type="reset" class="btn span1"><i class="icon-remove"></i>重填</button>
	</center>
	</form>

	</div>
	
	<br></br>
	<br></br>
	
	<div class="container">
	<center>
	<div class="btn-group">
	
	<a class="btn btn-danger " href = "tester.php?type=1"><i class="  icon-th-list icon-white"></i> 域名记录</a>
	<a class="btn btn-danger " href = "tester.php?type=2"><i class="  icon-th-list icon-white"></i> 泛解析记录</a>
	<a class="btn btn-danger " href = "tester.php?type=3"><i class="  icon-th-list icon-white"></i> 视图信息</a>
	<a class="btn btn-danger " href = "tester.php?type=4"><i class="  icon-th-list icon-white"></i> 掩码信息</a>
	</div>
	</center>
	</div>
	
	<br></br>
	<br></br>
	

	
	
	<?php
		require_once("./display.php");
	?>
	





	<div class="container">
	<BR>
	<footer>
        <p>&copy; Efly-DNS Team 2012-2013</p>
	</footer>
	</div>
	
  </body>
  
</html>
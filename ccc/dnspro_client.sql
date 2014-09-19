/*
SQLyog Ultimate v11.31 (32 bit)
MySQL - 5.1.73 : Database - dnspro_client
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`dnspro_client` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `dnspro_client`;

/*Table structure for table `client` */

DROP TABLE IF EXISTS `client`;

CREATE TABLE `client` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `type` int(4) NOT NULL COMMENT '个人0/企业1',
  `mail` varchar(128) NOT NULL,
  `pwd` varchar(128) NOT NULL,
  `org_name` varchar(128) NOT NULL,
  `org_num` varchar(16) NOT NULL,
  `link_phone` varchar(32) NOT NULL,
  `link_man` varchar(32) NOT NULL,
  `link_weixin` varchar(32) NOT NULL,
  `reg_time` varchar(32) NOT NULL COMMENT '注册时间',
  `login_time` varchar(32) NOT NULL COMMENT '最近登录时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=42 DEFAULT CHARSET=utf8;

/*Data for the table `client` */

insert  into `client`(`id`,`type`,`mail`,`pwd`,`org_name`,`org_num`,`link_phone`,`link_man`,`link_weixin`,`reg_time`,`login_time`) values (10,0,'hyb@qq.com','962012d09b8170d912f0669f6d7d9d07','','','','','','','2014-09-11 16:36:04'),(11,0,'test@efly.cc','e10adc3949ba59abbe56e057f20f883e','','','','','','2014-08-14 11:20:20','2014-09-11 16:36:04'),(12,0,'yanfa@efly.cc','671ae484da91ccf27bb96c687e931b7e','','','','','','2014-08-14 11:20:20','2014-09-17 21:42:09'),(13,1,'oujz@efly.cc','e10adc3949ba59abbe56e057f20f883e','广东睿江科技有限公司','','18565485654','测试员','46235412','2014-08-14 11:20:20','2014-09-11 16:36:04'),(18,0,'likx@efly.cc','e10adc3949ba59abbe56e057f20f883e','','','','','','2014-08-14 11:20:20','2014-09-11 16:36:04'),(19,0,'475386889@qq.com','3755024f3ab2a494a3829a24edb1b65e','','','','','','2014-08-27 14:25:55','2014-09-11 16:36:04'),(20,0,'testing@qq.com','e10adc3949ba59abbe56e057f20f883e','','','','','','2014-08-27 15:46:01','2014-09-11 16:36:04'),(21,0,'352658057@qq.com','0f8fde8612a2ac149d8fe20983af6ba5','','','','','','2014-09-03 10:58:16','2014-09-18 10:40:01'),(22,0,'364691617@qq.com','e10adc3949ba59abbe56e057f20f883e','','','','','','2014-09-03 10:59:29','2014-09-11 16:36:04'),(23,0,'abcd@qq.com','7ac66c0f148de9519b8bd264312c4d64','','','','','','2014-09-03 11:00:47','2014-09-11 16:36:04'),(24,0,'329653147@qq.com','e10adc3949ba59abbe56e057f20f883e','','','','','','2014-09-03 11:05:16','2014-09-11 16:36:04'),(25,0,'jihaoo009@163.com','e10adc3949ba59abbe56e057f20f883e','','','','','','2014-09-03 11:10:39','2014-09-11 16:36:04'),(26,0,'cnfreesw@126.com','a2c91cb0705b4699359d558325060659','','','','','','2014-09-03 14:52:55','2014-09-11 16:36:04'),(27,0,'maij@efly.cc','96e79218965eb72c92a549dd5a330112','','','','','','2014-09-03 15:04:53','2014-09-11 16:36:04'),(28,0,'703696673@qq.com','96e79218965eb72c92a549dd5a330112','','','','','','2014-09-03 17:18:05','2014-09-18 09:06:31'),(29,0,'huangyb@efly.cc','e10adc3949ba59abbe56e057f20f883e','','','','','','2014-09-04 11:46:08','2014-09-12 10:05:07'),(30,0,'qiujw@efly.cc','a74c0f57487a37a20d09180ef7eca8b8','','','','','','2014-09-05 09:44:29','2014-09-11 16:36:04'),(31,0,'dickmo@tom.com','d5242771387c4e88349706a0734af6e3','','','','','','2014-09-05 09:44:32','2014-09-11 16:36:04'),(32,0,'519385024@qq.com','dd4b85ba5e3329c450f67f6e76126dde','','','','','','2014-09-05 09:45:03','2014-09-11 16:36:04'),(33,0,'liuml@efly.cc','b3e61fcb4d0bd879400479954564b7bd','','','','','','2014-09-05 09:46:42','2014-09-11 16:36:04'),(34,0,'254762787@qq.com','05cc9e2a9b11ca652ea2a9b45ef369ea','','','','','','2014-09-05 09:49:19','2014-09-11 16:36:04'),(35,0,'396639205@qq.com','da53ab095ded4bec21fede813e69c0bf','','','','','','2014-09-05 09:56:16','2014-09-11 16:36:04'),(36,0,'huojc@efly.cc','be55eae0818f3329bc0c6029ecd9f7fe','','','','','','2014-09-05 10:11:39','2014-09-11 16:36:04'),(37,0,'18824109@qq.com','7869db9af9f4708b312f8c1d45adcbc8','','','','','','2014-09-05 11:07:20','2014-09-11 16:36:04'),(38,0,'huozf@efly.cc','376936c2118b01a4d8aec5b0d9ecd0d3','','','','','','2014-09-05 16:36:24','2014-09-11 16:36:04'),(39,0,'yuan_haoliang@163.com','74a028ca534127ffc46adaf8b832664e','','','','','','2014-09-05 17:29:01','2014-09-11 16:36:04'),(40,0,'790927151@qq.com','a875ce62550d120090529949d0947cca','','','','','','2014-09-06 16:11:00','2014-09-11 16:36:04'),(41,0,'yehb@efly.cc','3fde6bb0541387e4ebdadf7c2ff31123','','','','','','2014-09-09 10:24:12','2014-09-16 10:23:21');

/*Table structure for table `client_domain` */

DROP TABLE IF EXISTS `client_domain`;

CREATE TABLE `client_domain` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `client_id` int(16) NOT NULL,
  `zone_id` int(16) NOT NULL,
  `domain_id` int(16) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1004 DEFAULT CHARSET=utf8;

/*Data for the table `client_domain` */

insert  into `client_domain`(`id`,`client_id`,`zone_id`,`domain_id`) values (838,10,295,576),(839,10,295,577),(840,10,295,578),(841,10,295,579),(843,10,295,581),(847,10,295,585),(877,12,307,615),(878,12,307,616),(879,12,307,617),(880,12,308,618),(881,12,308,619),(882,12,308,620),(895,12,312,633),(896,12,312,634),(897,12,312,635),(899,12,307,637),(900,12,307,638),(901,13,315,643),(902,13,315,644),(903,13,315,645),(912,19,319,654),(913,19,319,655),(914,12,320,656),(915,12,320,657),(916,12,320,658),(919,20,321,661),(920,20,321,662),(921,12,320,663),(924,12,308,666),(933,13,322,675),(934,13,322,676),(935,12,323,677),(936,12,323,678),(937,12,320,679),(938,13,324,680),(939,13,324,681),(940,13,315,682),(941,21,325,683),(942,21,325,684),(943,21,325,685),(944,22,326,686),(945,22,326,687),(946,22,326,688),(947,24,327,689),(948,24,327,690),(950,12,328,692),(951,12,328,693),(952,26,329,694),(953,26,329,695),(954,12,312,696),(955,36,330,697),(956,36,330,698),(957,37,331,699),(958,37,331,700),(959,38,332,701),(960,38,332,702),(961,38,332,703),(966,12,312,708),(967,12,312,709),(968,12,320,710),(969,12,320,711),(970,12,320,712),(971,12,320,713),(972,12,320,714),(973,12,320,715),(974,12,320,716),(975,28,335,717),(976,28,335,718),(977,28,335,719),(978,28,335,720),(979,12,320,721),(980,28,336,722),(981,28,336,723),(982,12,307,724),(983,12,337,725),(984,12,337,726),(985,12,337,727),(986,12,337,728),(987,12,337,729),(990,12,337,732),(991,12,337,733),(992,12,337,734),(1000,28,341,742),(1001,28,341,743),(1002,12,308,744),(1003,12,337,745);

/*Table structure for table `client_group` */

DROP TABLE IF EXISTS `client_group`;

CREATE TABLE `client_group` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `client_id` int(16) NOT NULL,
  `group` varchar(64) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8;

/*Data for the table `client_group` */

insert  into `client_group`(`id`,`client_id`,`group`) values (13,28,'测试分组'),(14,12,'测试分组');

/*Table structure for table `domain` */

DROP TABLE IF EXISTS `domain`;

CREATE TABLE `domain` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `host` varchar(128) NOT NULL COMMENT '主机记录',
  `type` varchar(16) NOT NULL COMMENT '记录类型',
  `view` int(8) NOT NULL COMMENT '系统自带的路线',
  `route` int(16) NOT NULL COMMENT '自定义路线',
  `val` varchar(128) NOT NULL COMMENT '记录值',
  `mx` int(4) NOT NULL COMMENT 'mx优先级',
  `ttl` int(4) NOT NULL COMMENT 'ttl值，分钟',
  `is_edit` int(4) NOT NULL DEFAULT '1' COMMENT '是否可以修改，是1/否0',
  `is_on` int(4) NOT NULL DEFAULT '1' COMMENT '启用1/暂停0',
  `desc` varchar(512) NOT NULL,
  `up_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '最近修改时间，防止频繁修改操作',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=746 DEFAULT CHARSET=utf8;

/*Data for the table `domain` */

insert  into `domain`(`id`,`host`,`type`,`view`,`route`,`val`,`mx`,`ttl`,`is_edit`,`is_on`,`desc`,`up_time`) values (1,'@','NS',0,0,'vip1.eflydns.net',0,10,0,1,'这是VIP域名NS记录，不能修改！','2014-07-22 15:15:23'),(2,'@','NS',0,0,'vip2.eflydns.net',0,10,0,1,'这是VIP域名NS记录，不能修改！','2014-06-24 14:15:29'),(3,'@','NS',0,0,'ns1.eflydns.net',0,10,0,1,'这是免费用户专用NS记录，不能修改！','2014-06-24 14:15:29'),(4,'@','NS',0,0,'ns2.eflydns.net',0,10,0,1,'这是免费用户专用NS记录，不能修改！','2014-06-24 14:15:29'),(574,'@','NS',0,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-12 10:34:37'),(575,'@','NS',0,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-12 10:34:37'),(576,'@','NS',0,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-12 10:43:39'),(577,'@','NS',0,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-12 10:43:39'),(578,'www','A',1,0,'1.2.3.4',0,10,1,1,'','2014-08-12 10:45:54'),(579,'aaa','A',1,0,'3.4.5.6',0,10,1,1,'','2014-08-12 11:56:22'),(581,'*','A',1,0,'7.7.7.7',0,10,1,1,'','2014-08-12 14:47:18'),(585,'sss','A',1,0,'6.7.8.9',0,10,1,1,'','2014-08-12 15:35:02'),(615,'@','NS',0,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-19 09:44:18'),(616,'@','NS',0,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-19 09:44:18'),(617,'www','A',1,0,'202.96.128.166',0,10,1,1,'','2014-08-19 09:44:42'),(618,'@','NS',0,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-19 09:46:04'),(619,'@','NS',0,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-19 09:46:04'),(620,'www','A',1,0,'121.201.12.61',0,10,1,1,'','2014-08-19 09:46:25'),(633,'@','NS',0,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-19 10:51:11'),(634,'@','NS',0,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-19 10:51:11'),(635,'www','A',1,0,'12.12.1.2',0,10,1,1,'','2014-08-19 10:51:25'),(637,'baidu','A',1,0,'202.96.128.86',0,10,1,1,'','2014-08-19 15:16:55'),(638,'google','A',1,0,'202.96.128.187',0,10,1,1,'','2014-08-20 16:56:50'),(639,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-21 16:06:28'),(640,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-21 16:06:28'),(641,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-21 16:10:59'),(642,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-21 16:10:59'),(643,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-21 16:29:27'),(644,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-21 16:29:27'),(645,'www','A',1,0,'21.35.214.1',0,10,1,1,'测试备注用的','2014-08-21 16:29:48'),(654,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-27 14:26:30'),(655,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-27 14:26:30'),(656,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-27 14:59:46'),(657,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-27 14:59:46'),(658,'www','A',1,0,'121.201.12.66',0,10,1,1,'','2014-08-27 15:03:19'),(661,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-27 15:51:19'),(662,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-27 15:51:19'),(663,'kks','CNAME',1,0,'www.gdzjwl.net',0,10,1,1,'','2014-08-27 16:57:46'),(666,'*','A',1,0,'121.201.12.61',0,10,1,1,'123','2014-08-28 09:11:13'),(675,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-29 14:47:21'),(676,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-29 14:47:21'),(677,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-08-29 14:48:26'),(678,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-08-29 14:48:26'),(679,'aaa','A',1,0,'1.2.34.4',0,10,1,1,'','2014-08-29 14:59:32'),(680,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-01 09:43:15'),(681,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-01 09:43:15'),(682,'@','A',2,0,'10.10.10.10',0,10,1,1,'','2014-09-01 10:24:20'),(683,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-03 10:57:42'),(684,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-03 10:57:42'),(685,'www','A',1,0,'127.0.0.1',0,10,1,1,'','2014-09-03 10:58:37'),(686,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-03 10:59:10'),(687,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-03 10:59:10'),(688,'www','A',1,0,'1.1.1.1',0,10,1,1,'','2014-09-03 11:00:27'),(689,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-03 11:08:50'),(690,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-03 11:08:50'),(692,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-03 17:16:10'),(693,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-03 17:16:10'),(694,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-04 15:11:08'),(695,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-04 15:11:08'),(696,'eee','A',1,0,'5.5.5.5',0,10,1,1,'','2014-09-05 09:35:05'),(697,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-05 10:11:51'),(698,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-05 10:11:51'),(699,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-05 11:07:10'),(700,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-05 11:07:10'),(701,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-05 16:36:06'),(702,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-05 16:36:06'),(703,'www','A',1,0,'119.145.147.22',0,10,1,1,'','2014-09-05 16:55:32'),(708,'ddd','A',1,0,'4.4.4.4',0,10,1,1,'','2014-09-10 11:30:29'),(709,'ccc','A',1,0,'6.6.6.6',0,10,1,1,'','2014-09-10 14:20:13'),(710,'www.efly.cc','A',1,0,'115.238.154.90',0,10,1,1,'','2014-09-10 16:43:08'),(711,'www.efly.cc','A',1,0,'115.238.154.8',0,10,1,1,'','2014-09-10 16:44:36'),(712,'www.efly.cc','A',1,0,'115.238.154.9',0,10,1,1,'','2014-09-10 16:44:53'),(713,'www.efly.cc','A',1,0,'115.238.154.10',0,10,1,1,'','2014-09-10 16:45:04'),(714,'www.eflypro.com','A',1,0,'121.9.13.185',0,10,1,1,'','2014-09-10 17:01:22'),(715,'www.eflypro.com','A',2,0,'121.9.13.185',0,10,1,1,'','2014-09-11 09:24:00'),(716,'www.eflypro.com','A',3,0,'121.9.13.185',0,10,1,1,'','2014-09-11 09:24:11'),(717,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-11 10:03:22'),(718,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-11 10:03:22'),(719,'www','A',1,0,'11.11.11.11',0,10,1,1,'','2014-09-11 10:03:40'),(720,'www','A',1,0,'11.11.11.12',0,10,1,1,'','2014-09-11 10:05:21'),(721,'www.eflypro.com','A',4,0,'121.9.13.185',0,10,1,1,'','2014-09-11 10:12:46'),(722,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-11 11:55:05'),(723,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-11 11:55:05'),(724,'likunxiang','CNAME',1,0,'google.lkx.org',0,10,1,1,'','2014-09-11 15:35:43'),(725,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-12 10:46:34'),(726,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-12 10:46:34'),(727,'www','A',1,0,'121.9.13.185',0,10,1,1,'','2014-09-12 10:47:11'),(728,'www','A',2,0,'121.9.13.185',0,10,1,1,'','2014-09-12 10:47:19'),(729,'www','A',3,0,'121.9.13.185',0,10,1,1,'','2014-09-12 10:47:26'),(732,'skynet','A',1,0,'121.201.11.8',0,10,1,1,'','2014-09-12 11:09:53'),(733,'skynet','A',2,0,'121.201.11.8',0,10,1,1,'','2014-09-12 11:10:02'),(734,'skynet','A',3,0,'121.201.11.8',0,10,1,1,'','2014-09-12 11:10:09'),(742,'@','NS',1,0,'ns1.eflydns.net',0,10,0,1,'','2014-09-12 16:50:15'),(743,'@','NS',1,0,'ns2.eflydns.net',0,10,0,1,'','2014-09-12 16:50:15'),(744,'@','A',1,0,'121.201.12.61',0,10,1,1,'','2014-09-15 11:43:47'),(745,'@','A',1,0,'121.9.13.185',0,10,1,1,'','2014-09-16 12:38:52');

/*Table structure for table `level` */

DROP TABLE IF EXISTS `level`;

CREATE TABLE `level` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `level_id` int(4) NOT NULL COMMENT '套餐编号',
  `level_name` varchar(32) NOT NULL COMMENT '套餐名称',
  `mnt_rate` int(4) NOT NULL COMMENT '域名监控频率',
  `mnt_task` int(4) NOT NULL COMMENT '域名监控任务数',
  `mnt_node` int(4) NOT NULL COMMENT '域名监控节点数',
  `mnt_bak_ip` int(4) NOT NULL COMMENT '域名监控备用IP数',
  `mnt_alarm` int(4) NOT NULL COMMENT '域名监控,宕机报警共享',
  `sms_num` int(8) NOT NULL COMMENT '赠送短信数',
  `ttl_min` int(4) NOT NULL COMMENT 'ttl最小值',
  `load_balance` int(4) NOT NULL COMMENT 'A记录负载均衡节点数',
  `child_steps` int(4) NOT NULL COMMENT '子域名级别',
  `show_url` int(4) NOT NULL COMMENT '显性URL转发条数',
  `hide_url` int(4) NOT NULL COMMENT '隐性URL转发条数',
  `spread_rcd` int(4) NOT NULL COMMENT '泛解析个数',
  `asalias_num` int(4) NOT NULL COMMENT '别名个数',
  `share_num` int(4) NOT NULL COMMENT '域名共享用户数',
  `child_share_num` int(4) NOT NULL COMMENT '子域名共享用户数',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `level` */

/*Table structure for table `nameserver` */

DROP TABLE IF EXISTS `nameserver`;

CREATE TABLE `nameserver` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `level` int(4) NOT NULL COMMENT '服务级别',
  `val` varchar(128) NOT NULL COMMENT '记录值',
  `view` int(4) NOT NULL COMMENT '默认视图',
  `ttl` int(4) NOT NULL COMMENT 'ttl值，分钟',
  `up_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '最近修改时间，防止频繁修改操作',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=639 DEFAULT CHARSET=utf8;

/*Data for the table `nameserver` */

insert  into `nameserver`(`id`,`level`,`val`,`view`,`ttl`,`up_time`) values (1,1,'vip1.eflydns.net',1,10,'2014-07-22 15:15:23'),(2,1,'vip2.eflydns.net',1,10,'2014-06-24 14:15:29'),(3,0,'ns1.eflydns.net',1,10,'2014-06-24 14:15:29'),(4,0,'ns2.eflydns.net',1,10,'2014-06-24 14:15:29');

/*Table structure for table `route` */

DROP TABLE IF EXISTS `route`;

CREATE TABLE `route` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `name` varchar(128) NOT NULL,
  `up_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '更新时间',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=38 DEFAULT CHARSET=utf8;

/*Data for the table `route` */

/*Table structure for table `route_mask` */

DROP TABLE IF EXISTS `route_mask`;

CREATE TABLE `route_mask` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `route_id` int(16) NOT NULL,
  `mask_start` int(16) NOT NULL,
  `mask_end` int(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=51 DEFAULT CHARSET=utf8;

/*Data for the table `route_mask` */

/*Table structure for table `view` */

DROP TABLE IF EXISTS `view`;

CREATE TABLE `view` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `level` int(4) NOT NULL DEFAULT '0' COMMENT '免费用户0，创业版1，企业版2，旗舰版3',
  `up_time` varchar(32) NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8;

/*Data for the table `view` */

insert  into `view`(`id`,`name`,`level`,`up_time`) values (1,'默认',0,'2014-05-05 12:21:14'),(2,'电信',0,'2014-05-05 12:21:14'),(3,'联通',0,'2014-05-05 12:21:14'),(4,'移动',0,'2014-05-05 12:21:14'),(5,'教育网',0,'2014-05-05 12:21:14'),(6,'长城宽带',0,'2014-08-12 11:32:45');

/*Table structure for table `view_mask` */

DROP TABLE IF EXISTS `view_mask`;

CREATE TABLE `view_mask` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `view_id` int(8) NOT NULL,
  `mask_start` int(16) NOT NULL,
  `mask_end` int(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `view_mask` */

/*Table structure for table `zone` */

DROP TABLE IF EXISTS `zone`;

CREATE TABLE `zone` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `domain` varchar(128) NOT NULL,
  `level` int(4) NOT NULL DEFAULT '0' COMMENT '例如：免费版0，个人专业版1, 企业创业版8，企业标准版9，企业旗舰版10',
  `client_id` int(16) NOT NULL,
  `group_id` int(8) NOT NULL DEFAULT '0' COMMENT '所属分组',
  `default_ttl` int(8) NOT NULL DEFAULT '60' COMMENT 'ttl默认值',
  `is_author` int(4) NOT NULL DEFAULT '0' COMMENT '是否修改dns，未修改0/已修改1',
  `is_on` int(4) NOT NULL DEFAULT '1' COMMENT '启用1/暂停0',
  `is_star` int(4) NOT NULL DEFAULT '0' COMMENT '加星1/未加0',
  `is_lock` int(4) NOT NULL DEFAULT '0' COMMENT '锁定1/未锁定0',
  `desc` varchar(512) NOT NULL,
  `add_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '域名添加时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=342 DEFAULT CHARSET=utf8;

/*Data for the table `zone` */

insert  into `zone`(`id`,`domain`,`level`,`client_id`,`group_id`,`default_ttl`,`is_author`,`is_on`,`is_star`,`is_lock`,`desc`,`add_time`) values (295,'hyb.com',0,10,0,60,0,1,0,0,'','2014-08-12 10:43:39'),(307,'lkx.org',0,12,0,60,0,1,0,0,'','2014-08-19 09:44:18'),(308,'eflydns.com',0,12,0,60,1,1,0,0,'网站解析地址','2014-08-19 09:46:04'),(312,'abc.com',0,12,0,60,0,1,0,0,'测试域名abc，在哪里申请，主要用来做图片','2014-08-19 10:51:11'),(315,'dnspro.cn',0,13,0,60,0,1,0,0,'','2014-08-21 16:29:27'),(319,'daoba.com',0,19,0,60,0,1,0,0,'','2014-08-27 14:26:30'),(320,'gdzjwl.net',8,12,0,60,1,1,1,0,'123','2014-08-27 14:59:46'),(321,'testing.com',0,20,0,60,0,1,0,0,'','2014-08-27 15:51:19'),(322,'baidu.com',0,13,0,60,0,1,0,0,'','2014-08-29 14:47:21'),(323,'lkx.com',0,12,0,60,0,1,0,0,'','2014-08-29 14:48:26'),(324,'www.efly.cc',0,13,0,60,0,1,0,0,'这是公司网站域名','2014-09-01 09:43:15'),(325,'o-ran.com',0,21,0,60,0,1,0,0,'','2014-09-03 10:57:42'),(326,'he.com',0,22,0,60,0,1,0,0,'','2014-09-03 10:59:10'),(327,'tifa.com',0,24,0,60,0,1,0,0,'','2014-09-03 11:08:50'),(328,'lkx.cn',0,12,14,60,0,1,0,0,'','2014-09-03 17:16:10'),(329,'www.eflypro.com',0,26,0,60,0,1,0,0,'','2014-09-04 15:11:08'),(330,'www.chancheng.gov.cn',0,36,0,60,0,1,0,0,'','2014-09-05 10:11:51'),(331,'www.mi.com',0,37,0,60,0,1,0,0,'','2014-09-05 11:07:10'),(332,'efly.d2okkk.net',0,38,0,60,0,1,0,0,'','2014-09-05 16:36:06'),(333,'xiaotiandii.com',0,40,0,60,0,0,0,0,'','2014-09-06 16:12:59'),(334,'yhb111.com',0,41,0,60,0,0,0,0,'','2014-09-09 10:39:46'),(335,'www.lw.com',0,28,13,60,0,1,0,0,'','2014-09-11 10:03:22'),(336,'www.jv.com',0,28,0,60,0,1,0,0,'','2014-09-11 11:55:05'),(337,'eflypro.com',0,12,0,60,1,1,1,0,'','2014-09-12 10:46:34'),(341,'www.jc.com',0,28,0,60,0,1,0,0,'','2014-09-12 16:50:15');

/*Table structure for table `zone_level` */

DROP TABLE IF EXISTS `zone_level`;

CREATE TABLE `zone_level` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `start_time` varchar(32) NOT NULL,
  `end_time` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=utf8;

/*Data for the table `zone_level` */

/*Table structure for table `zone_name` */

DROP TABLE IF EXISTS `zone_name`;

CREATE TABLE `zone_name` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `name` varchar(128) NOT NULL COMMENT '别名',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=51 DEFAULT CHARSET=utf8;

/*Data for the table `zone_name` */

insert  into `zone_name`(`id`,`zone_id`,`name`) values (49,324,'efly.cc'),(50,328,'lkxtest.cn');

/*Table structure for table `zone_whois` */

DROP TABLE IF EXISTS `zone_whois`;

CREATE TABLE `zone_whois` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `expiration_time` varchar(32) NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT '域名到期时间',
  `update_time` varchar(32) NOT NULL DEFAULT '0000-00-00 00:00:00' COMMENT 'Last update',
  PRIMARY KEY (`id`),
  UNIQUE KEY `zone_id` (`zone_id`)
) ENGINE=MyISAM AUTO_INCREMENT=339 DEFAULT CHARSET=utf8;

/*Data for the table `zone_whois` */

insert  into `zone_whois`(`id`,`zone_id`,`expiration_time`,`update_time`) values (1,307,'2015-06-12 19:16:32','2014-09-17 21:42:11'),(2,308,'2015-07-22 00:00:00','2014-09-17 21:42:22'),(3,312,'2015-05-23 00:00:00','2014-09-17 21:42:24'),(312,315,'2017-07-05 22:17:43','2014-09-12 15:35:14'),(321,324,'2016-02-17 08:31:15','2014-09-12 15:35:18'),(320,323,'2015-03-28 00:00:00','2014-09-17 21:42:27'),(319,322,'2015-10-11 00:00:00','2014-09-12 15:35:16'),(316,319,'0000-00-00 00:00:00','0000-00-00 00:00:00'),(317,320,'2015-09-27 00:00:00','2014-09-17 21:42:24'),(318,321,'0000-00-00 00:00:00','0000-00-00 00:00:00'),(322,325,'2015-08-29 00:00:00','2014-09-18 10:36:54'),(323,326,'0000-00-00 00:00:00','0000-00-00 00:00:00'),(324,327,'0000-00-00 00:00:00','0000-00-00 00:00:00'),(325,328,'2018-02-18 04:30:19','2014-09-17 21:42:27'),(326,329,' 00:00:00','2014-09-05 09:49:57'),(327,330,'0000-00-00 00:00:00','0000-00-00 00:00:00'),(328,331,'0000-00-00 00:00:00','0000-00-00 00:00:00'),(329,332,' 00:00:00','2014-09-08 19:27:21'),(332,335,'2023-10-24 00:00:00','2014-09-18 09:05:29'),(333,336,'2021-11-23 00:00:00','2014-09-18 09:05:30'),(334,337,'2015-08-06 00:00:00','2014-09-17 21:42:28'),(338,341,'2014-11-20 00:00:00','2014-09-18 09:05:31');

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

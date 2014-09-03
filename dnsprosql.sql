/*
SQLyog Community v11.28 (32 bit)
MySQL - 5.1.73 : Database - dnspro_core
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`dnspro_core` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `dnspro_core`;

/*Table structure for table `a_record` */

DROP TABLE IF EXISTS `a_record`;

CREATE TABLE `a_record` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `zone` int(32) NOT NULL,
  `name` varchar(256) NOT NULL,
  `ttl` int(32) NOT NULL,
  `ipaddr` varchar(16) NOT NULL,
  `description` varchar(256) NOT NULL,
  `state` int(11) NOT NULL,
  `enable` int(11) NOT NULL,
  `rid` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `one_a_record` (`rid`)
) ENGINE=MyISAM AUTO_INCREMENT=54 DEFAULT CHARSET=utf8;

/*Data for the table `a_record` */

insert  into `a_record`(`id`,`zone`,`name`,`ttl`,`ipaddr`,`description`,`state`,`enable`,`rid`) values (3,2,'www.hyb.com',600,'1.2.3.4','',0,1,578),(4,2,'aaa.hyb.com',600,'3.4.5.6','',0,1,579),(8,2,'sss.hyb.com',600,'6.7.8.9','',0,1,585),(6,2,'*.hyb.com',600,'7.7.7.7','',0,1,581),(19,16,'www.eflydns.com',600,'121.201.12.61','',0,1,620),(30,15,'likunxiang.lkx.org',600,'8.8.8.8','',0,1,636),(35,26,'www.abc.com',600,'12.12.1.2','',0,1,635),(18,15,'www.lkx.org',600,'202.96.128.166','',0,1,617),(33,27,'www.dnspro.cn',600,'21.35.214.1','',0,1,645),(31,15,'baidu.lkx.org',600,'202.96.128.86','',0,1,637),(32,15,'google.lkx.org',600,'202.96.128.186','',0,1,638),(37,29,'www.gdzjwl.net',600,'121.9.212.124','',0,1,658),(38,29,'*.gdzjwl.net',600,'121.9.212.124','',0,0,659),(49,30,'aaa.gdzjwl.net',600,'1.2.34.4','',0,1,679),(40,16,'eflydns.com',600,'121.201.12.61','',0,1,665),(41,16,'*.eflydns.com',600,'121.201.12.61','',0,1,666),(47,29,'aaa.gdzjwl.net',600,'121.9.212.124','',0,1,673),(48,29,'ooo.gdzjwl.net',600,'121.9.212.124','',0,1,674),(50,31,'dnspro.cn',600,'10.10.10.10','',0,1,682),(51,32,'www.o-ran.com',600,'127.0.0.1','',0,1,685),(52,33,'www.he.com',600,'1.1.1.1','',0,1,688);

/*Table structure for table `aaaa_record` */

DROP TABLE IF EXISTS `aaaa_record`;

CREATE TABLE `aaaa_record` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `zone` int(32) NOT NULL,
  `name` varchar(256) NOT NULL,
  `ttl` int(32) NOT NULL,
  `ipaddr` varchar(32) NOT NULL,
  `description` varchar(256) NOT NULL,
  `state` int(11) NOT NULL,
  `enable` int(11) NOT NULL,
  `rid` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `one_aaaa_record` (`rid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `aaaa_record` */

/*Table structure for table `cname_record` */

DROP TABLE IF EXISTS `cname_record`;

CREATE TABLE `cname_record` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `zone` int(32) NOT NULL,
  `name` varchar(256) NOT NULL,
  `ttl` int(32) NOT NULL,
  `cname` varchar(256) NOT NULL,
  `description` varchar(256) NOT NULL,
  `state` int(11) NOT NULL,
  `enable` int(11) NOT NULL,
  `rid` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `one_cname_record` (`rid`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

/*Data for the table `cname_record` */

insert  into `cname_record`(`id`,`zone`,`name`,`ttl`,`cname`,`description`,`state`,`enable`,`rid`) values (2,29,'kks.gdzjwl.net',600,'www.gdzjwl.net','',0,1,663),(3,29,'oo.gdzjwl.net',600,'www.gdzjwl.net','',0,1,664),(4,29,'sss.gdzjwl.net',600,'www.gdzjwl.net','',0,1,668);

/*Table structure for table `domain_ns` */

DROP TABLE IF EXISTS `domain_ns`;

CREATE TABLE `domain_ns` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `domain` varchar(100) NOT NULL,
  `ttl` int(32) NOT NULL,
  `server` varchar(100) NOT NULL,
  `description` varchar(256) NOT NULL DEFAULT 'null',
  `rid` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `one_dom_ns` (`rid`)
) ENGINE=MyISAM AUTO_INCREMENT=73 DEFAULT CHARSET=utf8;

/*Data for the table `domain_ns` */

insert  into `domain_ns`(`id`,`domain`,`ttl`,`server`,`description`,`rid`) values (8,'hyb.com',600,'ns2.eflydns.net','null',577),(7,'fdsa.cn',600,'ns2.eflydns.net','null',575),(6,'hyb.com',600,'ns1.eflydns.net','null',576),(5,'fdsa.cn',600,'ns1.eflydns.net','null',574),(42,'abc.com',600,'ns1.eflydns.net','null',633),(34,'eflydns.com',600,'ns1.eflydns.net','null',618),(46,'dnspro.cn',600,'ns1.eflydns.net','null',643),(45,'dnspro.cn',600,'ns2.eflydns.net','null',644),(33,'eflydns.com',600,'ns2.eflydns.net','null',619),(32,'lkx.org',600,'ns1.eflydns.net','null',615),(31,'lkx.org',600,'ns2.eflydns.net','null',616),(41,'abc.com',600,'ns2.eflydns.net','null',634),(43,'None',600,'ns2.eflydns.net','null',639),(54,'daoba.com',600,'ns2.eflydns.net','null',655),(53,'daoba.com',600,'ns1.eflydns.net','null',654),(55,'gdzjwl.net',600,'ns1.eflydns.net','null',656),(56,'gdzjwl.net',600,'ns2.eflydns.net','null',657),(57,'testing.com',600,'ns2.eflydns.net','null',662),(58,'testing.com',600,'ns1.eflydns.net','null',661),(59,'baidu.com',600,'ns1.eflydns.net','null',675),(60,'baidu.com',600,'ns2.eflydns.net','null',676),(61,'lkx.com',600,'ns1.eflydns.net','null',677),(62,'lkx.com',600,'ns2.eflydns.net','null',678),(63,'www.efly.cc',600,'ns1.eflydns.net','null',680),(64,'www.efly.cc',600,'ns2.eflydns.net','null',681),(65,'o-ran.com',600,'ns1.eflydns.net','null',683),(66,'o-ran.com',600,'ns2.eflydns.net','null',684),(67,'he.com',600,'ns2.eflydns.net','null',687),(68,'he.com',600,'ns1.eflydns.net','null',686),(69,'tifa.com',600,'ns1.eflydns.net','null',689),(70,'tifa.com',600,'ns2.eflydns.net','null',690),(71,'lkx.cn',600,'ns1.eflydns.net','null',692),(72,'lkx.cn',600,'ns2.eflydns.net','null',693);

/*Table structure for table `domain_zone` */

DROP TABLE IF EXISTS `domain_zone`;

CREATE TABLE `domain_zone` (
  `domain` varchar(256) NOT NULL,
  `zone` varchar(256) NOT NULL,
  PRIMARY KEY (`domain`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `domain_zone` */

insert  into `domain_zone`(`domain`,`zone`) values ('www.hyb.com','hyb.com'),('aaa.hyb.com','hyb.com'),('ttt.hyb.com','hyb.com'),('*.hyb.com','hyb.com'),('www.eflydns.com','eflydns.com'),('sss.hyb.com','hyb.com'),('www.hhyb.com','hhyb.com'),('www.lkx.cn','lkx.cn'),('www.lkx.org','lkx.org'),('likunxiang.lkx.org','lkx.org'),('ew.mysq.cn','mysq.cn'),('www.abc.com','abc.com'),('baidu.com.lkx.org','lkx.org'),('baidu.lkx.org','lkx.org'),('google.lkx.org','lkx.org'),('www.dnspro.cn','dnspro.cn'),('test.com','test.com'),('abc.com','abc.com'),('www.gdzjwl.net','gdzjwl.net'),('*.gdzjwl.net','gdzjwl.net'),('kkk.gdzjwl.net','gdzjwl.net'),('gdzjwl.net','gdzjwl.net'),('oo.gdzjwl.net','gdzjwl.net'),('eflydns.com','eflydns.com'),('*.eflydns.com','eflydns.com'),('kks.gdzjwl.net','gdzjwl.net'),('ss.gdzjwl.net','gdzjwl.net'),('sss.gdzjwl.net','gdzjwl.net'),('mm.gdzjwl.net','gdzjwl.net'),('cc.gdzjwl.net','gdzjwl.net'),('aaa.gdzjwl.net','gdzjwl.net'),('xxx.gdzjwl.net','gdzjwl.net'),('ooo.gdzjwl.net','gdzjwl.net'),('dnspro.cn','dnspro.cn'),('www.o-ran.com','o-ran.com'),('www.he.com','he.com'),('www.tifa.com','tifa.com');

/*Table structure for table `mx_record` */

DROP TABLE IF EXISTS `mx_record`;

CREATE TABLE `mx_record` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `zone` int(32) NOT NULL,
  `name` varchar(256) NOT NULL,
  `ttl` int(32) NOT NULL,
  `level` int(16) NOT NULL,
  `server` varchar(256) NOT NULL,
  `description` varchar(256) NOT NULL,
  `state` int(11) NOT NULL,
  `enable` int(11) NOT NULL,
  `rid` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `one_mx_record` (`rid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `mx_record` */

/*Table structure for table `ns_record` */

DROP TABLE IF EXISTS `ns_record`;

CREATE TABLE `ns_record` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `zone` int(32) NOT NULL,
  `name` varchar(256) NOT NULL,
  `ttl` int(32) NOT NULL,
  `server` varchar(256) NOT NULL,
  `description` varchar(256) NOT NULL,
  `state` int(11) NOT NULL,
  `enable` int(11) NOT NULL,
  `rid` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `one_ns_record` (`rid`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

/*Data for the table `ns_record` */

insert  into `ns_record`(`id`,`zone`,`name`,`ttl`,`server`,`description`,`state`,`enable`,`rid`) values (4,29,'gdzjwl.net',600,'ns1.eflydns.net','',0,0,656),(3,29,'gdzjwl.net',600,'ns2.eflydns.net','',0,0,657);

/*Table structure for table `snd_record` */

DROP TABLE IF EXISTS `snd_record`;

CREATE TABLE `snd_record` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `class` varchar(255) NOT NULL,
  `type` int(11) NOT NULL,
  `viewid` int(11) NOT NULL,
  `data` varchar(255) NOT NULL,
  `state` int(11) NOT NULL,
  `chktime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `opt` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=345 DEFAULT CHARSET=utf8;

/*Data for the table `snd_record` */

insert  into `snd_record`(`id`,`class`,`type`,`viewid`,`data`,`state`,`chktime`,`opt`) values (312,'dns',1,1,'www.hyb.com',1,'2014-09-03 10:45:38',1),(313,'dns',1,1,'aaa.hyb.com',1,'2014-09-03 10:45:38',1),(314,'dns',1,1,'sss.hyb.com',1,'2014-09-03 10:45:38',1),(315,'dns',1,1,'*.hyb.com',1,'2014-09-03 10:45:38',1),(316,'dns',1,1,'www.eflydns.com',1,'2014-09-03 10:45:38',1),(317,'dns',1,1,'likunxiang.lkx.org',1,'2014-09-03 10:45:38',1),(318,'dns',1,1,'www.abc.com',1,'2014-09-03 10:45:38',1),(319,'dns',1,1,'www.lkx.org',1,'2014-09-03 10:45:38',1),(320,'dns',1,1,'www.dnspro.cn',1,'2014-09-03 10:45:38',1),(321,'dns',1,1,'baidu.lkx.org',1,'2014-09-03 10:45:38',1),(323,'dns',1,1,'www.gdzjwl.net',1,'2014-09-03 10:45:38',1),(324,'dns',1,2,'aaa.gdzjwl.net',1,'2014-09-03 10:45:38',1),(325,'dns',1,1,'eflydns.com',1,'2014-09-03 10:45:38',1),(326,'dns',1,1,'*.eflydns.com',1,'2014-09-03 10:45:38',1),(327,'dns',1,1,'aaa.gdzjwl.net',1,'2014-09-03 10:45:38',1),(328,'dns',1,1,'ooo.gdzjwl.net',1,'2014-09-03 10:45:38',1),(329,'dns',1,2,'dnspro.cn',1,'2014-09-03 10:45:38',1),(330,'dns',5,1,'kks.gdzjwl.net',1,'2014-09-03 10:45:38',1),(331,'dns',5,1,'oo.gdzjwl.net',1,'2014-09-03 10:45:38',1),(332,'dns',5,1,'sss.gdzjwl.net',1,'2014-09-03 10:45:38',1),(344,'dns',1,1,'google.lkx.org',1,'2014-09-03 17:20:49',1),(333,'dns',1,1,'www.o-ran.com',1,'2014-09-03 10:58:40',1),(334,'dns',1,1,'www.he.com',1,'2014-09-03 11:00:31',1),(341,'dns',1,1,'www.tifa.com',1,'2014-09-03 11:20:04',1),(343,'dns',1,1,'google.lkx.org',3,'2014-09-03 17:20:49',2),(342,'dns',1,1,'www.tifa.com',1,'2014-09-03 11:20:40',2);

/*Table structure for table `txt_record` */

DROP TABLE IF EXISTS `txt_record`;

CREATE TABLE `txt_record` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `zone` int(32) NOT NULL,
  `name` varchar(256) NOT NULL,
  `ttl` int(32) NOT NULL,
  `txt` text NOT NULL,
  `description` varchar(256) NOT NULL,
  `state` int(11) NOT NULL,
  `enable` int(11) NOT NULL,
  `rid` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `one_txt_record` (`rid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `txt_record` */

/*Table structure for table `view_index` */

DROP TABLE IF EXISTS `view_index`;

CREATE TABLE `view_index` (
  `index` int(11) NOT NULL AUTO_INCREMENT,
  `id` smallint(4) NOT NULL,
  `view_name` varchar(32) NOT NULL,
  `status` varchar(10) NOT NULL,
  `comment` varchar(255) NOT NULL,
  `ttl` int(11) NOT NULL DEFAULT '300',
  PRIMARY KEY (`index`),
  UNIQUE KEY `one_line` (`view_name`,`id`)
) ENGINE=MyISAM AUTO_INCREMENT=65544 DEFAULT CHARSET=utf8;

/*Data for the table `view_index` */

insert  into `view_index`(`index`,`id`,`view_name`,`status`,`comment`,`ttl`) values (1,2,'电信视图','true','广东电信',300),(2,3,'联通视图','true','',300),(3,4,'广东铁通视图','true','',300),(4,5,'睿江办公网视图','true','',300),(5,6,'中山广电视图','true','',300),(6,1,'默认视图','true','不能删除',300);

/*Table structure for table `view_mask` */

DROP TABLE IF EXISTS `view_mask`;

CREATE TABLE `view_mask` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `network` varchar(20) NOT NULL,
  `viewid` smallint(11) unsigned NOT NULL,
  `status` varchar(10) NOT NULL DEFAULT 'true',
  PRIMARY KEY (`id`),
  UNIQUE KEY `network` (`network`)
) ENGINE=MyISAM AUTO_INCREMENT=648 DEFAULT CHARSET=utf8;

/*Data for the table `view_mask` */

/*Table structure for table `zone` */

DROP TABLE IF EXISTS `zone`;

CREATE TABLE `zone` (
  `id` int(32) NOT NULL AUTO_INCREMENT,
  `client_id` int(11) NOT NULL,
  `domain` varchar(256) NOT NULL,
  `view` int(16) NOT NULL,
  `update_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `description` varchar(256) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `domain` (`domain`,`view`)
) ENGINE=MyISAM AUTO_INCREMENT=35 DEFAULT CHARSET=utf8;

/*Data for the table `zone` */

insert  into `zone`(`id`,`client_id`,`domain`,`view`,`update_time`,`description`) values (2,1,'hyb.com',1,'2014-08-12 15:35:04',''),(3,1,'hyb.com',2,'2014-08-12 15:34:46',''),(15,1,'lkx.org',1,'2014-09-03 17:20:49',''),(16,1,'eflydns.com',1,'2014-08-28 09:11:16',''),(29,1,'gdzjwl.net',1,'2014-08-29 14:59:19',''),(27,1,'dnspro.cn',1,'2014-08-21 16:29:51',''),(26,1,'abc.com',1,'2014-08-25 15:30:56',''),(30,1,'gdzjwl.net',2,'2014-08-29 14:59:35',''),(31,1,'dnspro.cn',2,'2014-09-01 10:24:23',''),(32,1,'o-ran.com',1,'2014-09-03 10:58:40',''),(33,1,'he.com',1,'2014-09-03 11:00:31',''),(34,1,'tifa.com',1,'2014-09-03 11:20:40','');

/* Trigger structure for table `a_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `insert_a_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `insert_a_record` AFTER INSERT ON `a_record` FOR EACH ROW BEGIN
call update_domain_zone(new.name,new.zone);

call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `a_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `monthly_a_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `monthly_a_record` AFTER UPDATE ON `a_record` FOR EACH ROW BEGIN

IF NEW.name != OLD.name THEN       
call update_domain_zone(new.name,new.zone);
END IF;

call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `aaaa_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `insert_aaaa_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `insert_aaaa_record` AFTER INSERT ON `aaaa_record` FOR EACH ROW BEGIN

call update_domain_zone(new.name,new.zone);
call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `aaaa_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `monthly_aaaa_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `monthly_aaaa_record` AFTER UPDATE ON `aaaa_record` FOR EACH ROW BEGIN

IF NEW.name != OLD.name THEN       
call update_domain_zone(new.name,new.zone);
END IF;

call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `cname_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `insert_cname_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `insert_cname_record` AFTER INSERT ON `cname_record` FOR EACH ROW BEGIN

call update_domain_zone(new.name,new.zone);
call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `cname_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `monthly_cname_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `monthly_cname_record` AFTER UPDATE ON `cname_record` FOR EACH ROW BEGIN

IF NEW.name != OLD.name THEN       
call update_domain_zone(new.name,new.zone);
END IF;

call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `mx_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `insert_mx_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `insert_mx_record` AFTER INSERT ON `mx_record` FOR EACH ROW BEGIN

call update_domain_zone(new.name,new.zone);
call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `mx_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `monthly_mx_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `monthly_mx_record` AFTER UPDATE ON `mx_record` FOR EACH ROW BEGIN

IF NEW.name != OLD.name THEN       
call update_domain_zone(new.name,new.zone);
END IF;

call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `ns_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `insert_ns_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `insert_ns_record` AFTER INSERT ON `ns_record` FOR EACH ROW BEGIN

call update_domain_zone(new.name,new.zone);
call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `ns_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `monthly_ns_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `monthly_ns_record` AFTER UPDATE ON `ns_record` FOR EACH ROW BEGIN

IF NEW.name != OLD.name THEN       
call update_domain_zone(new.name,new.zone);
END IF;

call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `txt_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `insert_txt_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `insert_txt_record` AFTER INSERT ON `txt_record` FOR EACH ROW BEGIN

call update_domain_zone(new.name,new.zone);
call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Trigger structure for table `txt_record` */

DELIMITER $$

/*!50003 DROP TRIGGER*//*!50032 IF EXISTS */ /*!50003 `monthly_txt_record` */$$

/*!50003 CREATE */ /*!50017 DEFINER = 'root'@'%' */ /*!50003 TRIGGER `monthly_txt_record` AFTER UPDATE ON `txt_record` FOR EACH ROW BEGIN

IF NEW.name != OLD.name THEN       
call update_domain_zone(new.name,new.zone);
END IF;

call update_zone(new.zone);
	
END */$$


DELIMITER ;

/* Function  structure for function  `tbl_to_dnstype` */

/*!50003 DROP FUNCTION IF EXISTS `tbl_to_dnstype` */;
DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` FUNCTION `tbl_to_dnstype`(tbl_name VARCHAR(255)) RETURNS int(11)
    NO SQL
BEGIN
	IF STRCMP(tbl_name, 'a_record')=0 THEN
		RETURN 1;
	ELSEIF STRCMP(tbl_name, 'aaaa_record')=0 THEN
		RETURN 28;
	ELSEIF STRCMP(tbl_name, 'cname_record')=0 THEN
		RETURN 5;
	ELSEIF STRCMP(tbl_name, 'ns_record')=0 THEN 
		RETURN 2;
	ELSEIF STRCMP(tbl_name, 'txt_record')=0 THEN 
		RETURN 16;
	ELSEIF STRCMP(tbl_name, 'mx_record')=0 THEN
		RETURN 15;
	END IF;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `add_a_line` */

/*!50003 DROP PROCEDURE IF EXISTS  `add_a_line` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `add_a_line`(
	IN n_vid INT, 
	IN str_vname VARCHAR(255),
	in n_state int,
	in str_comm varchar(255))
BEGIN

	DECLARE done INT DEFAULT 0;
	DECLARE onw VARCHAR(255) DEFAULT NULL;
	DECLARE oview INT DEFAULT -1;
	DECLARE cur_mask CURSOR FOR 
		SELECT network, viewid FROM `view_mask` WHERE viewid=n_vid AND `status`='true';
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
	
	OPEN cur_mask;
	REPEAT
		IF NOT done THEN
			FETCH cur_mask INTO onw, oview;
			IF oview!=-1 THEN
				CALL add_snd_req('view', 0, oview, onw, 0, 2);
				SELECT onw, oview;
			END IF;
			SET oview=-1;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cur_mask;
	
	UPDATE `view_mask` SET `status`='false' WHERE viewid=n_view AND `status`='true';
	UPDATE `view_index` SET `status`='false' WHERE `id`=n_view AND `status`='true';
    END */$$
DELIMITER ;

/* Procedure structure for procedure `add_a_record` */

/*!50003 DROP PROCEDURE IF EXISTS  `add_a_record` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `add_a_record`(
	IN `tab_name` VARCHAR(255), 
	IN `str_name` VARCHAR(255), 
	IN `str_main` VARCHAR(255), 
	IN `n_view` INT, 
	IN `n_ttl` INT, 
	IN `str_data` VARCHAR(255), 
	IN `n_state` INT, 
	IN `n_enable` INT, IN `n_rid` INT)
BEGIN
	DECLARE n_zid INT default null;
	declare arr_tbls varchar(255);
	declare cut_tbl varchar(255);
	declare i int;  
	SELECT `id` INTO n_zid FROM zone WHERE domain=str_main AND `view`=n_view LIMIT 1;
	if n_zid is null then
		insert into zone(client_id,domain,`view`,description) values(1,str_main,n_view,'');
		SELECT `id` INTO n_zid FROM zone WHERE domain=str_main AND `view`=n_view LIMIT 1;
	end if;
	set arr_tbls = 'a_record,aaaa_record,cname_record,mx_record,ns_record,txt_record';
	set i=1;  
	while_label: while i <= 6 do 	
		SET @oname = NULL;
		SET @oview = NULL;
		set cut_tbl=concat(substring_index(substring_index(arr_tbls, ',' ,i),',',-1));
		SET @sqlcmd=CONCAT("SELECT ar.name,ze.view into @oname,@oview FROM ",cut_tbl,
			" ar LEFT JOIN zone ze ON ar.zone=ze.id WHERE ar.enable=1 AND ar.rid=",n_rid);
		PREPARE stmt FROM @sqlcmd;
		EXECUTE stmt;
		IF @oname IS NOT NULL AND @oview IS NOT NULL THEN
			if strcmp(cut_tbl, tab_name)!=0 then
				
				SET @sqlcmd = CONCAT("delete from `", cut_tbl, "` WHERE rid=", n_rid);
				PREPARE stmt FROM @sqlcmd;
				EXECUTE stmt;
			end if;
			CALL add_snd_req('dns', tbl_to_dnstype(cut_tbl), @oview, @oname, 0, 2);
			leave while_label;
		end if;
		set i = i + 1;  
	end while;
	
	
	
	
	
				
	SET @sqlcmd = 'error';
	IF STRCMP(tab_name, 'a_record')=0 OR STRCMP(tab_name, 'aaaa_record')=0 THEN
		
		
		
		
		SET @sqlcmd = CONCAT('INSERT INTO ',tab_name,' (zone,`name`,ttl,ipaddr,state,`enable`,`rid`) VALUES(',
				     n_zid,',\'',str_name,'\',',n_ttl,',\'',str_data,'\',',n_state,',',n_enable,',',n_rid,
				     ') ON DUPLICATE KEY UPDATE zone=',n_zid,',`name`=\'',str_name,'\',`ttl`=',n_ttl,
				     ',ipaddr=\'',str_data,'\',state=',n_state,',`enable`=',n_enable);
	ELSEIF STRCMP(tab_name, 'cname_record')=0 THEN
		
		
		SET @sqlcmd = CONCAT('INSERT INTO ',tab_name,' (zone,`name`,ttl,`cname`,state,`enable`,`rid`) VALUES(',
				     n_zid,',\'',str_name,'\',',n_ttl,',\'',str_data,'\',',n_state,',',n_enable,',',n_rid,
				     ') ON DUPLICATE KEY UPDATE zone=',n_zid,',`name`=\'',str_name,'\',`ttl`=',n_ttl,
				     ',`cname`=\'',str_data,'\',state=',n_state,',`enable`=',n_enable);
		
	ELSEIF STRCMP(tab_name, 'ns_record')=0 THEN
		
		
		SET @sqlcmd = CONCAT('INSERT INTO ',tab_name,' (zone,`name`,ttl,`server`,state,`enable`,`rid`) VALUES(',
				     n_zid,',\'',str_name,'\',',n_ttl,',\'',str_data,'\',',n_state,',',n_enable,',',n_rid,
				     ') ON DUPLICATE KEY UPDATE zone=',n_zid,',`name`=\'',str_name,'\',`ttl`=',n_ttl,
				     ',`server`=\'',str_data,'\',state=',n_state,',`enable`=',n_enable);
		
	ELSEIF STRCMP(tab_name, 'txt_record')=0 THEN
		
		
		SET @sqlcmd = CONCAT('INSERT INTO ',tab_name,' (zone,`name`,ttl,`txt`,state,`enable`,`rid`) VALUES(',
				     n_zid,',\'',str_name,'\',',n_ttl,',\'',str_data,'\',',n_state,',',n_enable,',',n_rid,
				     ') ON DUPLICATE KEY UPDATE zone=',n_zid,',`name`=\'',str_name,'\',`ttl`=',n_ttl,
				     ',`txt`=\'',str_data,'\',state=',n_state,',`enable`=',n_enable);
		
	END IF ;
	
	PREPARE stmt FROM @sqlcmd; 
	EXECUTE stmt;  
		
	DEALLOCATE PREPARE stmt; 
	
	CALL add_snd_req('dns', tbl_to_dnstype(tab_name), n_view, str_name, 0, 1);
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `add_a_view_mask` */

/*!50003 DROP PROCEDURE IF EXISTS  `add_a_view_mask` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `add_a_view_mask`(
	IN n_view INT, 
	in str_nw varchar(255))
BEGIN

	DECLARE done INT DEFAULT 0;
	declare onw varchar(255) default null;
	declare oview int default -1;
	declare cur cursor for 
		SELECT network, viewid FROM `view_mask` WHERE network=str_nw AND `status`='true';
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
	
	OPEN cur;
	REPEAT
		IF not done THEN
			FETCH cur INTO onw, oview;
			if oview!=-1 and oview!=n_view then
				CALL add_snd_req('view', 0, oview, onw, 0, 2);
				select onw, oview;
			end if;
			set oview=-1;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cur ;
	
	insert into view_mask (network, viewid, `status`) values (str_nw, n_view, 'true') 
		oN DUPLICATE KEY UPDATE viewid=n_view, `status`='true';
    END */$$
DELIMITER ;

/* Procedure structure for procedure `add_mx_record` */

/*!50003 DROP PROCEDURE IF EXISTS  `add_mx_record` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `add_mx_record`(
	IN str_name VARCHAR(255),
	IN str_main VARCHAR(255),
	IN n_view INT,
	IN n_ttl INT,
	IN n_level INT,
	IN str_data VARCHAR(255),
	IN n_state INT,
	IN n_enable INT,
	IN n_rid INT)
BEGIN
	DECLARE n_zid INT default NULL;	
	DECLARE arr_tbls VARCHAR(255);
	DECLARE cut_tbl VARCHAR(255);
	declare i int;
	SELECT `id` INTO n_zid FROM zone WHERE domain=str_main AND `view`=n_view LIMIT 1;
	IF n_zid IS NULL THEN
		INSERT INTO zone(client_id,domain,`view`,description) VALUES(1,str_main,n_view,'');
		SELECT `id` INTO n_zid FROM zone WHERE domain=str_main AND `view`=n_view LIMIT 1;
	END IF;
	SET @oname = NULL;
	SET @oview = NULL;
	SET arr_tbls = 'a_record,aaaa_record,cname_record,mx_record,ns_record,txt_record';
	SET i=1;  
	while_label: WHILE i <= 6 DO 	
		SET @oname = NULL;
		SET @oview = NULL;
		SET cut_tbl=CONCAT(SUBSTRING_INDEX(SUBSTRING_INDEX(arr_tbls, ',' ,i),',',-1));
		SET @sqlcmd=CONCAT("SELECT ar.name,ze.view into @oname,@oview FROM ",cut_tbl,
			" ar LEFT JOIN zone ze ON ar.zone=ze.id WHERE ar.enable=1 AND ar.rid=",n_rid);
		PREPARE stmt FROM @sqlcmd;
		EXECUTE stmt;
		IF @oname IS NOT NULL AND @oview IS NOT NULL THEN
			IF STRCMP(cut_tbl, 'mx_record')!=0 THEN
				
				SET @sqlcmd = CONCAT("delete from `", cut_tbl, "` WHERE rid=", n_rid);
				PREPARE stmt FROM @sqlcmd;
				EXECUTE stmt;
			END IF;
			CALL add_snd_req('dns', tbl_to_dnstype(cut_tbl), @oview, @oname, 0, 2);
			LEAVE while_label;
		END IF;
		SET i = i + 1;  
	END WHILE;
	
	
	
	
	
	
	SET @sqlcmd = CONCAT('INSERT INTO `mx_record`',' (zone,`name`,ttl,`level`,`server`,state,`enable`,`rid`) VALUES(',
		n_zid,',\'',str_name,'\',',n_ttl,',',n_level,',\'',str_data,'\',',n_state,',',n_enable,',',n_rid,
		') ON DUPLICATE KEY UPDATE zone=',n_zid,',`name`=\'',str_name,'\',`ttl`=',n_ttl,',`level`=',n_level,
		',`server`=\'',str_data,'\',state=',n_state,',`enable`=',n_enable);
	PREPARE stmt FROM @sqlcmd;  
	EXECUTE stmt;  
	DEALLOCATE PREPARE stmt; 
	
	CALL add_snd_req('dns', tbl_to_dnstype('mx_record'), n_view, str_name, 0, 1);
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `add_snd_req` */

/*!50003 DROP PROCEDURE IF EXISTS  `add_snd_req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `add_snd_req`(
	IN str_class VARCHAR(255), 
	IN n_type INT,
	IN n_vid INT,
	IN str_data VARCHAR(255),
	IN n_status INT,
	IN n_opt INT)
BEGIN
	declare mopt int default 1;
	if n_opt=1 then
		set mopt = 2;
	end if;
	
	UPDATE snd_record SET state=3 WHERE class=str_class 
		AND `type`=n_type 
		AND viewid=n_vid
		AND `data`=str_data
		and state=0
		AND `opt`=mopt;
		
	DELETE FROM `snd_record` WHERE class=str_class 
				AND `type`=n_type 
				AND viewid=n_vid
				AND `data`=str_data
				AND `opt`=n_opt;
	INSERT INTO `snd_record` (class,`type`,viewid,`data`,state,`opt`) 
		VALUES(str_class,n_type,n_vid,str_data,n_status,n_opt);
    END */$$
DELIMITER ;

/* Procedure structure for procedure `del_a_domain` */

/*!50003 DROP PROCEDURE IF EXISTS  `del_a_domain` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `del_a_domain`(
	IN str_domain VARCHAR(255))
BEGIN
	DECLARE done INT DEFAULT 0;
	DECLARE aname VARCHAR(255) DEFAULT NULL;
	DECLARE aview INT DEFAULT -1;
	DECLARE arr_tbls VARCHAR(255);
	DECLARE cut_tbl VARCHAR(255);
	DECLARE n_zid INT DEFAULT 0;
	DECLARE i INT;
	DECLARE cursor_a CURSOR FOR 
		(SELECT ar.name,ze.view FROM `a_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.domain=str_domain);
	DECLARE cursor_aaaa CURSOR FOR 
		(SELECT ar.name,ze.view FROM `aaaa_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.domain=str_domain);
	DECLARE cursor_cname CURSOR FOR 
		(SELECT ar.name,ze.view FROM `cname_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.domain=str_domain);
	DECLARE cursor_mx CURSOR FOR 
		(SELECT ar.name,ze.view FROM `mx_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.domain=str_domain);
	DECLARE cursor_ns CURSOR FOR 
		(SELECT ar.name,ze.view FROM `ns_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.domain=str_domain);
	DECLARE cursor_txt CURSOR FOR 
		(SELECT ar.name,ze.view FROM `txt_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.domain=str_domain);
	DECLARE cursor_zone CURSOR FOR 
		SELECT `id` FROM `zone` WHERE `domain`=str_domain;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
	
	SET arr_tbls = 'a_record,aaaa_record,cname_record,mx_record,ns_record,txt_record';
	
	OPEN cursor_a;
	REPEAT
		IF NOT done THEN
			FETCH cursor_a INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('a_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'a_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_a ;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_aaaa;
	REPEAT
		IF NOT done THEN
			FETCH cursor_aaaa INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('aaaa_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'aaaa_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_aaaa;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_cname;
	REPEAT
		IF NOT done THEN
			FETCH cursor_cname INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('cname_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'cname_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_cname;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_mx;
	REPEAT
		IF NOT done THEN
			FETCH cursor_mx INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('mx_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'mx_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_mx;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_ns;
	REPEAT
		IF NOT done THEN
			FETCH cursor_ns INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('ns_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'ns_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_ns;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_txt;
	REPEAT
		IF NOT done THEN
			FETCH cursor_txt INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('txt_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'txt_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_txt;
	
	SET done = 0;
	SET n_zid = -1;
	OPEN cursor_zone;
	REPEAT
		IF NOT done THEN
			FETCH cursor_zone INTO n_zid;
			IF n_zid!=-1 THEN
				SET i=1;  
				WHILE i <= 6 DO
					SET cut_tbl=CONCAT(SUBSTRING_INDEX(SUBSTRING_INDEX(arr_tbls, ',' ,i),',',-1));
					
					SET @sqlcmd=CONCAT("delete from `", cut_tbl, "` WHERE `zone`=", n_zid);
					PREPARE stmt FROM @sqlcmd;
					EXECUTE stmt;
					DEALLOCATE PREPARE stmt; 
					SET i = i + 1;
				END WHILE;
			SET n_zid=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_zone;
	
	DELETE FROM zone WHERE domain=str_domain;
	DELETE FROM domain_ns WHERE domain=str_domain;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `del_a_record` */

/*!50003 DROP PROCEDURE IF EXISTS  `del_a_record` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `del_a_record`(IN `tab_name` VARCHAR(255), IN `n_rid` INT)
BEGIN
        
        
	SET @oname = NULL;
	SET @oview = NULL;
	SET @ozid = NULL;
	SET @sqlcmd = CONCAT("SELECT ar.name,ze.view,ar.zone into @oname,@oview,@ozid FROM ",tab_name,
		" ar LEFT JOIN zone ze ON ar.zone=ze.id WHERE ar.enable=1 AND ar.rid=",n_rid," LIMIT 1");
	PREPARE stmt FROM @sqlcmd;
	EXECUTE stmt;
	
	
	
	SET @sqlcmd = CONCAT('delete from ',tab_name,' WHERE rid=',n_rid);
	PREPARE stmt FROM @sqlcmd;
	EXECUTE stmt;
	
	SET @same_dn_cnt = 0;	
	if @oname is not null and @ozid IS NOT NULL then
		SET @sqlcmd = CONCAT("SELECT count(*) into @same_dn_cnt FROM ",tab_name,
			" WHERE `name`='",@oname,"' AND `zone`=",@ozid," AND `enable`=1");
		PREPARE stmt FROM @sqlcmd;
		EXECUTE stmt;
	end if;
	
	DEALLOCATE PREPARE stmt;
	
	if @ozid is not null then
		update zone set update_time=NOW() where id=@ozid;
	end if;
    
	if @oview is not null and @oname is not null then
		if @same_dn_cnt > 0 then
			CALL add_snd_req('dns', tbl_to_dnstype(tab_name), @oview, @oname, 0, 1);
		else
			CALL add_snd_req('dns', tbl_to_dnstype(tab_name), @oview, @oname, 0, 2);
		end if;
	end if;
	SELECT @oname,@oview,@same_dn_cnt;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `del_a_view` */

/*!50003 DROP PROCEDURE IF EXISTS  `del_a_view` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `del_a_view`(
	IN n_view INT)
BEGIN

	DECLARE done INT DEFAULT 0;
	DECLARE aname VARCHAR(255) DEFAULT NULL;
	DECLARE aview INT DEFAULT -1;
	DECLARE arr_tbls VARCHAR(255);
	DECLARE cut_tbl VARCHAR(255);
	DECLARE n_zid INT DEFAULT 0;
	DECLARE i INT;
	DECLARE cursor_a CURSOR FOR 
		(SELECT ar.name,ze.view FROM `a_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view);
	DECLARE cursor_aaaa CURSOR FOR 
		(SELECT ar.name,ze.view FROM `aaaa_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view);
	DECLARE cursor_cname CURSOR FOR 
		(SELECT ar.name,ze.view FROM `cname_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view);
	DECLARE cursor_mx CURSOR FOR 
		(SELECT ar.name,ze.view FROM `mx_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view);
	DECLARE cursor_ns CURSOR FOR 
		(SELECT ar.name,ze.view FROM `ns_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view);
	DECLARE cursor_txt CURSOR FOR 
		(SELECT ar.name,ze.view FROM `txt_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view);
	DECLARE cursor_zone CURSOR FOR 
		SELECT `id` FROM `zone` WHERE `view`=n_view;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
	
	SET arr_tbls = 'a_record,aaaa_record,cname_record,mx_record,ns_record,txt_record';
	
	OPEN cursor_a;
	REPEAT
		IF NOT done THEN
			FETCH cursor_a INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('a_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'a_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_a ;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_aaaa;
	REPEAT
		IF NOT done THEN
			FETCH cursor_aaaa INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('aaaa_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'aaaa_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_aaaa;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_cname;
	REPEAT
		IF NOT done THEN
			FETCH cursor_cname INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('cname_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'cname_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_cname;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_mx;
	REPEAT
		IF NOT done THEN
			FETCH cursor_mx INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('mx_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'mx_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_mx;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_ns;
	REPEAT
		IF NOT done THEN
			FETCH cursor_ns INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('ns_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'ns_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_ns;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_txt;
	REPEAT
		IF NOT done THEN
			FETCH cursor_txt INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('txt_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'txt_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_txt;
	
	SET done = 0;
	SET n_zid = -1;
	OPEN cursor_zone;
	REPEAT
		IF NOT done THEN
			FETCH cursor_zone INTO n_zid;
			IF n_zid!=-1 THEN
				SET i=1;  
				WHILE i <= 6 DO
					SET cut_tbl=CONCAT(SUBSTRING_INDEX(SUBSTRING_INDEX(arr_tbls, ',' ,i),',',-1));
					SET @sqlcmd=CONCAT("UPDATE `", cut_tbl, "` SET `enable`=0 WHERE `enable`=1 AND `zone`=", n_zid);
					PREPARE stmt FROM @sqlcmd;
					EXECUTE stmt;
					DEALLOCATE PREPARE stmt; 
					SET i = i + 1;
				END WHILE;
			SET n_zid=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_zone;
	
	DELETE FROM zone WHERE `view`=n_view;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `del_a_view_domain` */

/*!50003 DROP PROCEDURE IF EXISTS  `del_a_view_domain` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `del_a_view_domain`(
	IN n_view INT,
	in str_domain varchar(255))
BEGIN

	DECLARE done INT DEFAULT 0;
	DECLARE aname VARCHAR(255) DEFAULT NULL;
	DECLARE aview INT DEFAULT -1;
	DECLARE arr_tbls VARCHAR(255);
	DECLARE cut_tbl VARCHAR(255);
	DECLARE n_zid INT default 0;
	DECLARE i INT;
	DECLARE cursor_a CURSOR FOR 
		(SELECT ar.name,ze.view FROM `a_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view and ze.domain=str_domain);
	DECLARE cursor_aaaa CURSOR FOR 
		(SELECT ar.name,ze.view FROM `aaaa_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view AND ze.domain=str_domain);
	DECLARE cursor_cname CURSOR FOR 
		(SELECT ar.name,ze.view FROM `cname_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view AND ze.domain=str_domain);
	DECLARE cursor_mx CURSOR FOR 
		(SELECT ar.name,ze.view FROM `mx_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view AND ze.domain=str_domain);
	DECLARE cursor_ns CURSOR FOR 
		(SELECT ar.name,ze.view FROM `ns_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view AND ze.domain=str_domain);
	DECLARE cursor_txt CURSOR FOR 
		(SELECT ar.name,ze.view FROM `txt_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=1 AND ze.view=n_view AND ze.domain=str_domain);
	DECLARE cursor_zone CURSOR FOR 
		SELECT `id` FROM `zone` WHERE `domain`=str_domain AND `view`=n_view;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
	
	SET arr_tbls = 'a_record,aaaa_record,cname_record,mx_record,ns_record,txt_record';
	
	OPEN cursor_a;
	REPEAT
		IF NOT done THEN
			FETCH cursor_a INTO aname, aview;
			if aview!=-1 then
				CALL add_snd_req('dns', tbl_to_dnstype('a_record'), aview, aname, 0, 2);
				select aname, aview, 'a_record';
				set aview=-1;
			end if;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_a ;
	
	SET done = 0;
	set aview = -1;
	OPEN cursor_aaaa;
	REPEAT
		IF NOT done THEN
			FETCH cursor_aaaa INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('aaaa_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'aaaa_record';
				SET aview=-1;
			end if;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_aaaa;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_cname;
	REPEAT
		IF NOT done THEN
			FETCH cursor_cname INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('cname_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'cname_record';
				SET aview=-1;
			end if;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_cname;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_mx;
	REPEAT
		IF NOT done THEN
			FETCH cursor_mx INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('mx_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'mx_record';
				SET aview=-1;
			end if;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_mx;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_ns;
	REPEAT
		IF NOT done THEN
			FETCH cursor_ns INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('ns_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'ns_record';
				SET aview=-1;
			end if;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_ns;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_txt;
	REPEAT
		IF NOT done THEN
			FETCH cursor_txt INTO aname, aview;
			IF aview!=-1 THEN
				CALL add_snd_req('dns', tbl_to_dnstype('txt_record'), aview, aname, 0, 2);
				SELECT aname, aview, 'txt_record';
				SET aview=-1;
			end if;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_txt;
	
	SET done = 0;
	SET n_zid = -1;
	OPEN cursor_zone;
	REPEAT
		IF NOT done THEN
			FETCH cursor_zone INTO n_zid;
			IF n_zid!=-1 THEN
				SET i=1;  
				WHILE i <= 6 DO
					SET cut_tbl=CONCAT(SUBSTRING_INDEX(SUBSTRING_INDEX(arr_tbls, ',' ,i),',',-1));
					SET @sqlcmd=CONCAT("UPDATE `", cut_tbl, "` SET `enable`=0 WHERE `enable`=1 AND `zone`=", n_zid);
					PREPARE stmt FROM @sqlcmd;
					EXECUTE stmt;
					DEALLOCATE PREPARE stmt; 
					SET i = i + 1;
				END WHILE;
			SET n_zid=-1;
			end if;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_zone;
	
	delete from zone where `view`=n_view and domain=str_domain;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `del_a_view_mask` */

/*!50003 DROP PROCEDURE IF EXISTS  `del_a_view_mask` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `del_a_view_mask`(
	IN n_view INT, 
	IN str_nw VARCHAR(255))
BEGIN

	DECLARE done INT DEFAULT 0;
	DECLARE onw VARCHAR(255) DEFAULT NULL;
	DECLARE oview INT DEFAULT -1;
	DECLARE cur CURSOR FOR 
		SELECT network, viewid FROM `view_mask` 
		WHERE viewid=n_view and network=str_nw AND `status`='true';
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
	
	OPEN cur;
	REPEAT
		IF NOT done THEN
			FETCH cur INTO onw, oview;
			IF oview!=-1 THEN
				CALL add_snd_req('view', 0, oview, onw, 0, 2);
				SELECT onw, oview;
			END IF;
			SET oview=-1;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cur ;
	
	update `view_mask` set `status`='false'
		WHERE viewid=n_view AND network=str_nw AND `status`='true';
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `onoff_a_domain` */

/*!50003 DROP PROCEDURE IF EXISTS  `onoff_a_domain` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `onoff_a_domain`(
	IN str_domain VARCHAR(255), in n_rop int)
BEGIN
	-- n_rop 反操作，即：想开启传0，想关闭传1
	DECLARE done INT DEFAULT 0;
	DECLARE aname VARCHAR(255) DEFAULT NULL;
	DECLARE aview INT DEFAULT -1;
	DECLARE arr_tbls VARCHAR(255);
	DECLARE cut_tbl VARCHAR(255);
	DECLARE n_zid INT DEFAULT 0;
	declare n_goodop int default 0;
	DECLARE i INT;
	DECLARE cursor_a CURSOR FOR 
		(SELECT ar.name,ze.view FROM `a_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=n_rop AND ze.domain=str_domain);
	DECLARE cursor_aaaa CURSOR FOR 
		(SELECT ar.name,ze.view FROM `aaaa_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=n_rop AND ze.domain=str_domain);
	DECLARE cursor_cname CURSOR FOR 
		(SELECT ar.name,ze.view FROM `cname_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=n_rop AND ze.domain=str_domain);
	DECLARE cursor_mx CURSOR FOR 
		(SELECT ar.name,ze.view FROM `mx_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=n_rop AND ze.domain=str_domain);
	DECLARE cursor_ns CURSOR FOR 
		(SELECT ar.name,ze.view FROM `ns_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=n_rop AND ze.domain=str_domain);
	DECLARE cursor_txt CURSOR FOR 
		(SELECT ar.name,ze.view FROM `txt_record` ar LEFT JOIN `zone` ze ON ar.zone=ze.id 
		WHERE ar.enable=n_rop AND ze.domain=str_domain);
	DECLARE cursor_zone CURSOR FOR 
		SELECT `id` FROM `zone` WHERE `domain`=str_domain;
	DECLARE CONTINUE HANDLER FOR NOT FOUND SET done = 1;
	
	SET arr_tbls = 'a_record,aaaa_record,cname_record,mx_record,ns_record,txt_record';
	
	if n_rop=0 then
		set n_goodop=1;
	end if;
	
	OPEN cursor_a;
	REPEAT
		IF NOT done THEN
			FETCH cursor_a INTO aname, aview;
			IF aview!=-1 THEN
				if n_rop != 0 then
					CALL add_snd_req('dns', tbl_to_dnstype('a_record'), aview, aname, 0, 2);
				end if;
				SELECT aname, aview, 'a_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_a ;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_aaaa;
	REPEAT
		IF NOT done THEN
			FETCH cursor_aaaa INTO aname, aview;
			IF aview!=-1 THEN
				IF n_rop != 0 THEN
					CALL add_snd_req('dns', tbl_to_dnstype('aaaa_record'), aview, aname, 0, 2);
				end if;
				SELECT aname, aview, 'aaaa_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_aaaa;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_cname;
	REPEAT
		IF NOT done THEN
			FETCH cursor_cname INTO aname, aview;
			IF aview!=-1 THEN
				IF n_rop != 0 THEN
					CALL add_snd_req('dns', tbl_to_dnstype('cname_record'), aview, aname, 0, 2);
				end if;				
				SELECT aname, aview, 'cname_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_cname;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_mx;
	REPEAT
		IF NOT done THEN
			FETCH cursor_mx INTO aname, aview;
			IF aview!=-1 THEN
				IF n_rop != 0 THEN
					CALL add_snd_req('dns', tbl_to_dnstype('mx_record'), aview, aname, 0, 2);
				end if;
				SELECT aname, aview, 'mx_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_mx;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_ns;
	REPEAT
		IF NOT done THEN
			FETCH cursor_ns INTO aname, aview;
			IF aview!=-1 THEN
				IF n_rop != 0 THEN
					CALL add_snd_req('dns', tbl_to_dnstype('ns_record'), aview, aname, 0, 2);
				end if;
				SELECT aname, aview, 'ns_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_ns;
	
	SET done = 0;
	SET aview = -1;
	OPEN cursor_txt;
	REPEAT
		IF NOT done THEN
			FETCH cursor_txt INTO aname, aview;
			IF aview!=-1 THEN
				IF n_rop != 0 THEN
					CALL add_snd_req('dns', tbl_to_dnstype('txt_record'), aview, aname, 0, 2);
				end if;
				SELECT aname, aview, 'txt_record';
				SET aview=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_txt;
	
	SET done = 0;
	SET n_zid = -1;
	OPEN cursor_zone;
	REPEAT
		IF NOT done THEN
			FETCH cursor_zone INTO n_zid;
			IF n_zid!=-1 THEN
				SET i=1;  
				WHILE i <= 6 DO
					SET cut_tbl=CONCAT(SUBSTRING_INDEX(SUBSTRING_INDEX(arr_tbls, ',' ,i),',',-1));
					
					SET @sqlcmd=CONCAT("update `", cut_tbl, "` set `enable`=", n_goodop, " WHERE `zone`=", n_zid);
					PREPARE stmt FROM @sqlcmd;
					EXECUTE stmt;
					DEALLOCATE PREPARE stmt; 
					SET i = i + 1;
				END WHILE;
			SET n_zid=-1;
			END IF;
		END IF;
	UNTIL done END REPEAT;
	CLOSE cursor_zone;
	
    END */$$
DELIMITER ;

/* Procedure structure for procedure `update_domain_zone` */

/*!50003 DROP PROCEDURE IF EXISTS  `update_domain_zone` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `update_domain_zone`(IN `sub` VARCHAR(100), IN `nid` INT)
begin
declare v_main varchar(256) default null;    
select `domain` INTO v_main from `zone` where `id`=nid limit 1;

INSERT INTO `domain_zone`(`domain`,`zone`) VALUES(sub,v_main) 
	on duplicate key update zone=v_main;
end */$$
DELIMITER ;

/* Procedure structure for procedure `update_snd_req` */

/*!50003 DROP PROCEDURE IF EXISTS  `update_snd_req` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `update_snd_req`(
	IN str_class VARCHAR(255), 
	IN n_type INT,
	IN n_vid INT,
	IN str_data VARCHAR(255),
	IN n_status INT,
	IN n_opt INT)
BEGIN
	UPDATE snd_record SET state=n_status WHERE class=str_class 
		AND `type`=n_type 
		AND viewid=n_vid
		AND `data`=str_data
		AND `opt`=n_opt;
    END */$$
DELIMITER ;

/* Procedure structure for procedure `update_zone` */

/*!50003 DROP PROCEDURE IF EXISTS  `update_zone` */;

DELIMITER $$

/*!50003 CREATE DEFINER=`root`@`%` PROCEDURE `update_zone`(IN `idin` INT)
begin
update `zone` set update_time=now() where `id`=idin;
end */$$
DELIMITER ;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

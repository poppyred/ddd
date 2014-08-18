/*
SQLyog Ultimate v11.31 (32 bit)
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
) ENGINE=MyISAM AUTO_INCREMENT=16 DEFAULT CHARSET=utf8;

/*Data for the table `a_record` */

insert  into `a_record`(`id`,`zone`,`name`,`ttl`,`ipaddr`,`description`,`state`,`enable`,`rid`) values (3,2,'www.hyb.com',600,'1.2.3.4','',0,1,578),(4,2,'aaa.hyb.com',600,'3.4.5.6','',0,1,579),(8,2,'sss.hyb.com',600,'6.7.8.9','',0,1,585),(6,2,'*.hyb.com',600,'7.7.7.7','',0,1,581),(10,6,'www.eflydns.com',600,'10.10.12.56','',0,1,595),(9,5,'www.hhyb.com',600,'1.2.3.4','',0,1,590),(15,10,'www.lkx.org',600,'202.96.128.166','',0,1,608);

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
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `cname_record` */

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
) ENGINE=MyISAM AUTO_INCREMENT=27 DEFAULT CHARSET=utf8;

/*Data for the table `domain_ns` */

insert  into `domain_ns`(`id`,`domain`,`ttl`,`server`,`description`,`rid`) values (8,'hyb.com',600,'ns2.eflydns.net','null',577),(7,'fdsa.cn',600,'ns2.eflydns.net','null',575),(6,'hyb.com',600,'ns1.eflydns.net','null',576),(5,'fdsa.cn',600,'ns1.eflydns.net','null',574),(18,'eflydns.com',600,'ns1.eflydns.net','null',593),(17,'eflydns.com',600,'ns2.eflydns.net','null',594),(11,'mysq.cn',600,'ns2.eflydns.net','null',587),(12,'mysq.cn',600,'ns1.eflydns.net','null',586),(13,'hhyb.com',600,'ns2.eflydns.net','null',589),(14,'hhyb.com',600,'ns1.eflydns.net','null',588),(26,'lkx.org',600,'ns2.eflydns.net','null',607),(25,'lkx.org',600,'ns1.eflydns.net','null',606);

/*Table structure for table `domain_zone` */

DROP TABLE IF EXISTS `domain_zone`;

CREATE TABLE `domain_zone` (
  `domain` varchar(256) NOT NULL,
  `zone` varchar(256) NOT NULL,
  PRIMARY KEY (`domain`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `domain_zone` */

insert  into `domain_zone`(`domain`,`zone`) values ('www.hyb.com','hyb.com'),('aaa.hyb.com','hyb.com'),('ttt.hyb.com','hyb.com'),('*.hyb.com','hyb.com'),('www.eflydns.com','eflydns.com'),('sss.hyb.com','hyb.com'),('www.hhyb.com','hhyb.com'),('www.lkx.cn','lkx.cn'),('www.lkx.org','lkx.org');

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
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ns_record` */

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
) ENGINE=MyISAM AUTO_INCREMENT=8688 DEFAULT CHARSET=utf8;

/*Data for the table `snd_record` */

insert  into `snd_record`(`id`,`class`,`type`,`viewid`,`data`,`state`,`chktime`,`opt`) values (8669,'dns',1,1,'www.hyb.com',1,'2014-08-12 15:01:19',1),(8670,'dns',1,1,'aaa.hyb.com',1,'2014-08-12 15:01:19',1),(8671,'dns',1,2,'ttt.hyb.com',1,'2014-08-12 15:01:19',1),(8672,'dns',1,1,'*.hyb.com',1,'2014-08-12 15:01:19',1),(8678,'dns',1,1,'www.eflydns.com',1,'2014-08-18 10:36:54',1),(8674,'dns',1,2,'ttt.hyb.com',1,'2014-08-12 15:34:46',2),(8675,'dns',1,1,'sss.hyb.com',1,'2014-08-12 15:35:04',1),(8676,'dns',1,1,'www.hhyb.com',1,'2014-08-18 09:13:06',1),(8677,'dns',1,1,'www.eflydns.com',1,'2014-08-18 10:35:53',2),(8685,'dns',1,1,'www.lkx.cn',1,'2014-08-18 10:49:35',1),(8687,'dns',1,1,'www.lkx.org',1,'2014-08-18 11:00:22',1),(8686,'dns',1,1,'www.lkx.cn',1,'2014-08-18 10:54:39',2);

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
) ENGINE=MyISAM AUTO_INCREMENT=11 DEFAULT CHARSET=utf8;

/*Data for the table `zone` */

insert  into `zone`(`id`,`client_id`,`domain`,`view`,`update_time`,`description`) values (2,1,'hyb.com',1,'2014-08-12 15:35:04',''),(3,1,'hyb.com',2,'2014-08-12 15:34:46',''),(6,1,'eflydns.com',1,'2014-08-18 10:36:54',''),(5,1,'hhyb.com',1,'2014-08-18 09:13:06',''),(10,1,'lkx.org',1,'2014-08-18 11:00:21','');

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
	
	DELETE FROM zone WHERE domain=str_domain;
	DELETE FROM domain_ns WHERE domain=str_domain;
	
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

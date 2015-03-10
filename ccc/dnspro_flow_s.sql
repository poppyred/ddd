/*
SQLyog Ultimate v11.31 (32 bit)
MySQL - 5.1.73 : Database - dnspro_flow
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`dnspro_flow` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `dnspro_flow`;

/*Table structure for table `cpu_record` */

DROP TABLE IF EXISTS `cpu_record`;

CREATE TABLE `cpu_record` (
  `ID` int(4) NOT NULL,
  `ip` varchar(16) NOT NULL,
  `rate` int(10) NOT NULL,
  `pid_rate` int(10) NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`ID`,`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `domain_all_conc` */

DROP TABLE IF EXISTS `domain_all_conc`;

CREATE TABLE `domain_all_conc` (
  `id` bigint(8) NOT NULL AUTO_INCREMENT,
  `domain` text NOT NULL,
  `date` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=41 DEFAULT CHARSET=utf8;

/*Table structure for table `domain_conc` */

DROP TABLE IF EXISTS `domain_conc`;

CREATE TABLE `domain_conc` (
  `id` bigint(8) NOT NULL AUTO_INCREMENT,
  `domain` text NOT NULL,
  `date` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=37 DEFAULT CHARSET=utf8;

/*Table structure for table `domain_ip_conc` */

DROP TABLE IF EXISTS `domain_ip_conc`;

CREATE TABLE `domain_ip_conc` (
  `domain` varchar(32) NOT NULL,
  `id` bigint(8) NOT NULL AUTO_INCREMENT,
  `ip` text NOT NULL,
  `date` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=401 DEFAULT CHARSET=utf8;

/*Table structure for table `domain_main_conc` */

DROP TABLE IF EXISTS `domain_main_conc`;

CREATE TABLE `domain_main_conc` (
  `id` bigint(8) NOT NULL AUTO_INCREMENT,
  `domain` text NOT NULL,
  `date` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=35 DEFAULT CHARSET=utf8;

/*Table structure for table `error_record` */

DROP TABLE IF EXISTS `error_record`;

CREATE TABLE `error_record` (
  `ID` int(4) NOT NULL,
  `ip` varchar(16) CHARACTER SET utf8 NOT NULL,
  `number` int(10) NOT NULL,
  `detail` text,
  `domain` text CHARACTER SET utf8 NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`ID`,`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

/*Table structure for table `error_record_hour` */

DROP TABLE IF EXISTS `error_record_hour`;

CREATE TABLE `error_record_hour` (
  `ID` int(8) NOT NULL AUTO_INCREMENT,
  `ip` varchar(16) CHARACTER SET utf8 NOT NULL,
  `number` int(10) NOT NULL,
  `detail` text,
  `domain` text CHARACTER SET utf8 NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`ID`,`ip`)
) ENGINE=MyISAM AUTO_INCREMENT=6215 DEFAULT CHARSET=latin1;

/*Table structure for table `ip_conc` */

DROP TABLE IF EXISTS `ip_conc`;

CREATE TABLE `ip_conc` (
  `id` bigint(8) NOT NULL AUTO_INCREMENT,
  `ip` text NOT NULL,
  `date` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=31 DEFAULT CHARSET=utf8;

/*Table structure for table `region_conc` */

DROP TABLE IF EXISTS `region_conc`;

CREATE TABLE `region_conc` (
  `id` bigint(8) NOT NULL AUTO_INCREMENT,
  `region` text NOT NULL,
  `date` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=35 DEFAULT CHARSET=utf8;

/*Table structure for table `server_list` */

DROP TABLE IF EXISTS `server_list`;

CREATE TABLE `server_list` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `server` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=utf8;

/*Table structure for table `type_conc` */

DROP TABLE IF EXISTS `type_conc`;

CREATE TABLE `type_conc` (
  `id` bigint(8) NOT NULL AUTO_INCREMENT,
  `type` text NOT NULL,
  `date` varchar(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=38 DEFAULT CHARSET=utf8;

/*Table structure for table `type_record` */

DROP TABLE IF EXISTS `type_record`;

CREATE TABLE `type_record` (
  `ID` int(16) NOT NULL,
  `al` int(10) NOT NULL,
  `ca` int(10) NOT NULL,
  `jc` int(10) NOT NULL,
  `re` int(10) NOT NULL,
  `an` int(10) NOT NULL,
  `ir` int(10) NOT NULL,
  `dr` int(10) NOT NULL DEFAULT '0',
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `ip` varchar(20) CHARACTER SET utf8 NOT NULL,
  PRIMARY KEY (`ID`,`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

/*Table structure for table `type_record_hour` */

DROP TABLE IF EXISTS `type_record_hour`;

CREATE TABLE `type_record_hour` (
  `ID` int(16) NOT NULL AUTO_INCREMENT,
  `al` int(10) NOT NULL,
  `ca` int(10) NOT NULL,
  `jc` int(10) NOT NULL,
  `re` int(10) NOT NULL,
  `an` int(10) NOT NULL,
  `ir` int(10) NOT NULL,
  `dr` int(10) NOT NULL DEFAULT '0',
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `ip` varchar(20) CHARACTER SET utf8 NOT NULL,
  PRIMARY KEY (`ID`,`ip`)
) ENGINE=MyISAM AUTO_INCREMENT=6196 DEFAULT CHARSET=latin1;

/*Table structure for table `view_index` */

DROP TABLE IF EXISTS `view_index`;

CREATE TABLE `view_index` (
  `id` smallint(4) NOT NULL AUTO_INCREMENT,
  `view_name` varchar(32) NOT NULL,
  `status` varchar(10) NOT NULL,
  `comment` varchar(255) NOT NULL,
  `err_status` varchar(11) NOT NULL DEFAULT 'false',
  `ttl` int(11) NOT NULL DEFAULT '300',
  PRIMARY KEY (`id`),
  UNIQUE KEY `id` (`id`,`view_name`)
) ENGINE=MyISAM AUTO_INCREMENT=8 DEFAULT CHARSET=utf8;

/*Table structure for table `view_record` */

DROP TABLE IF EXISTS `view_record`;

CREATE TABLE `view_record` (
  `view_id` int(10) NOT NULL,
  `time_id` int(10) NOT NULL,
  `al` int(10) NOT NULL DEFAULT '0',
  `dr` int(10) NOT NULL,
  `ca` int(10) NOT NULL,
  `jc` int(10) NOT NULL,
  `re` int(10) NOT NULL,
  `an` int(10) NOT NULL,
  `ir` int(10) NOT NULL DEFAULT '0',
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `ip` varchar(16) NOT NULL,
  UNIQUE KEY `view_id` (`view_id`,`time_id`,`ip`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Table structure for table `view_record_hour` */

DROP TABLE IF EXISTS `view_record_hour`;

CREATE TABLE `view_record_hour` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `view_id` int(10) NOT NULL,
  `al` int(10) NOT NULL DEFAULT '0',
  `dr` int(10) NOT NULL,
  `ca` int(10) NOT NULL,
  `jc` int(10) NOT NULL,
  `re` int(10) NOT NULL,
  `an` int(10) NOT NULL,
  `ir` int(10) NOT NULL DEFAULT '0',
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `ip` varchar(16) NOT NULL,
  UNIQUE KEY `view_id` (`id`,`view_id`,`ip`)
) ENGINE=MyISAM AUTO_INCREMENT=44472 DEFAULT CHARSET=utf8;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

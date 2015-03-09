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

/*Data for the table `server_list` */

insert  into `server_list`(`id`,`server`) values (1,'121.201.12.56'),(2,'121.201.12.66');

/*Data for the table `view_index` */

insert  into `view_index`(`id`,`view_name`,`status`,`comment`,`err_status`,`ttl`) values (2,'电信视图','true','广东电信','false',300),(3,'联通视图','true','','false',300),(4,'广东铁通视图','true','','false',300),(5,'睿江办公网视图','true','','false',300),(6,'全局视图','true','不能删除','false',300),(1,'默认视图','true','不能删除','false',300),(7,'efly劫持','true','20140220添加','false',300);

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

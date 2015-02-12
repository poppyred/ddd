/*
SQLyog Ultimate v11.31 (32 bit)
MySQL - 5.1.73 : Database - dns_http_ref
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`dns_http_ref` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `dns_http_ref`;

/*Table structure for table `http_ref` */

DROP TABLE IF EXISTS `http_ref`;

CREATE TABLE `http_ref` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `src` varchar(256) NOT NULL,
  `dst` varchar(256) NOT NULL,
  `status` varchar(10) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `src` (`src`)
) ENGINE=MyISAM AUTO_INCREMENT=33 DEFAULT CHARSET=utf8;

/*Data for the table `http_ref` */

insert  into `http_ref`(`id`,`src`,`dst`,`status`) values (29,'dns.eflypro.com','www.eflydns.com','true');

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

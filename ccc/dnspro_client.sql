-- phpMyAdmin SQL Dump
-- version 2.11.11.3
-- http://www.phpmyadmin.net
--
-- 主机: localhost
-- 生成日期: 2014 年 08 月 19 日 16:27
-- 服务器版本: 5.1.73
-- PHP 版本: 5.3.3

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- 数据库: `dnspro_client`
--

-- --------------------------------------------------------

--
-- 表的结构 `client`
--

CREATE TABLE IF NOT EXISTS `client` (
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
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=19 ;

--
-- 导出表中的数据 `client`
--

INSERT INTO `client` (`id`, `type`, `mail`, `pwd`, `org_name`, `org_num`, `link_phone`, `link_man`, `link_weixin`, `reg_time`, `login_time`) VALUES
(10, 0, 'hyb@qq.com', '962012d09b8170d912f0669f6d7d9d07', '', '', '', '', '', '', ''),
(11, 0, 'test@efly.cc', 'e10adc3949ba59abbe56e057f20f883e', '', '', '', '', '', '', ''),
(12, 0, 'yanfa@efly.cc', '671ae484da91ccf27bb96c687e931b7e', '', '', '', '', '', '', ''),
(13, 0, 'oujz@efly.cc', 'e10adc3949ba59abbe56e057f20f883e', '广东睿江科技有限公司', '', '18565485654', '测试员', '46235412', '', ''),
(14, 0, 'ddd@efly.cc', '77963b7a931377ad4ab5ad6a9cd718aa', '', '', '', '', '', '', ''),
(15, 0, '123@qq.com', 'e10adc3949ba59abbe56e057f20f883e', '', '', '', '', '', '', ''),
(16, 0, 'fff@fek.cc', '671ae484da91ccf27bb96c687e931b7e', '', '', '', '', '', '', ''),
(17, 0, '1234@qq.com', '671ae484da91ccf27bb96c687e931b7e', '', '', '', '', '', '', ''),
(18, 0, 'likx@efly.cc', 'e10adc3949ba59abbe56e057f20f883e', '', '', '', '', '', '', '');

-- --------------------------------------------------------

--
-- 表的结构 `client_domain`
--

CREATE TABLE IF NOT EXISTS `client_domain` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `client_id` int(16) NOT NULL,
  `zone_id` int(16) NOT NULL,
  `domain_id` int(16) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=900 ;

--
-- 导出表中的数据 `client_domain`
--

INSERT INTO `client_domain` (`id`, `client_id`, `zone_id`, `domain_id`) VALUES
(838, 10, 295, 576),
(839, 10, 295, 577),
(840, 10, 295, 578),
(841, 10, 295, 579),
(843, 10, 295, 581),
(847, 10, 295, 585),
(848, 13, 297, 586),
(849, 13, 297, 587),
(877, 12, 307, 615),
(878, 12, 307, 616),
(879, 12, 307, 617),
(880, 12, 308, 618),
(881, 12, 308, 619),
(882, 12, 308, 620),
(891, 13, 297, 629),
(895, 12, 312, 633),
(896, 12, 312, 634),
(897, 12, 312, 635),
(898, 12, 307, 636),
(899, 12, 307, 637);

-- --------------------------------------------------------

--
-- 表的结构 `client_group`
--

CREATE TABLE IF NOT EXISTS `client_group` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `client_id` int(16) NOT NULL,
  `group` varchar(64) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=13 ;

--
-- 导出表中的数据 `client_group`
--


-- --------------------------------------------------------

--
-- 表的结构 `domain`
--

CREATE TABLE IF NOT EXISTS `domain` (
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
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=638 ;

--
-- 导出表中的数据 `domain`
--

INSERT INTO `domain` (`id`, `host`, `type`, `view`, `route`, `val`, `mx`, `ttl`, `is_edit`, `is_on`, `desc`, `up_time`) VALUES
(1, '@', 'NS', 0, 0, 'vip1.eflydns.net', 0, 10, 0, 1, '这是VIP域名NS记录，不能修改！', '2014-07-22 15:15:23'),
(2, '@', 'NS', 0, 0, 'vip2.eflydns.net', 0, 10, 0, 1, '这是VIP域名NS记录，不能修改！', '2014-06-24 14:15:29'),
(3, '@', 'NS', 0, 0, 'ns1.eflydns.net', 0, 10, 0, 1, '这是免费用户专用NS记录，不能修改！', '2014-06-24 14:15:29'),
(4, '@', 'NS', 0, 0, 'ns2.eflydns.net', 0, 10, 0, 1, '这是免费用户专用NS记录，不能修改！', '2014-06-24 14:15:29'),
(574, '@', 'NS', 0, 0, 'ns1.eflydns.net', 0, 10, 0, 1, '', '2014-08-12 10:34:37'),
(575, '@', 'NS', 0, 0, 'ns2.eflydns.net', 0, 10, 0, 1, '', '2014-08-12 10:34:37'),
(576, '@', 'NS', 0, 0, 'ns1.eflydns.net', 0, 10, 0, 1, '', '2014-08-12 10:43:39'),
(577, '@', 'NS', 0, 0, 'ns2.eflydns.net', 0, 10, 0, 1, '', '2014-08-12 10:43:39'),
(578, 'www', 'A', 1, 0, '1.2.3.4', 0, 10, 1, 1, '', '2014-08-12 10:45:54'),
(579, 'aaa', 'A', 1, 0, '3.4.5.6', 0, 10, 1, 1, '', '2014-08-12 11:56:22'),
(581, '*', 'A', 1, 0, '7.7.7.7', 0, 10, 1, 1, '', '2014-08-12 14:47:18'),
(585, 'sss', 'A', 1, 0, '6.7.8.9', 0, 10, 1, 1, '', '2014-08-12 15:35:02'),
(586, '@', 'NS', 0, 0, 'ns1.eflydns.net', 0, 10, 0, 1, '', '2014-08-14 11:27:34'),
(587, '@', 'NS', 0, 0, 'ns2.eflydns.net', 0, 10, 0, 1, '', '2014-08-14 11:27:34'),
(615, '@', 'NS', 0, 0, 'ns1.eflydns.net', 0, 10, 0, 1, '', '2014-08-19 09:44:18'),
(616, '@', 'NS', 0, 0, 'ns2.eflydns.net', 0, 10, 0, 1, '', '2014-08-19 09:44:18'),
(617, 'www', 'A', 1, 0, '202.96.128.166', 0, 10, 1, 1, '', '2014-08-19 09:44:42'),
(618, '@', 'NS', 0, 0, 'ns1.eflydns.net', 0, 10, 0, 1, '', '2014-08-19 09:46:04'),
(619, '@', 'NS', 0, 0, 'ns2.eflydns.net', 0, 10, 0, 1, '', '2014-08-19 09:46:04'),
(620, 'www', 'A', 1, 0, '121.201.12.61', 0, 10, 1, 1, '', '2014-08-19 09:46:25'),
(629, 'ew', 'A', 1, 0, '20.45.4.4', 0, 10, 1, 0, '', '2014-08-19 10:32:51'),
(633, '@', 'NS', 0, 0, 'ns1.eflydns.net', 0, 10, 0, 1, '', '2014-08-19 10:51:11'),
(634, '@', 'NS', 0, 0, 'ns2.eflydns.net', 0, 10, 0, 1, '', '2014-08-19 10:51:11'),
(635, 'www', 'A', 1, 0, '7.7.73.7', 0, 10, 1, 1, '', '2014-08-19 10:51:25'),
(636, 'likunxiang', 'A', 1, 0, '8.8.8.8', 0, 10, 1, 1, '', '2014-08-19 15:11:15'),
(637, 'baidu', 'A', 1, 0, '202.96.128.86', 0, 10, 1, 1, '', '2014-08-19 15:16:55');

-- --------------------------------------------------------

--
-- 表的结构 `level`
--

CREATE TABLE IF NOT EXISTS `level` (
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
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- 导出表中的数据 `level`
--


-- --------------------------------------------------------

--
-- 表的结构 `route`
--

CREATE TABLE IF NOT EXISTS `route` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `name` varchar(128) NOT NULL,
  `timespan` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '添加时间',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=38 ;

--
-- 导出表中的数据 `route`
--


-- --------------------------------------------------------

--
-- 表的结构 `route_mask`
--

CREATE TABLE IF NOT EXISTS `route_mask` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `route_id` int(16) NOT NULL,
  `mask_start` int(16) NOT NULL,
  `mask_end` int(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=51 ;

--
-- 导出表中的数据 `route_mask`
--


-- --------------------------------------------------------

--
-- 表的结构 `view`
--

CREATE TABLE IF NOT EXISTS `view` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `name` varchar(64) NOT NULL,
  `level` int(4) NOT NULL DEFAULT '0' COMMENT '免费用户0，创业版1，企业版2，旗舰版3',
  `up_time` varchar(32) NOT NULL COMMENT '更新时间',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=8 ;

--
-- 导出表中的数据 `view`
--

INSERT INTO `view` (`id`, `name`, `level`, `up_time`) VALUES
(1, '默认', 0, '2014-05-05 12:21:14'),
(2, '电信', 0, '2014-05-05 12:21:14'),
(3, '联通', 0, '2014-05-05 12:21:14'),
(4, '广东铁通', 0, '2014-05-05 12:21:14'),
(5, '睿江', 1, '2014-05-05 12:21:14'),
(6, '中山广电', 0, '2014-08-12 11:32:45');

-- --------------------------------------------------------

--
-- 表的结构 `view_mask`
--

CREATE TABLE IF NOT EXISTS `view_mask` (
  `id` int(8) NOT NULL AUTO_INCREMENT,
  `view_id` int(8) NOT NULL,
  `view_mask` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- 导出表中的数据 `view_mask`
--


-- --------------------------------------------------------

--
-- 表的结构 `zone`
--

CREATE TABLE IF NOT EXISTS `zone` (
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
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=313 ;

--
-- 导出表中的数据 `zone`
--

INSERT INTO `zone` (`id`, `domain`, `level`, `client_id`, `group_id`, `default_ttl`, `is_author`, `is_on`, `is_star`, `is_lock`, `desc`, `add_time`) VALUES
(295, 'hyb.com', 0, 10, 0, 60, 0, 1, 0, 0, '', '2014-08-12 10:43:39'),
(297, 'mysq.cn', 0, 13, 0, 60, 0, 1, 0, 0, '', '2014-08-14 11:27:34'),
(307, 'lkx.org', 0, 12, 0, 60, 0, 1, 0, 0, '', '2014-08-19 09:44:18'),
(308, 'eflydns.com', 0, 12, 0, 60, 0, 1, 0, 0, '', '2014-08-19 09:46:04'),
(312, 'abc.com', 0, 12, 0, 60, 0, 1, 0, 0, '', '2014-08-19 10:51:11');

-- --------------------------------------------------------

--
-- 表的结构 `zone_level`
--

CREATE TABLE IF NOT EXISTS `zone_level` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `start_time` varchar(32) NOT NULL,
  `end_time` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 AUTO_INCREMENT=2 ;

--
-- 导出表中的数据 `zone_level`
--


-- --------------------------------------------------------

--
-- 表的结构 `zone_name`
--

CREATE TABLE IF NOT EXISTS `zone_name` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `name` varchar(128) NOT NULL COMMENT '别名',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=4 ;

--
-- 导出表中的数据 `zone_name`
--


-- --------------------------------------------------------

--
-- 表的结构 `zone_route`
--

CREATE TABLE IF NOT EXISTS `zone_route` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `route_id` int(16) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- 导出表中的数据 `zone_route`
--


-- --------------------------------------------------------

--
-- 表的结构 `zone_whois`
--

CREATE TABLE IF NOT EXISTS `zone_whois` (
  `id` int(16) NOT NULL AUTO_INCREMENT,
  `zone_id` int(16) NOT NULL,
  `expiration_time` varchar(32) NOT NULL COMMENT '域名到期时间',
  `update_time` varchar(32) NOT NULL COMMENT 'Last update',
  `whois` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 AUTO_INCREMENT=1 ;

--
-- 导出表中的数据 `zone_whois`
--


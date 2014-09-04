<?php

function dnspro_start_log()
{
	openlog("dnspro dnsbroker ", LOG_NDELAY, LOG_LOCAL4); 
}

function dnspro_log($pri, $msg)
{
	syslog($pri, $msg);
}

function dnspro_close_log()
{
	closelog();
}


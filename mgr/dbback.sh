#!/bin/bash

mysqldump -h121.201.12.58 -uroot -prjkj@rjkj --opt -RB dnspro_core |gzip -9 > /media/wdata/linuxshare/db_backup/dnspro_core58-`date +%Y-%m-%d_%H%M%S`.sql.gz
mysqldump -h121.201.12.68 -uroot -prjkj@rjkj --opt -RB dnspro_core |gzip -9 > /media/wdata/linuxshare/db_backup/dnspro_core68-`date +%Y-%m-%d_%H%M%S`.sql.gz

/bin/rm -rf /media/wdata/linuxshare/db_backup/dnspro_core58-`date -d '-1 day' '+%Y-%m-%d'`*
/bin/rm -rf /media/wdata/linuxshare/db_backup/dnspro_core68-`date -d '-1 day' '+%Y-%m-%d'`*

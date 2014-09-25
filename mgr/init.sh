#!/bin/bash
sed -i -r '/MGRHOME|mgrhome|mgrtail|mgrstart|mgrstop|mgrps/'d  /etc/profile
echo -e "export MGRHOME=/usr/local/dnspro\nalias mgrhome=\"cd \$MGRHOME\"\nalias mgrtail=\"tail -f \$MGRHOME/log/dnspro.log\"\
\nalias mgrstart=\"sh \$MGRHOME/start.sh\"\nalias mgrstop=\"sh \$MGRHOME/stop.sh\"\nalias mgrps=\"sh \$MGRHOME/psmgr.sh\"\
\nalias mgrchlog=\"sh \$MGRHOME/chlog.sh\"" >> /etc/profile

sleep 1

source /etc/profile
/bin/rm *.pyc -fr
python -c "import compileall; compileall.compile_dir('.')"
/bin/mv mgr_conf.py /etc/ -f
/bin/rm *.py -fr
/bin/mv /etc/mgr_conf.py . -f
/bin/rm init.sh -f

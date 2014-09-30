#!/bin/bash
# made by likunxiang

sleep 1

/bin/rm *.pyc -fr
python -c "import compileall; compileall.compile_dir('.')"
/bin/mv mgr_conf.py /etc/ -f
/bin/rm *.py -fr
/bin/mv /etc/mgr_conf.py . -f
/bin/rm init.sh -f

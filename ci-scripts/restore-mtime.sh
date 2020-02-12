
#!/bin/bash
git clone https://github.com/MestreLion/git-tools.git
export PATH="$(pwd)/git-tools:$PATH"
git config --local diff.renameLimit 999999
git config --local merge.renameLimit 999999
git-restore-mtime
LAST_PATH=$(pwd)
#for d in ./packages/*; do
#  if [ -f "${d}/.git" ]; then
#    (
#      cd "$d"
##      git config --local diff.renameLimit 999999
##      git config --local merge.renameLimit 999999
#      git-restore-mtime
#    )
#  fi
#done
cd $LAST_PATH

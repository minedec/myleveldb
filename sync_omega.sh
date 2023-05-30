# GLOBIGNORE='build'
# scp -r ./* jjd@10.0.1.92:/home/jjd/myleveldb
rsync -av --exclude 'build' --exclude 'build.sh' ./* jjd@10.0.1.92:/home/jjd/myleveldb
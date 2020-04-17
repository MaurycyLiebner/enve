cd build
export GREP_PATH=grep;
curl -s -H "Authorization: token $GITHUB_TOKEN" https://api.github.com/repos/MaurycyLiebner/enve/commits/master -o repo.txt;
export REMOTE=$($GREP_PATH -Po '(?<=: \")(([a-z0-9])\w+)(?=\")' -m 1 repo.txt);
export LOCAL=$(git rev-parse HEAD);
if [[ "$REMOTE" != "$LOCAL" ]]; then echo "Build no longer current. $REMOTE vs $LOCAL - aborting upload."; exit 0; fi;
wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh;
export UPLOADTOOL_SUFFIX="linux";
bash upload.sh enve*.AppImage*;
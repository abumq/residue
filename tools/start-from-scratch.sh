BASE_URL=$1

if [ "$BASE_URL" == "" ];then
	echo "Base URL not provided"
	exit;
fi

mkdir residue
mkdir residue/configurations
wget https://github.com/muflihun/ripe/releases/download/v3.3.0/ripe-3.3.0-x86_64-linux.tar.gz
tar xf ripe-3.3.0-x86_64-linux.tar.gz
rm ripe-3.3.0-x86_64-linux.tar.gz
mv ripe-3.3.0-x86_64-linux/* residue/
rm -r ripe-3.3.0-x86_64-linux
cd residue/
wget $BASE_URL/residue
wget $BASE_URL/residue.conf.json
wget $BASE_URL/libboost_system.so.1.60.0
wget -O configurations/default-Logger.conf $BASE_URL/configurations/default-Logger.conf
wget -O configurations/residue-Logger.conf $BASE_URL/configurations/residue-Logger.conf
chmod +x residue ripe
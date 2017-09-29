# Usage: package.sh darwin

CURR_DIR=${PWD##*/}

if [ "$CURR_DIR" != "build" ];then
	echo "Run this script from 'build' directory"
	exit;
fi

TYPE=$1
VERSION=$2

if [ "$TYPE" = "" ] || [ "$VERSION" = "" ];then
	echo "Usage: $0 <type> version>"
	echo "  example: $0 darwin $RIPE_VERSION"
	exit;
fi

PACK=residue-$VERSION-x86_64-$TYPE

if [ -d "$PACK" ];then
	echo "$PACK already exist. Remove $PACK first"
	exit;
fi


cmake -DCMAKE_BUILD_TYPE=Release -Duse_mine=OFF -Ddebug=OFF -Dproduction=ON ..
make

echo "Creating $PACK.tar.gz ..."
mkdir $PACK
cp residue-$VERSION $PACK/residue
cp residue-config-validator-$VERSION $PACK/residue-config-validator
cp residue-license-manager-$VERSION $PACK/residue-license-manager

ls -lh $PACK

tar cfz $PACK.tar.gz $PACK
rm -rf $PACK
shasum $PACK.tar.gz
echo `pwd`/$PACK.tar.gz

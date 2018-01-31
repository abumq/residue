# Usage: package.sh darwin

CURR_DIR=${PWD##*/}

if [ "$CURR_DIR" != "build" ];then
    echo "Run this script from 'build' directory"
    exit;
fi

STRIP=strip
TYPE=$1
VERSION=$2

if [ "$SHASUM" = "" ];then
    export SHASUM="shasum"
fi

if [ "$TYPE" = "" ] || [ "$VERSION" = "" ];then
    echo "Usage: $0 <type> version>"
    echo "  example: $0 darwin $RIPE_VERSION"
    exit;
fi

if [ `grep -o ' -O0 ' ../CMakeLists.txt -c` != "0" ];then
    echo "Error: Optimization not reset"
    exit;
fi

PACK=residue-$VERSION-x86_64-$TYPE

if [ -d "$PACK" ];then
    echo "Error: $PACK already exist. Remove $PACK first"
    exit;
fi


cmake -DCMAKE_BUILD_TYPE=Release -Duse_mine=OFF -Ddebug=OFF -Dprofiling=OFF -Dproduction=ON ..
make

echo "Creating $PACK.tar.gz ..."
mkdir $PACK
cp residue-$VERSION $PACK/residue
cp residue-config-validator-$VERSION $PACK/residue-config-validator
# cp residue-license-manager-$VERSION $PACK/residue-license-manager
# $STRIP $PACK/residue-license-manager
$STRIP $PACK/residue-config-validator
$STRIP $PACK/residue

ls -lh $PACK

tar cfz $PACK.tar.gz $PACK
rm -rf $PACK
$SHASUM $PACK.tar.gz
echo `pwd`/$PACK.tar.gz

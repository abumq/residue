# Usage: package.sh darwin

CURR_DIR=${PWD##*/}

if [ "$CURR_DIR" != "build" ];then
    echo "Run this script from 'build' directory"
    exit;
fi

STRIP=strip
TYPE=$1
VERSION=$2

DEBUG_VERSION=$3
if [ "$DEBUG_VERSION" = "" ];then
    DEBUG_VERSION="OFF"
fi

if [ "$SHASUM" = "" ];then
    export SHASUM="shasum"
fi


if [ "$TYPE" = "" ] || [ "$VERSION" = "" ];then
    echo "Usage: $0 <type> <version> <debug=OFF>"
    echo "  example: $0 darwin 1.2.3 OFF OFF"
    exit;
fi

PACK=residue-$VERSION-$TYPE-x86_64

if [ -d "$PACK" ];then
    echo "Error: $PACK already exist. Remove $PACK first"
    exit;
fi


cmake -DCMAKE_BUILD_TYPE=Release -Duse_mine=OFF -Ddebug=$DEBUG_VERSION -Dprofiling=OFF -Dproduction=ON ..
make -j4

echo "Creating $PACK.tar.gz ..."
mkdir $PACK
cp residue-$VERSION $PACK/residue
cp residue-config-validator-$VERSION $PACK/residue-config-validator
cp libresidue-extension-st.a $PACK/libresidue-extension-st.$VERSION.a
cp libresidue-extension.$VERSION.* $PACK/
cp -r ../dist/pkg/* $PACK/

$STRIP $PACK/residue-config-validator
$STRIP $PACK/residue

ls -lh $PACK

tar cfz $PACK.tar.gz $PACK
rm -rf $PACK
$SHASUM $PACK.tar.gz
echo `pwd`/$PACK.tar.gz

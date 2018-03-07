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

EXTENSIONS=$4
if [ "$EXTENSIONS" = "" ];then
    EXTENSIONS="OFF"
fi

if [ "$SHASUM" = "" ];then
    export SHASUM="shasum"
fi


if [ "$TYPE" = "" ] || [ "$VERSION" = "" ];then
    echo "Usage: $0 <type> <version> <debug=OFF> <enable_extensions=OFF>"
    echo "  example: $0 darwin 1.2.3 OFF OFF"
    exit;
fi

PACK=residue-$VERSION-$TYPE-x86_64
PACK_EXT=residue-extensions-$VERSION-$TYPE-x86_64

if [ -d "$PACK" ];then
    echo "Error: $PACK already exist. Remove $PACK first"
    exit;
fi

if [ "$EXTENSIONS" = "ON" ];then
    if [ -d "$PACK_EXT" ];then
    echo "Error: $PACK_EXT already exist. Remove $PACK_EXT first"
    exit;
    fi
fi


cmake -DCMAKE_BUILD_TYPE=Release -Duse_mine=OFF -Denable_extensions=$EXTENSIONS -Ddebug=$DEBUG_VERSION -Dprofiling=OFF -Dproduction=ON ..
make -j4

echo "Creating $PACK.tar.gz ..."
mkdir $PACK
cp residue-$VERSION $PACK/residue
cp residue-config-validator-$VERSION $PACK/residue-config-validator
cp -r ../dist/pkg/* $PACK/

$STRIP $PACK/residue-config-validator
$STRIP $PACK/residue

ls -lh $PACK
tar cfz $PACK.tar.gz $PACK
rm -rf $PACK
$SHASUM $PACK.tar.gz
echo `pwd`/$PACK.tar.gz

if [ "$EXTENSIONS" = "ON" ];then
    echo "Creating $PACK_EXT.tar.gz ..."
    mkdir $PACK_EXT
    mkdir $PACK_EXT/include
    mkdir $PACK_EXT/include/residue/
    mkdir $PACK_EXT/include/residue/extensions/
    mkdir $PACK_EXT/include/residue/core/
    mkdir $PACK_EXT/include/residue/gason/
    cp libresidue-extension-st.a $PACK_EXT/libresidue-extension-st.$VERSION.a
    cp libresidue-extension.$VERSION.* $PACK_EXT/
    cp ../include/extensions.h $PACK_EXT/include/residue/
    cp ../src/extensions/extension.h $PACK_EXT/include/residue/extensions/
    cp ../src/extensions/log-extension.h $PACK_EXT/include/residue/extensions/
    cp ../src/extensions/pre-archive-extension.h $PACK_EXT/include/residue/extensions/
    cp ../src/extensions/post-archive-extension.h $PACK_EXT/include/residue/extensions/
    cp ../src/core/json-doc.h $PACK_EXT/include/residue/core/
    cp ../deps/gason/gason.h $PACK_EXT/include/residue/gason/

    ls -lh $PACK_EXT
    tar cfz $PACK_EXT.tar.gz $PACK_EXT
    rm -rf $PACK_EXT
    $SHASUM $PACK_EXT.tar.gz
    echo `pwd`/$PACK_EXT.tar.gz
fi

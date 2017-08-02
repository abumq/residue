# Usage: package.sh darwin $RIPE_VERSION

CURR_DIR=${PWD##*/}

if [ "$CURR_DIR" != "build" ];then
	echo "Run this script from 'build' directory"
	exit;
fi

TYPE=$1
VERSION=$2
RIPE_VERSION=3.3.0

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


cmake -DCMAKE_BUILD_TYPE=Release -Ddebug=OFF -Dproduction=ON ..
make

mkdir $PACK
cp residue-$VERSION $PACK/residue

if [ -f "libresidue.$VERSION.dylib" ];then
	cp libresidue.$VERSION.dylib $PACK/
	cp /usr/local/lib/libboost_system-mt.dylib $PACK/
	cp /usr/local/lib/libripe.3.2.2.dylib $PACK/
	cd $PACK
	ln -s libresidue.$VERSION.dylib libresidue.dylib
	ln -s libripe.$RIPE_VERSION.dylib libripe.dylib
elif [ -f "libresidue.so.$VERSION" ];then
	cp libresidue.so.$VERSION $PACK/
	cp /usr/lib64/libstdc++.so.6.0.22 $PACK/
	cp /usr/lib64/libboost_system.so.1.60.0 $PACK/
	cp /usr/local/lib/libripe.so.$RIPE_VERSION $PACK/
	cd $PACK
	ln -s libresidue.so.$VERSION libresidue.so
	ln -s libstdc++.so.6.0.22 libstdc++.so.6
fi
cd ..

ls -lh $PACK

tar zcf $PACK.tar.gz $PACK
rm -rf $PACK
shasum $PACK.tar.gz
echo `pwd`/$PACK.tar.gz

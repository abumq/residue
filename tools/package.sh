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
PACK_DEV=residue-client-$VERSION-86_64-$TYPE

if [ -d "$PACK" ];then
	echo "$PACK already exist. Remove $PACK first"
	exit;
fi
if [ -d "$PACK_DEV" ];then
	echo "$PACK_DEV already exist. Remove $PACK_DEV first"
	exit;
fi


cmake -DCMAKE_BUILD_TYPE=Release -Ddebug=OFF -Dproduction=ON ..
make

mkdir $PACK
mkdir $PACK_DEV
cp residue-$VERSION $PACK/residue

if [ -f "libresidue.$VERSION.dylib" ];then
	cp libresidue.$VERSION.dylib $PACK_DEV/
	cd $PACK_DEV
	ln -s libresidue.$VERSION.dylib libresidue.dylib
	
	#cp /usr/local/lib/libboost_system-mt.dylib $PACK/
	cp /usr/local/lib/libripe.3.2.2.dylib $PACK/
	ln -s libripe.$RIPE_VERSION.dylib libripe.dylib
elif [ -f "libresidue.so.$VERSION" ];then
	cp libresidue.so.$VERSION $PACK/
	cp /usr/lib64/libstdc++.so.6.0.22 $PACK/
	#cp /usr/lib64/libboost_system.so.1.60.0 $PACK/
	cp /usr/local/lib/libripe.so.$RIPE_VERSION $PACK/
	cd $PACK
	ln -s libresidue.so.$VERSION libresidue.so
	ln -s libstdc++.so.6.0.22 libstdc++.so.6
fi
cd ..

echo "Server package"
ls -lh $PACK
echo "Client lib package"
ls -lh $PACK_DEV

#tar zcf $PACK.tar.gz $PACK
#rm -rf $PACK
#rm -rf $PACK_DEV
#shasum $PACK.tar.gz
#echo `pwd`/$PACK.tar.gz

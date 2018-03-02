CURR_DIR=${PWD##*/}

if [ "$CURR_DIR" != "npm" ];then
    echo "Run this script from 'dist/npm' directory"
    exit;
fi

DAR=`uname -a | grep 'Darwin' | wc -c | grep -o '[0-9]'`
TYPE="darwin"
if [ "$DAR" = "0" ];then
    TYPE="linux"
fi
if [ "$1" != "" ];then
    TYPE="$1"
fi

MAJOR=`grep 'RESIDUE_MAJOR' ../../CMakeLists.txt | grep -o [0-9] | tr -d '\n'`
MINOR=`grep 'RESIDUE_MINOR' ../../CMakeLists.txt | grep -o [0-9] | tr -d '\n'`
PATCH=`grep 'RESIDUE_PATCH' ../../CMakeLists.txt | grep [0-9] | awk '{print $3}' | cut -d "\"" -f2`
VERSION="$MAJOR.$MINOR.$PATCH"

echo "Publish $VERSION on $TYPE to NPM. Continue (y/n)?"

read confirm
if [ "$confirm" = "y" ]; then
    if [ ! -f ../../build/residue-$VERSION-$TYPE-x86_64.tar.gz ];then
        echo "Could not find residue-$VERSION-$TYPE-x86_64.tar.gz in build directory"
        exit 1;
    fi
    tar -xf ../../build/residue-$VERSION-$TYPE-x86_64.tar.gz -C $TYPE/
    mv $TYPE/residue-$VERSION-$TYPE-x86_64/residue $TYPE/residued
    rm -rf $TYPE/residue-$VERSION-$TYPE-x86_64
    echo "---- CONTENTS ------"
    ls -l $TYPE/
    echo "---- RESIDUE ------"
    $TYPE/./residued --version
    echo "---- package.json ------"
    grep name $TYPE/package.json
    grep version $TYPE/package.json
    echo "---- npm whoami ----- (please wait...)"
    npm whoami
    echo "Do above contents look good? Continue (y/n)?"
    read good
    if [ "$good" = "y" ];then
        cd $TYPE/
        npm publish
        cd ../
    fi
fi

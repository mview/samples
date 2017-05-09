build for android 5.0
export PATH=~/android-ndk-r10e:$PATH
NDK_EXT_HOME=/opt/ndk-ext VER=5.0 DIR=wcap /opt/ndk-ext/build/build.sh

clear
DIR=wcap /opt/ndk-ext/build/clean.sh 

install 
adb push wcap/lua-services/env-fs/ /data/local/tmp
adb push wcap/libs/<arch>/ /data/local/tmp

in android device run
./wcap

in web brower access
http://<android device ip>:8000

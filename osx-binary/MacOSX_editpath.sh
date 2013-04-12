#!/bin/sh
#
# changes path for libfreenect in pix_freenect.pd_darwin and libfreenect.0.0.1.dylib

install_name_tool -id ola2pd.pd_darwin ola2pd.pd_darwin
install_name_tool -change /opt/local/lib/libola.1.dylib @loader_path/libola.1.dylib ola2pd.pd_darwin


install_name_tool -id @loader_path/libola.1.dylib libola.1.dylib
install_name_tool -change /opt/local/lib/libolacommon.0.dylib @loader_path/libolacommon.0.dylib libola.1.dylib
install_name_tool -change /opt/local/lib/libprotobuf.7.dylib @loader_path/libprotobuf.7.dylib libola.1.dylib
install_name_tool -change /opt/local/lib/libz.1.dylib @loader_path/libz.1.dylib libola.1.dylib

install_name_tool -id @loader_path/libolacommon.0.dylib libolacommon.0.dylib
install_name_tool -change /opt/local/lib/libprotobuf.7.dylib @loader_path/libprotobuf.7.dylib libolacommon.0.dylib
install_name_tool -change /opt/local/lib/libz.1.dylib @loader_path/libz.1.dylib libolacommon.0.dylib

install_name_tool -id @loader_path/libprotobuf.7.dylib libprotobuf.7.dylib
install_name_tool -change /opt/local/lib/libz.1.dylib @loader_path/libz.1.dylib libprotobuf.7.dylib

install_name_tool -id @loader_path/libz.1.dylib libz.1.dylib




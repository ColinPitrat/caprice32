# The idea of this test is to ensure no new DLL dependency is added involuntarly.
# All needed DLLs should be included in the archive.
#
# If the test fails, the package needs to be verified manually under windows or
# wine to see if a DLL needs to be added to it. If so, it needs to be added to
# the DLLS variable in makefile.
ARCH=$1
ACTUAL=$(mktemp)
EXPECTED=$(mktemp)

cat << EOF > ${EXPECTED}
libbrotlicommon.dll
libbrotlidec.dll
libbz2-1.dll
libfreetype-6.dll
libgcc_s_seh-1.dll
libglib-2.0-0.dll
libgraphite2.dll
libharfbuzz-0.dll
libiconv-2.dll
libintl-8.dll
libpcre-1.dll
libpng16-16.dll
libstdc++-6.dll
libwinpthread-1.dll
SDL.dll
zlib1.dll
EOF

ldd cap32.exe | grep mingw | sed 's/ =>.*//' | sed 's/ *//' | sort > ${ACTUAL}

if ! diff ${ACTUAL} ${EXPECTED}
then
  echo "!! dependencies test for ${ARCH} failed"
  cat ${LOGFILE}
  exit 1
else
  echo "The test was successful!"
  cat printer.dat
  cat ${LOGFILE}
fi

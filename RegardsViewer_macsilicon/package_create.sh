if [ -d package ]; then echo "Exists"; else mkdir package;fi
cp -r ./Release/RegardsLiteViewer.app ./package/RegardsLiteViewer/
rm -f ./package/RegardsLiteViewer.dmg
echo "Create Package"
hdiutil create -fs HFS+ -srcfolder ./package/RegardsLiteViewer -volname RegardsLiteViewer ./package/RegardsLiteViewer.dmg
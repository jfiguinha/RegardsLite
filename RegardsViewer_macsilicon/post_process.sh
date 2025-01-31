#/bin/bash
./MacOsLibExplorer ./Release/RegardsLiteViewer.app/Contents/MacOS ./Release/RegardsLiteViewer.app/Contents/Frameworks change_dylib.sh 1
./MacOsLibExplorer ./Release/RegardsLiteViewer.app/Contents/Frameworks ./Release/RegardsLiteViewer.app/Contents/Frameworks change_dylib.sh 1
codesign -fs - ./Release/RegardsLiteViewer.app --deep

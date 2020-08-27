 emconfigure cmake -DCMAKE_BUILD_TYPE=Debug $1 . && emmake make
 emcc libAWoWWebViewerJs.a ./A/libWoWViewerLib.a --pre-js prejs.js --js-library jsLib.js --source-map-base "/" -s USE_GLFW=3 --preload-file glsl -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 -O0 -g4 -s ALLOW_MEMORY_GROWTH=1 -s FETCH=1 -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s "EXPORTED_FUNCTIONS=['_createWebJsScene', '_gameloop', '_createWoWScene', '_setScene', '_setSceneSize', '_setSceneFileDataId', '_setNewUrls', '_addCameraViewOffset', '_addHorizontalViewDir', '_addVerticalViewDir', '_zoomInFromMouseScroll', '_startMovingForward', '_startMovingBackwards', '_stopMovingForward', '_stopMovingBackwards']" -o project.js
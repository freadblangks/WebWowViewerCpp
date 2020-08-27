emconfigure cmake -DCMAKE_BUILD_TYPE=Release $1 && emmake make VERBOSE=1 -s ALLOW_MEMORY_GROWTH=1
emcc libAWoWWebViewerJs.a ./A/libWoWViewerLib.a --pre-js prejs.js --js-library jsLib.js -s USE_GLFW=3 --preload-file glsl -O3 -s ALLOW_MEMORY_GROWTH=1 -s FETCH=1 -s WASM=1 -s USE_WEBGL2=1 -s FULL_ES3=1 -s "EXPORTED_FUNCTIONS=['_createWebJsScene', '_gameloop', '_createWoWScene', '_setScene', '_setSceneSize', '_setSceneFileDataId', '_setNewUrls', '_addCameraViewOffset', '_addHorizontalViewDir', '_addVerticalViewDir', '_zoomInFromMouseScroll', '_startMovingForward', '_startMovingBackwards', '_stopMovingForward', '_stopMovingBackwards']" -o project.js

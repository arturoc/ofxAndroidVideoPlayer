Video player addon for android based on ffmpeg, this probably won't be official since the libraries are too big for an android application. To use it:

- download to the addons folder
- add the name of the addons in the addons.make file of the project
- create a normal ofVideoPlayer and pass an ofxAndroidVideoPlayer like:

  ~~~~{.cpp}
  player.setPlayer(ofPtr<ofxAndroidVideoPlayer>(new ofxAndroidVideoPlayer))
  ~~~~

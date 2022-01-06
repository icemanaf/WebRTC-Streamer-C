# WebRTC Streamer - using LibDatachannel and GStreamer

This is the C implementation.Has been tested on a Raspberry Pi Zero.

## Dependencies
install the GStreamer dependencies
```
apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev \
gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl \
gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio
```
If you want to use a Raspberry Pi, you need to do the following as well
```
sudo modprobe -r bcm2835_v4l2
sudo modprobe bcm2835_v4l2 gst_v4l2src_is_broken=1
sudo apt-get install gstreamer1.0-omx-rpi gstreamer1.0-omx
```

## Building

Build LibDatachannel.




##Useage

Run the app and paster the offer in the [sfiddle](https://jsfiddle.net/y76Ljqms/29)
Retrieve the answer and paste it back in the application.
You should now be able to see the video playing.




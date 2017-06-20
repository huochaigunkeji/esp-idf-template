# 2017.6.6 #
1.添加smartconfig功能，上电启动smartconfig功能。
# 2017.6.7 #
1.将smartconfig启动改为按键触发，按下GPIO0对应的按键5s，设备进入smartconfig功能。  
# 2017.6.8 #
1.设备启动后，检测是否保存有AP的信息，如果有，则自动连接AP。  
2.添加 UDP Server线程，端口：6666，用于在局域网下查找设备，局域网的udp客户端广播find ESP32消息，设备收到此消息后回复I'm a ESP32 device,MAC=XXXXXXXXXXXX，"XXXXXXXXXXXX"是设备的MAC地址。
# 2017.6.9 #
1.添加TCP Server线程，端口：88，局域网Tcp客户端连接上设备后，发送什么消息给设备，设备就回复什么消息。  
# 2017.6.20 #
1.将用户LED又GPIO25改为GPIO4，和ESP32S IOT EB V1.0一致。
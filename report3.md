系统日志截图

![logging](http://ww4.sinaimg.cn/mw690/bd615ff8gw1f5ff4wzwvcj211v0bzdqh.jpg)

Wireshark截图

![wireshark](http://ww2.sinaimg.cn/mw690/bd615ff8gw1f5gjdy1r9xj20y60igjxb.jpg)

PyViz截图，网络由P2P以及csma组成

![PyViz](http://ww3.sinaimg.cn/mw690/bd615ff8gw1f5gh9vydg5j21170g1dlv.jpg)

PyViz安装步骤

电脑安装的系统为fedora 23

1.安装依赖包

yum install python-devel gnome-python2 gnome-python2-gnomedesktop gnome-python2-rsvg graphviz-python pygoocanvas python-kiwi

2.安装安装交互python

yum install ipython

3.测试

./waf --pyrun src/flow-monitor/examples/wifi-olsr-flowmon.py --vis

参考资料：https://www.nsnam.org/wiki/index.php/PyViz


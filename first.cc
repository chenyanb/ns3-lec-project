/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");//声明了一个叫FirstScriptExample的日志构件，通过引用FirstScriptExample这个名字的操作，
//可以实现打开或者关闭控制台日志的输出。

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc,argv);
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);  //下面两行脚本是用来使两个日志组件生效的。它们被内建在Echo Client 和Echo Server 应用中
  //设置日志为LOG_LEVEL_INFO级别
////在不改变脚本的前提下，也可通过在命令行中输入export NS_LOG=UdpEchoClientApplication=level_all；第二个等号左边：想要设置的日志组件名称
//等号右边：要用的日志级别
  NS_LOG_INFO("Creating Topology");   
  NS_LOG_WARN("Message:level_warn");
  NS_LOG_INFO("Message:level_info");
  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  AsciiTraceHelper ascii;           //创建一个ASCII trace对象
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("cyb.tr"));//包含两个方法调用。
  pointToPoint.EnablePcapAll ("first");
//CreateFileStream()用未命名的对象在协议栈中创建了一个文件流，并把这个文件流传递给了调用方法，即创建了一个对象代表着一个名为“first.tr”的文件，并传递给了NS3。

//EnableAsciiAll()告诉helper你想要将ASCII tracing安装在仿真中的点到点设备上，并且你想要接收端以ASCII格式写出数据包移动信息。
/*   这两行代码用来打开一个将被写入名为“first.tr”文件中的数据流。代码段中的第二行告诉NS3在仿真中为所有点到底设备启用ASCII tracing功能，并且你想要用ASCII格式来写出数据流中数据包的移动信息。类似于NS2中，trace事件和一般的trace“+”、“-”、“d”、“r”事件。  */
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

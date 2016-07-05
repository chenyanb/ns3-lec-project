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
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

// Default Network Topology
//网络拓扑
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   CSMA2 10.1.3.0
//                      AP
//  *      *      *     *
//  |      |      |     |      10.1.1.0
//  n5     n6     n7    n0 ------------------ n1   n2   n3   n4
//                          point-to-point     |    |    |    |
//                                                   ================
//                                                     csma1    LAN 10.1.2.0


////////////////////////////////n0,n1,n2,n3,n4,n5,n6,n7均为为csma节点；同时n0,n1为点对点节点，n0为AP
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");		//定义记录组件

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t ncsma1 = 3,ncsma2=3;			//csma节点数量
  bool tracing = true;


  CommandLine cmd;
  cmd.AddValue ("nCsma1", "Number of \"extra\" CSMA nodes/devices", ncsma1);
  cmd.AddValue ("nCsma2", "Number of \"extra\" CSMA nodes/devices", ncsma2);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  //判断是否超过了250个，超过报错
  if ((ncsma1 > 250)|(ncsma2 > 250))
    {
      std::cout << "Too many  csma nodes, no more than 250 each." << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);	//启动记录组件
    }


  //创建2个节点，p2p链路两端，两个P2P
  NodeContainer p2pNodes;// NodeContainer 创建节点的类
  p2pNodes.Create (2);

  //创建信道，设置信道参数，在设备安装到节点上
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;///创建节点后，创建设备
  p2pDevices = pointToPoint.Install (p2pNodes);//////////把节点安装到P2P链路两端，，节点安装到设备

  //创建csma节点，包含一个p2p节点

  NodeContainer csma1Nodes;
  csma1Nodes.Add (p2pNodes.Get (1));
  csma1Nodes.Create (ncsma1);

  NodeContainer csma2Nodes;
  csma2Nodes.Add (p2pNodes.Get (0));
  csma2Nodes.Create (ncsma2);

  //创建信道，设置信道参数，在设备安装到节点上
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csma1Devices;
  csma1Devices = csma.Install (csma1Nodes);

  NetDeviceContainer csma2Devices;
  csma2Devices = csma.Install (csma2Nodes);

  //配置协议栈
  InternetStackHelper stack;
  stack.Install (csma1Nodes);//////n1在安装CSMA时已经安装，n1属于CSMA
  stack.Install (csma2Nodes);  

  //分配IP地址
  Ipv4AddressHelper address;
 //P2P信道
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
 //csma信道
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csma1Interfaces;
  csma1Interfaces = address.Assign (csma1Devices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer csma2Interfaces;
  csma2Interfaces = address.Assign (csma2Devices);
  //放置echo服务端程序在最右边的csma节点,端口为9
  UdpEchoServerHelper echoServer (19);
  ApplicationContainer serverApps = echoServer.Install (csma1Nodes.Get (ncsma1-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));/////////////1 秒开始10秒结束

  //指向CSMA网络的服务器，上面的节点地址，端口为19
  UdpEchoClientHelper echoClient (csma1Interfaces.GetAddress (ncsma1-1),19);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  //安装其他节点应用程序
  ApplicationContainer clientApps[4]= 
 { echoClient.Install (csma1Nodes.Get (ncsma1 - 2)),echoClient.Install (csma1Nodes.Get (ncsma1 - 3)),echoClient.Install (csma2Nodes.Get (ncsma2 - 1)),
echoClient.Install (csma2Nodes.Get (ncsma2 - 2))};
for(int i=0;i<4;i++)
 { 
  clientApps[i].Start (Seconds (i+2));
  clientApps[i].Stop (Seconds (10.0));
                                         }

  //启动互联网络路由
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  AsciiTraceHelper ascii;           //创建一个ASCII trace对象
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("project3-ptp.tr"));//包含两个方法调用。
  pointToPoint.EnablePcapAll ("project3-ptp");
  csma.EnableAsciiAll(ascii.CreateFileStream("project3-csma.tr"));//包含两个方法调用。
  csma.EnablePcapAll ("project3-csma");

/*CreateFileStream()用未命名的对象在协议栈中创建了一个文件流，并把这个文件流传递给了调用方法，即创建了一个对象代表着一个名为“first.tr”的文件，并传递给了NS3。

  EnableAsciiAll()告诉helper你想要将ASCII tracing安装在仿真中的点到点设备上，并且你想要接收端以ASCII格式写出数据包移动信息。
   这两行代码用来打开一个将被写入名为“first.tr”文件中的数据流。代码段中的第二行告诉NS3在仿真中为所有点到底设备启用ASCII tracing功能，并且你想要用ASCII格式来写出数据流中数据包的移动信息。类似于NS2中，trace事件和一般的trace“+”、“-”、“d”、“r”事件。  */

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

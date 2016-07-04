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
//   WIFI 10.1.2.0
//                                            AP
//  *      *      *     *
//  |      |      |     |                     *
//  n5     n6     n7    n0 ------------------ n1   n2   n3   n4
//                                             |    |    |    |
//                                                   ================
//                                                       csma 10.1.3.0


////////////////////////////////n1,n2,n3,n4为csma节点；n0,n1,n5,n6,n7为WIFI节点；
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");		//定义记录组件

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t ncsma= 4,nWifi=4;			//csma节点数量
  bool tracing = true;


  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", ncsma);
  cmd.AddValue ("nwifi", "Number of \"extra\" wifi nodes/devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  //判断是否超过了250个，超过报错 
  if ((ncsma > 250)|(nWifi > 250))  
    {
      std::cout << "Too many wifi or csma nodes, no more than 250 each." << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);	//启动记录组件
    }

  //创建csma节点

  NodeContainer csmaNodes;
  csmaNodes.Create (ncsma);
  //创建信道，设置信道参数，在设备安装到节点上
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);
///////////////////wifi节点
  NodeContainer wifiStaNodes;
  NodeContainer wifiApNode = csmaNodes.Get(0);
  wifiStaNodes.Create (nWifi);////3个WIFI再加上一个csma节点（n0）
  
 
  YansWifiChannelHelper channel= YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  //配置速率控制算法，AARF算法
  WifiHelper wifi= WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  //配置mac类型为sta模式，不发送探测请求
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  //创建无线设备，将mac层和phy层安装到设备上
  NetDeviceContainer staDevices;
  staDevices= wifi.Install(phy, mac, wifiStaNodes);

  //配置AP节点的mac层为AP模式，创建AP设备
  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices= wifi.Install (phy, mac, wifiApNode);

///////////移动模型
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (wifiStaNodes);
  for (uint n = 0; n < wifiStaNodes.GetN(); n++)
  {
    Ptr<ConstantVelocityMobilityModel> mob = wifiStaNodes.Get(n)->GetObject<ConstantVelocityMobilityModel>();
    mob->SetPosition(Vector(80.0, 50.0, 0.0));
    mob->SetVelocity(Vector(-(n/2.0+0.5), 0.0, 0.0));   

  }
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  Ptr<ConstantPositionMobilityModel> mob = wifiApNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
  mob->SetPosition(Vector(80.0, 50.0, 0.0)); 
/////////////////////协议栈
  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiStaNodes);



  //分配IP地址
  Ipv4AddressHelper address;
 //csma信道
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);
//////wifi 信道
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);
  //放置echo服务端程序在最右边的csma节点,端口为9
  UdpEchoServerHelper echoServer (19);
  ApplicationContainer serverApps = echoServer.Install (csmaNodes.Get (ncsma-1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));/////////////1 秒开始10秒结束

  //指向CSMA网络的服务器，上面的节点地址，端口为19
  UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (ncsma-1),19);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  //安装其他节点应用程序
  ApplicationContainer clientApps[4]= 
 { echoClient.Install (csmaNodes.Get (ncsma-2)),echoClient.Install (csmaNodes.Get (ncsma-3)),echoClient.Install (csmaNodes.Get (ncsma-4)),
  echoClient.Install (wifiStaNodes.Get (nWifi-2))};
  for(int i=0;i<4;i++)
 { 
  clientApps[i].Start (Seconds (i+2));
  clientApps[i].Stop (Seconds (10.0));
                                         }

  //启动互联网络路由
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));
 if (tracing == true)
{
  // pointToPoint.EnablePcapAll ("ns3-lec-project2");
  // csma.EnablePcapAll("ns3-lec-project2-csma");
  csma.EnablePcapAll("ns3-lec-project2-2-csma");     //指定节点 
  phy.EnablePcapAll("ns3-lec-project2-2-wifi");                                                         
                                                                               }

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

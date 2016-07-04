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
//默认网络拓扑
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   Wifi1 10.1.3.0
//                                             AP                     AP
//                   *     *    *    *    *     *                     *
//                   |     |    |    |    |     |      10.1.1.0
//                   n14  n13  n12   n10   n9   n8 ------------------ n1   n2   n3   n4  n5  n6  n7
//                                                    point-to-point       
//                                                                                ================
//                                                                             WIFI2


////////////////////////////////
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");		//定义记录组件

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi1 = 6,nWifi2= 6;				//wifi节点数量

  CommandLine cmd;
  cmd.AddValue ("nWifi1", "Number of wifi STA devices", nWifi1);
  cmd.AddValue ("nWifi2", "Number of wifi STA devices", nWifi2);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);


  cmd.Parse (argc,argv);
		                 //判断是否超过了250个，超过报错 , 原因？
  if ((nWifi1> 250)||(nWifi2 > 250))
    {
      std::cout << "Too many wifi or csma nodes, no more than 250 each." << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);	//启动记录组件
    }


  //创建2个节点，p2p链路两端，1个P2P
  NodeContainer p2pNodes;// NodeContainer 创建节点的类
  p2pNodes.Create (2);

  //创建信道，设置信道参数，在设备安装到节点上
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;///创建节点后，创建设备
  p2pDevices = pointToPoint.Install (p2pNodes);//////////把节点安装到P2P链路两端，，节点安装到设备
//第一个WIFI网络
//创建wifista无线终端，AP接入点
  NodeContainer wifi1StaNodes;
  NodeContainer wifi1ApNode = p2pNodes.Get(0);
  wifi1StaNodes.Create (nWifi1);////3个WIFI再加上一个P2P节点（n0）
  
 
  YansWifiChannelHelper channel1 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy1 = YansWifiPhyHelper::Default ();
  phy1.SetChannel (channel1.Create ());

  //配置速率控制算法，AARF算法
  WifiHelper wifi1 = WifiHelper::Default ();
  wifi1.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac1 = NqosWifiMacHelper::Default ();

  //配置mac类型为sta模式，不发送探测请求
  Ssid ssid1 = Ssid ("ns-3-ssid");
  mac1.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid1),
               "ActiveProbing", BooleanValue (false));

  //创建无线设备，将mac层和phy层安装到设备上
  NetDeviceContainer staDevices1;
  staDevices1= wifi1.Install(phy1, mac1, wifi1StaNodes);

  //配置AP节点的mac层为AP模式，创建AP设备
  mac1.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid1));

  NetDeviceContainer apDevices1;
  apDevices1 = wifi1.Install (phy1, mac1, wifi1ApNode);
 
  //第二个WIFI网络
//创建wifista无线终端，AP接入点
  NodeContainer wifi2StaNodes;
  wifi2StaNodes.Create (nWifi2);////3个WIFI再加上一个P2P节点（n0）
  NodeContainer wifi2ApNode = p2pNodes.Get (1);
  

  //创建无线设备于无线节点之间的互联通道，并将通道对象与物理层对象关联
  //确保所有物理层对象使用相同的底层信道，即无线信道
  YansWifiChannelHelper channel2 = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy2 = YansWifiPhyHelper::Default ();
  phy2.SetChannel (channel2.Create ());

  //配置速率控制算法，AARF算法
  WifiHelper wifi2 = WifiHelper::Default ();
  wifi2.SetRemoteStationManager ("ns3::AarfWifiManager");

  NqosWifiMacHelper mac2 = NqosWifiMacHelper::Default ();

  //配置mac类型为sta模式，不发送探测请求
  Ssid ssid2 = Ssid ("ns-3-ssid2");
  mac2.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid2),
               "ActiveProbing", BooleanValue (false));

  //创建无线设备，将mac层和phy层安装到设备上
  NetDeviceContainer staDevices2;
  staDevices2= wifi2.Install (phy2, mac2, wifi2StaNodes);

  //配置AP节点的mac层为AP模式，创建AP设备
  mac2.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid2));

  NetDeviceContainer apDevices2;
  apDevices2 = wifi2.Install (phy2, mac2, wifi2ApNode);

///////////////////////////移动模型为随机游走模型，设定初始位置为两个六边形，边长10m，sta在6个顶点上，ap在六边形中间；两个六边形中心相距50m
//////////////则取AP1为（-25，0），AP2（25.，0）

  MobilityHelper mobility1;
  mobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-100, 100, -100, 100)));
/////设置移动模型
  mobility1.Install (wifi1StaNodes.Get(0));
  Ptr<RandomWalk2dMobilityModel> mob1 = wifi1StaNodes.Get(0)->GetObject<RandomWalk2dMobilityModel>();
  mob1->SetPosition(Vector(-15, 0, 0.0)); 
//sta0

  mobility1.Install (wifi1StaNodes.Get(1));
  Ptr<RandomWalk2dMobilityModel> mob3 = wifi1StaNodes.Get(1)->GetObject<RandomWalk2dMobilityModel>();
  mob3->SetPosition(Vector(-20, 10*sqrt(3)/2, 0.0));
//sta1

  mobility1.Install (wifi1StaNodes.Get(2));
  Ptr<RandomWalk2dMobilityModel> mob4 = wifi1StaNodes.Get(2)->GetObject<RandomWalk2dMobilityModel>();
  mob4->SetPosition(Vector(-30, 10*sqrt(3)/2, 0.0));
//sta2

  mobility1.Install (wifi1StaNodes.Get(3));
  Ptr<RandomWalk2dMobilityModel> mob5 = wifi1StaNodes.Get(3)->GetObject<RandomWalk2dMobilityModel>();
  mob5->SetPosition(Vector(-35, 0, 0.0));
//sta3

  mobility1.Install (wifi1StaNodes.Get(4));
  Ptr<RandomWalk2dMobilityModel> mob6 = wifi1StaNodes.Get(4)->GetObject<RandomWalk2dMobilityModel>();
  mob6->SetPosition(Vector(-30, -10*sqrt(3)/2, 0.0));
//sta4

  mobility1.Install (wifi1StaNodes.Get(5));
  Ptr<RandomWalk2dMobilityModel> mob7 = wifi1StaNodes.Get(5)->GetObject<RandomWalk2dMobilityModel>();
  mob7->SetPosition(Vector(-20, -10*sqrt(3)/2, 0.0));
//sta5

 
  mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility1.Install (wifi1ApNode);
  Ptr<ConstantPositionMobilityModel> mob = wifi1ApNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
  mob->SetPosition(Vector(-25, 0, 0.0)); 
/////wifi ap 移动模型


////wifi2 sta移动模型 
  MobilityHelper mobility2;
  mobility2.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-100, 100, -100, 100)));
  mobility2.Install (wifi2StaNodes.Get(0));
  Ptr<RandomWalk2dMobilityModel> mod1 = wifi2StaNodes.Get(0)->GetObject<RandomWalk2dMobilityModel>();
  mod1->SetPosition(Vector(15, 0, 0.0)); 
  mobility2.Install (wifi2StaNodes.Get(1));
  Ptr<RandomWalk2dMobilityModel> mod2 = wifi2StaNodes.Get(1)->GetObject<RandomWalk2dMobilityModel>();
  mod2->SetPosition(Vector(20, 10*sqrt(3)/2, 0.0));
  mobility2.Install (wifi2StaNodes.Get(2));
  Ptr<RandomWalk2dMobilityModel> mod3 = wifi2StaNodes.Get(2)->GetObject<RandomWalk2dMobilityModel>();
  mod3->SetPosition(Vector(30,10*sqrt(3)/2, 0.0));
  mobility2.Install (wifi2StaNodes.Get(3));
  Ptr<RandomWalk2dMobilityModel> mod4 = wifi2StaNodes.Get(3)->GetObject<RandomWalk2dMobilityModel>();
  mod4->SetPosition(Vector(35, 0, 0.0));
  mobility2.Install (wifi2StaNodes.Get(4));
  Ptr<RandomWalk2dMobilityModel> mod5 = wifi2StaNodes.Get(4)->GetObject<RandomWalk2dMobilityModel>();
  mod5->SetPosition(Vector(30, -10*sqrt(3)/2, 0.0));
  mobility2.Install (wifi2StaNodes.Get(5));
  Ptr<RandomWalk2dMobilityModel> mod6 = wifi2StaNodes.Get(5)->GetObject<RandomWalk2dMobilityModel>();
  mod6->SetPosition(Vector(20, -10*sqrt(3)/2, 0.0));

//wifi2 ap移动模型
  mobility2.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility2.Install (wifi2ApNode);
  Ptr<ConstantPositionMobilityModel> mob2 = wifi2ApNode.Get(0)->GetObject<ConstantPositionMobilityModel>();
  mob2->SetPosition(Vector(25, 0, 0.0)); 

////安装IP
  InternetStackHelper stack;
  stack.Install (wifi1ApNode);
  stack.Install (wifi1StaNodes);
  stack.Install (wifi2ApNode);
  stack.Install (wifi2StaNodes);
  
  //分配IP地址
  Ipv4AddressHelper address;
 //P2P信道
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);
 //wifi1信道
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer wifistaip;
  wifistaip=address.Assign (staDevices1);//////////////////////////////设定服务器时需用到wifi ip
  address.Assign (apDevices1);
 //wifi2信道
  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices2);
  address.Assign (apDevices2);

  //放置echo服务端程序在最右边的csma节点,端口为9
  UdpEchoServerHelper echoServer (9);//////安装服务器应用程序，端口号为9，，发送端口也为监听端口

  ApplicationContainer serverApps = echoServer.Install (wifi1StaNodes.Get(nWifi1-1));////服务程序安装到CSMA的最后一个设备n4
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));/////////////设置应用程序开始以及结束时间，1 秒开始10秒结束

  //回显客户端放在最后的STA节点，指向CSMA网络的服务器，上面的节点地址，端口为9
  UdpEchoClientHelper echoClient (wifistaip.GetAddress(nWifi1-1), 9);/////////安装客户端应用程序，，客户端发送的目标节点IP，，即客户端程序往哪里发送数据，;
//发送到n4,n4为服务器端程序安装的节点，端口为9
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (512));////////////客户端发送的一些参数

  //安装其他节点应用程序
  ApplicationContainer clientApps[]=  { echoClient.Install (wifi2StaNodes.Get (nWifi2-1)),echoClient.Install (wifi2StaNodes.Get     (nWifi2-2)),echoClient.Install (wifi2StaNodes.Get (nWifi2-3)),
  echoClient.Install (wifi2StaNodes.Get (nWifi1-2))};
  for(int i=0;i<4;i++)
  { 
  clientApps[i].Start (Seconds (i+2));
  clientApps[i].Stop (Seconds (10.0));
                                         }

  //启动互联网络路由
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

 
   pointToPoint.EnablePcapAll ("ns3-lec-project4-p2p");
   phy1.EnablePcap ("ns3-lec-project4-wifi1", apDevices1.Get (0));
   phy2.EnablePcap ("ns3-lec-project4-wifi2", apDevices2.Get (0));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

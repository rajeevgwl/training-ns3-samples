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
//
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CBR_tracingExample");

double firstRxTime = -1.0, lastRxTime;
uint32_t bytesTotal = 0;

void SinkRxTrace(Ptr<const Packet> pkt, const Address &addr)
{
  if (firstRxTime < 0)
    firstRxTime = Simulator::Now().GetSeconds();

  lastRxTime = Simulator::Now().GetSeconds();
  bytesTotal += pkt->GetSize();
}


int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (nWifi > 18)
    {
      std::cout << "Number of wifi nodes " << nWifi << 
                   " specified exceeds the mobility bounding box" << std::endl;
      exit (1);
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NS_LOG_INFO ("Project_03_ns3Wireless_results_with_Existingtracing");

  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
   YansWifiPhyHelper phy;
   phy.SetChannel (channel.Create ());

  WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");
    mac.SetType ("ns3::StaWifiMac",
                 "Ssid", SsidValue (ssid),
                 "ActiveProbing", BooleanValue (false));

    NetDeviceContainer staDevices;
    staDevices = wifi.Install (phy, mac, wifiStaNodes);

    mac.SetType ("ns3::ApWifiMac",
                 "Ssid", SsidValue (ssid));

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNode);

    MobilityHelper mobility;


  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);


  uint16_t port = 9;
    OnOffHelper onoff(
        "ns3::UdpSocketFactory",
        InetSocketAddress("10.1.2.4", port));
    onoff.SetAttribute("OnTime",StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onoff.SetAttribute("OffTime",StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    onoff.SetAttribute("DataRate", StringValue("512Kbps"));
    onoff.SetAttribute("PacketSize", StringValue("512"));
    onoff.Install(wifiStaNodes.Get(nWifi - 1));
    ApplicationContainer apps = onoff.Install(wifiStaNodes.Get(nWifi-1));
    apps.Start(Seconds(5.0));
    apps.Stop(Seconds(20.0));

    PacketSinkHelper sink(
          "ns3::UdpSocketFactory",
          InetSocketAddress("10.1.2.4", port));
      sink.Install(csmaNodes.Get(nCsma));

      Simulator::Stop(Seconds(25.0));


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


  pointToPoint.EnablePcapAll ("pm_ex3CBR_existraced");
  phy.EnablePcap ("pm_ex3CBR_existraced", apDevices.Get (0));
  csma.EnablePcap ("pm_ex3CBR_existraced", csmaDevices.Get (0), true);

  Config::ConnectWithoutContext(
        "/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",
        MakeCallback(&SinkRxTrace));


  Simulator::Run ();

  std::cout << "Avg throughput = "
        << bytesTotal*8/(lastRxTime-firstRxTime)/1024
        << " kbits/sec" << std::endl;



  Simulator::Destroy ();
  return 0;
}

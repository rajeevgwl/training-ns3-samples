/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005,2006,2007 INRIA, 2015 Konstantinos Katsaros
 *
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
 // ./waf --run "scratch/training-wifi-example.cc"
 // ==>output files: training-wifi-example.xml; train-wifi-4-0.pcap; train-wifi-0-0.pcap; train-wifi-1-0.pcap; train-wifi-2-0.pcap
 // train-wifi-3-0.pcap; train-wifi-4-0.pcap
 //
 // ./waf --run "scratch/training-wifi-example.cc --infra='true'"
 // ==>output files: training-wifi-example.xml; train-wifi-4-0.pcap; train-wifi-0-0.pcap; train-wifi-1-0.pcap; train-wifi-2-0.pcap
 // train-wifi-3-0.pcap; train-wifi-4-0.pcap train-wifi-5-0.pcap
 //
 // ./waf --run "scratch/training-wifi-example.cc --infra='true' --verbose = 'true'"

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/netanim-module.h"
#include <iostream>

using namespace ns3;
AnimationInterface * pAnim = 0;


static void
SetPosition (Ptr<Node> node, Vector position)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  mobility->SetPosition (position);
}

static Vector
GetPosition (Ptr<Node> node)
{
  Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();
  return mobility->GetPosition ();
}

static void
AdvancePosition (Ptr<Node> node)
{
  Vector pos = GetPosition (node);
  pos.x += 5.0;
  if (pos.x >= 210.0)
    {
      return;
    }
  SetPosition (node, pos);
  Simulator::Schedule (Seconds (1.0), &AdvancePosition, node);
}




int main (int argc, char *argv[])
{
  bool g_verbose = true;
  bool infra_mode = false;
  bool adap_rate = false;
  double radius = 70.0;

  std::string phyMode ("OfdmRate54Mbps");

  CommandLine cmd;
  cmd.AddValue ("verbose", "Print trace information if true", g_verbose);
  cmd.AddValue ("infra", "Infrastructure or Adhoc mode", infra_mode);
  cmd.Parse (argc, argv);

std::string animFile = "training-wifi-example.xml" ;  // Name of file for animation output

  Packet::EnablePrinting ();

  // enable rts cts all the time.
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
  // disable fragmentation
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));

  NodeContainer stas;
  NodeContainer ap;
  NetDeviceContainer staDevs, apDevs;
  PacketSocketHelper packetSocket;

  stas.Create (5);
  if (infra_mode){
	ap.Create (1);
  }
  // give packet socket powers to nodes.
  packetSocket.Install (stas);
  if (infra_mode){
    packetSocket.Install (ap);
  }

  // mobility.
  if (infra_mode){
    // Position of ap
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    MobilityHelper enbMobility;
    enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    enbMobility.SetPositionAllocator (positionAlloc);
    enbMobility.Install (ap);
  }
  // Position of mobile nodes
  MobilityHelper mobile;
  mobile.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (radius));
  mobile.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobile.Install (stas);

  if (infra_mode){
	  Simulator::Schedule (Seconds (1.0), &AdvancePosition, ap.Get (0));
  }

  // Net Devices
  WifiHelper wifi;
  wifi.SetStandard (WIFI_STANDARD_80211a);
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
//  Default Error Model -- NistErrorRateModel

//  For manual WiFi Channel Configuration
//  YansWifiChannelHelper wifiChannel;
//  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
//  wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
//                                  "Exponent", DoubleValue (3.0));


  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);


  Ssid ssid = Ssid ("wifi-default");

  // Rate Manager
  if (adap_rate){
	  wifi.SetRemoteStationManager ("ns3::ArfWifiManager");
  }
  else{
	  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
	                                "DataMode",StringValue (phyMode),
	                                "ControlMode",StringValue (phyMode));
  }
  WifiMacHelper wifiMac;
  // setup stas.
  if (infra_mode){

		  wifiMac.SetType ("ns3::StaWifiMac",
						   "Ssid", SsidValue (ssid),
						   "ActiveProbing", BooleanValue (false));
		  staDevs = wifi.Install (wifiPhy, wifiMac, stas);
		  // setup ap.
		  wifiMac.SetType ("ns3::ApWifiMac",
						   "Ssid", SsidValue (ssid));
		  apDevs = wifi.Install (wifiPhy, wifiMac, ap);


  }
  else {

		 wifiMac.SetType ("ns3::AdhocWifiMac");
		 staDevs = wifi.Install (wifiPhy, wifiMac, stas);

  }

  wifiPhy.EnablePcap("train-wifi",staDevs);

  if (infra_mode){
	  wifiPhy.EnablePcap("train-wifi",apDevs);
  }

  PacketSocketAddress psocket;
  psocket.SetSingleDevice (staDevs.Get (0)->GetIfIndex ());
  psocket.SetPhysicalAddress (staDevs.Get (1)->GetAddress ());
  psocket.SetProtocol (1);

  OnOffHelper onoff ("ns3::PacketSocketFactory", Address (psocket));
  onoff.SetConstantRate (DataRate ("500kb/s"));

  ApplicationContainer apps = onoff.Install (stas.Get (0));
  apps.Start (Seconds (0.5));
  apps.Stop (Seconds (43.0));


  Simulator::Stop (Seconds (44.0));


  /****  animation code ********/

  // Create the animation object and configure for specified output
  pAnim = new AnimationInterface (animFile);


  Simulator::Run ();
  std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy ();

  return 0;
}

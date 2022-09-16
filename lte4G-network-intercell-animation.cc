//
// Animation of LTE/4G Network (working without error, 31july)
//f
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include "ns3/radio-bearer-stats-calculator.h"

#include <iomanip>
#include <string>

#include "ns3/netanim-module.h"

using namespace ns3;


AnimationInterface * pAnim = 0;

/// RGB struture
struct rgb {
  uint8_t r; ///< red
  uint8_t g; ///< green
  uint8_t b; ///< blue
};

struct rgb colors [] = {
                        { 255, 0, 0 }, // Red
                        { 0, 255, 0 }, // Blue
                        { 0, 0, 255 }  // Green
                        };

                        static double size = 50;
                        static uint32_t resourceUE;
                        static uint32_t resourceEnB;

void modify ()
{
  std::ostringstream oss;
  oss << "Update:" << Simulator::Now ().GetSeconds ();
  pAnim->UpdateLinkDescription (0, 1, oss.str ());
  pAnim->UpdateLinkDescription (0, 2, oss.str ());
  pAnim->UpdateLinkDescription (0, 3, oss.str ());
  pAnim->UpdateLinkDescription (0, 4, oss.str ());
  pAnim->UpdateLinkDescription (0, 5, oss.str ());
  pAnim->UpdateLinkDescription (0, 6, oss.str ());
  pAnim->UpdateLinkDescription (1, 7, oss.str ());
  pAnim->UpdateLinkDescription (1, 8, oss.str ());
  pAnim->UpdateLinkDescription (1, 9, oss.str ());
  pAnim->UpdateLinkDescription (1, 10, oss.str ());
  pAnim->UpdateLinkDescription (1, 11, oss.str ());



        // image addition to Enode-B0,1
        for (uint32_t nodeId = 0; nodeId < 2; ++nodeId){
          pAnim->UpdateNodeSize (nodeId, 100, 120);
          pAnim->UpdateNodeImage (nodeId, resourceEnB);}
        // image addition to cellphones, 2,3,4,5,6,7,8,9,10,11,
      for (uint32_t nodeId = 2; nodeId < 12; ++nodeId){
        pAnim->UpdateNodeSize (nodeId, size, size);
        pAnim->UpdateNodeImage (nodeId, resourceUE);}
        // image addition to cellphones
        for (uint32_t nodeId = 12; nodeId < 22; ++nodeId){
          pAnim->UpdateNodeSize (nodeId, size, size);
          pAnim->UpdateNodeImage (nodeId, resourceUE);}


  if (Simulator::Now ().GetSeconds () < 10) // This is important or the simulation
    // will run endlessly
    Simulator::Schedule (Seconds (0.1), modify);

}
/**
 * This simulation script creates two eNodeBs and drops randomly several UEs in
 * a disc around them (same number on both). The number of UEs , the radius of
 * that disc and the distance between the eNodeBs can be configured.
 */
int main (int argc, char *argv[])
{
  double enbDist = 60.0;
  double radius = 70.0;
  uint32_t numUes = 10;
  double simTime = 3.0;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("enbDist", "distance between the two eNBs", enbDist);
  cmd.AddValue ("radius", "the radius of the disc where UEs are placed around an eNB", radius);
  cmd.AddValue ("numUes", "how many UEs are attached to each eNB", numUes);
  cmd.AddValue ("simTime", "Total duration of the simulation (in seconds)", simTime);
  cmd.Parse (argc, argv);

  std::string animFile = "lte-EnodeB-UEs.xml" ;  // Name of file for animation output


  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  // determine the string tag that identifies this simulation run
  // this tag is then appended to all filenames

  UintegerValue runValue;
  GlobalValue::GetValueByName ("RngRun", runValue);

  std::ostringstream tag;
  tag  << "_enbDist" << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << enbDist
       << "_radius"  << std::setw (3) << std::setfill ('0') << std::fixed << std::setprecision (0) << radius
       << "_numUes"  << std::setw (3) << std::setfill ('0')  << numUes
       << "_rngRun"  << std::setw (3) << std::setfill ('0')  << runValue.Get () ;

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();

  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes1, ueNodes2;
  enbNodes.Create (2);
  ueNodes1.Create (numUes);
  ueNodes2.Create (numUes);

  // Position of eNBs
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (enbDist, 0.0, 0.0));
  MobilityHelper enbMobility;
  enbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbMobility.SetPositionAllocator (positionAlloc);
  enbMobility.Install (enbNodes);

  // Position of UEs attached to eNB 1
  MobilityHelper ue1mobility;
  ue1mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (0.0),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (radius));
  ue1mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ue1mobility.Install (ueNodes1);

  // Position of UEs attached to eNB 2
  MobilityHelper ue2mobility;
  ue2mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                    "X", DoubleValue (enbDist),
                                    "Y", DoubleValue (0.0),
                                    "rho", DoubleValue (radius));
  ue2mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ue2mobility.Install (ueNodes2);



  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  NetDeviceContainer ueDevs2;
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  // Attach UEs to a eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (1));

  // Activate a data radio bearer each UE
  enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
  EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);
  lteHelper->ActivateDataRadioBearer (ueDevs2, bearer);

  Simulator::Stop (Seconds (simTime));

  // Insert RLC Performance Calculator
  std::string dlOutFname = "DlRlcStats";
  dlOutFname.append (tag.str ());
  std::string ulOutFname = "UlRlcStats";
  ulOutFname.append (tag.str ());

  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();

  /****  animation code ********/

  // Create the animation object and configure for specified output
  pAnim = new AnimationInterface (animFile);


  //for (uint32_t i = 0; i < remoteHostContainer.GetN (); ++i)
  //  {
  //    pAnim->UpdateNodeDescription (remoteHostContainer.Get (i), "EPC-Remote-Host"); // Optional
  //    pAnim->UpdateNodeColor (remoteHostContainer.Get (i), 255, 0, 0); // Optional
  //  }
  resourceUE = pAnim->AddResource ("/home/rajiv/ns3/images/EUsBig.png");
  resourceEnB = pAnim->AddResource ("/home/rajiv/ns3/images/EnodeBigB.png");
  pAnim->SetBackgroundImage ("/home/rajiv/ns3/images/astuBig3.png", 0, 0, 0.2, 0.2, 0.1);

  //resourceUE = pAnim->AddResource ("/home/rajiv/ns3/images/EUsBig.png");
  //resourceEnB = pAnim->AddResource ("/home/rajiv/ns3/images/EnodeBigB.png");
  //pAnim->SetBackgroundImage ("/home/rajiv/ns3/images/adamauniversity.png", 0, 0, 0.2, 0.2, 0.1);

  for (uint32_t i = 0; i < enbNodes.GetN (); ++i)
    {
      pAnim->UpdateNodeDescription (enbNodes.Get (i), "EnodeB"); // Optional
      pAnim->UpdateNodeColor (enbNodes.Get (i), 0, 255, 0); // Optional
    }


  for (uint32_t i = 0; i < ueNodes1.GetN (); ++i)
    {
      pAnim->UpdateNodeDescription (ueNodes1.Get (i), "UE1"); // Optional
      pAnim->UpdateNodeColor (ueNodes1.Get (i),255, 0,  0); // Optional
    }


    for (uint32_t i = 0; i < ueNodes2.GetN (); ++i)
      {
        pAnim->UpdateNodeDescription (ueNodes2.Get (i), "UE2"); // Optional
        pAnim->UpdateNodeColor (ueNodes2.Get (i), 0, 0, 255); // Optional
      }



            Simulator::Schedule (Seconds (0.1), modify);

  //  pAnim->EnablePacketMetadata (); // Optional
  //  pAnim->EnableIpv4RouteTracking ("lte-simple-epc.xml", Seconds (0), Seconds (5), Seconds (0.25)); //Optional
  //  pAnim->EnableWifiMacCounters (Seconds (0), Seconds (10)); //Optional
  //  pAnim->EnableWifiPhyCounters (Seconds (0), Seconds (10)); //Optional

  /****  animation code ********/




  Simulator::Run ();
    std::cout << "Animation Trace file created:" << animFile.c_str ()<< std::endl;
  Simulator::Destroy ();
  return 0;
}

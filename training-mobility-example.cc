/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
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
//./waf --run "scratch/training-mobility-example.cc"
/*
*./waf --run "scratch/training-mobility-example.cc --PrintHelp"
*
*Program Options:
*    --nodes:     Number of nodes [10]
*    --scenario:  1.ParkingLot, 2.Highway, 3.TraceFile [1]
*    --verbose:   Print on screen positions [false]
*    --anim:      Animate scenario [false]
*
*
*./waf --run "scratch/training-mobility-example.cc --nodes=30 --scenario=1 --anim='true'"
*==>output files:  mobility-training.xml
*
*./waf --run "scratch/training-mobility-example.cc --nodes=30 --scenario=2 --anim='true'"
*
*./waf --run "scratch/training-mobility-example.cc --nodes=30 --scenario=3 --anim='true'"
*
*
*./waf --run "scratch/training-mobility-example.cc --nodes=30 --scenario=1 --verbose = 'true' --anim='true'"
*
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  uint32_t n_nodes = 10;
  int scenario = 1;
  bool verbose = false;
  bool animate = false;
  std::string animFile = "mobility-training.xml";
  std::string traceFile = "scratch/grid-trace.txt";

  CommandLine cmd;
  cmd.AddValue("nodes","Number of nodes", n_nodes);
  cmd.AddValue("scenario", "1.ParkingLot, 2.Highway, 3.TraceFile", scenario);
  cmd.AddValue("verbose","Print on screen positions", verbose);
  cmd.AddValue("anim", "Animate scenario", animate);
  cmd.Parse (argc, argv);

  // create nodes for testing
  NodeContainer nodes;
  nodes.Create (n_nodes);

  MobilityHelper mobility;

  switch (scenario){
  case 1:
  {
	  // setup the grid itself: nodes are layed out
	  // started from (-100,-100) with 20 nodes per row,
	  // the x interval between each object is 5 meters
	  // and the y interval between each node is 20 meters
	  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
									 "MinX", DoubleValue (-100.0),
									 "MinY", DoubleValue (-100.0),
									 "DeltaX", DoubleValue (5.0),
									 "DeltaY", DoubleValue (20.0),
									 "GridWidth", UintegerValue (20),
									 "LayoutType", StringValue ("RowFirst"));
	  // each object will be attached a static position.
	  // i.e., once set by the "position allocator", the
	  // position will never change.
	  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

	  // finalize the setup by attaching to each object
	  // in the input array a position and initializing
	  // this position with the calculated coordinates.
	  mobility.Install (nodes);
	  break;
  }
  case 2:
  {
	  // setup the highway: place the nodes on a straight line from X (0,1000)
	  // using uniform random variable and Y is constant at 50.
	  Ptr<RandomRectanglePositionAllocator> positionAloc = CreateObject<RandomRectanglePositionAllocator>();
	  positionAloc->SetAttribute("X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"));
	  positionAloc->SetAttribute("Y", StringValue("ns3::ConstantRandomVariable[Constant=50.0]"));
	  mobility.SetPositionAllocator(positionAloc);

	  // Set a constant velocity mobility model
	  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
	  mobility.Install (nodes);

	  // setup a uniform random variable for the speed
	  Ptr<UniformRandomVariable> rvar = CreateObject<UniformRandomVariable>();
	  // for each node set up its speed according to the random variable
	  for (NodeContainer::Iterator iter= nodes.Begin(); iter!=nodes.End(); ++iter){
		  Ptr<Node> tmp_node = (*iter);
		  // select the speed from (15,25) m/s
		  double speed = rvar->GetValue(15, 25);
		  tmp_node->GetObject<ConstantVelocityMobilityModel>()->SetVelocity(Vector(speed, 0, 0));
	  }
	  break;
  }
  case 3:
  {
	  // Create Ns2MobilityHelper with the specified trace log file as parameter
	  // Trace file generated from BonnMotion with these parameters:
	  //	  model=ManhattanGrid
	  //	  ignore=3600.0
	  //	  randomSeed=1358102148290
	  //	  x=2000.0
	  //	  y=2000.0
	  //	  duration=200.0
	  //	  nn=200
	  //	  circular=false
	  //	  xblocks=3
	  //	  yblocks=3
	  //	  updateDist=5.0
	  //	  turnProb=0.5
	  //	  speedChangeProb=0.2
	  //	  minSpeed=10.0
	  //	  meanSpeed=12.0
	  //	  speedStdDev=0.2
	  //	  pauseProb=0.0
	  //	  maxPause=0.0

	  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);

	  // configure movements for each node, while reading
	  ns2.Install ();
	  break;
  }
  default:
	  NS_LOG_UNCOND ("Scenario not supported");
	  return 0;
  }


  if (verbose){
	  // iterate our nodes and print their position.
	  for (NodeContainer::Iterator j = nodes.Begin ();
		   j != nodes.End (); ++j)
		{
		  Ptr<Node> object = *j;
		  Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
		  NS_ASSERT (position != 0);
		  Vector pos = position->GetPosition ();
		  NS_LOG_UNCOND ("Node: " << object->GetId() << " x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z );
		}
  }

  AnimationInterface * panim;
  if (animate)
  {
	  panim = new AnimationInterface (animFile);
  }

  Simulator::Stop(Seconds(10));
  Simulator::Run();
  Simulator::Destroy ();
  return 0;
}

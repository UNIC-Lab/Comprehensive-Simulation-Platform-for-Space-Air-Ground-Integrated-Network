/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/* 
 * Author: Weisen Shi w46shi@uwaterloo.ca
 *
 * This is the main simulation script (under construciton) of SAG platform.

 */

// includes
#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include <iostream>

#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"

#include "ns3/netanim-module.h"

#include <fstream>
#include <sstream>
#include "ns3/core-module.h"
#include "ns3/ns2-mobility-helper.h"

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/gtk-config-store.h"

#include "ns3/satellite-module.h"


using namespace ns3;

/**
 * Sample simulation script for space air ground platform. It instantiates 30 vehicles, 1 eNodeB,
 * 3 UAVs, 1 satellite and 1 server (edge controller).
 * Each vehicle is equipped with 3 devices (LTE user devices, UAV user devices, SAT user deivces)
 * Each vehicle updates uplink controller message periodically in uplink control message simulation || receives downlink data message in donwlink data simulation
 * The server is connected with eNodeB, UAVs, and satellite through p2p links.
 * The server receives downlink data message in uplink control message simulation || generate data message in downlink data simulation to each vehicle via different methods (LTE, UAV, SAT)
 * The decision down by the server is at what time ~ which vehicle ~ should access which type of access methods (LTE, UAV, SAT)
 */

NS_LOG_COMPONENT_DEFINE ("SpaceAirGroundSimple");

// ** function indicates that the server received one uplink control message through LTE method------------------------------------------------------------------------------------------
void ReceivePacketUlControlLte (Ptr<Socket> socket){
  Ptr<Packet> packetSocket;
  while ((packetSocket = socket->Recv ())){
    Address toIpv4;
    socket->GetSockName(toIpv4);
    // uint32_t toNode = socket->GetNode()->GetId();
    SeqTsHeader seqTsHrecv = SeqTsHeader();
    packetSocket->RemoveHeader(seqTsHrecv);
    // Ipv4Address oToIpv4 = InetSocketAddress::ConvertFrom(toIpv4).GetIpv4();
    uint8_t *packetContentPrt = new uint8_t[packetSocket->GetSize()];
    packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());
    // uint32_t packetContent = packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());

    // ** printout the information of received message
    // ** format: receiving time ~ rULC ~ method types(lte, uav, sat) ~ X coordinate ~ Y coordinate ~ package sending time
    if((Simulator::Now().GetSeconds() - seqTsHrecv.GetTs().GetSeconds()) < 2.0){
    	NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << "rUlC" << "\t" << "lte" << "\t" << (char *)packetContentPrt << "\t" <<seqTsHrecv.GetTs().GetSeconds());
    }
  }
}

// ** function indicates that the server received one uplink control message through SAT method------------------------------------------------------------------------------------------
void ReceivePacketUlControlSat (Ptr<Socket> socket){
  Ptr<Packet> packetSocket;
  while ((packetSocket = socket->Recv ())){
    Address toIpv4;
    socket->GetSockName(toIpv4);
    // uint32_t toNode = socket->GetNode()->GetId();
    SeqTsHeader seqTsHrecv = SeqTsHeader();
    packetSocket->RemoveHeader(seqTsHrecv);
    // Ipv4Address oToIpv4 = InetSocketAddress::ConvertFrom(toIpv4).GetIpv4();
    uint8_t *packetContentPrt = new uint8_t[packetSocket->GetSize()];
    packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());
    // uint32_t packetContent = packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << "rUlC" << "\t" << "sat" << "\t" << (char *)packetContentPrt << "\t" <<seqTsHrecv.GetTs().GetSeconds());
  }
}

// ** function indicates that the server received one uplink control message through UAV method------------------------------------------------------------------------------------------
void ReceivePacketUlControlUav (Ptr<Socket> socket){
  Ptr<Packet> packetSocket;
  while ((packetSocket = socket->Recv ())){
    Address toIpv4;
    socket->GetSockName(toIpv4);
    uint32_t toNode = socket->GetNode()->GetId() - 34;
    SeqTsHeader seqTsHrecv = SeqTsHeader();
    packetSocket->RemoveHeader(seqTsHrecv);
    // Ipv4Address oToIpv4 = InetSocketAddress::ConvertFrom(toIpv4).GetIpv4();
    uint8_t *packetContentPrt = new uint8_t[packetSocket->GetSize()];
    packetSocket->CopyData(packetContentPrt, packetSocket->GetSize()); 
    // uint32_t packetContent = packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << "rUlC" << "\t" << "uav" << toNode << "\t" << (char *)packetContentPrt << "\t" <<seqTsHrecv.GetTs().GetSeconds());
  }
}

// ** function generates the uplink control message from each vehicle--------------------------------------------------------------------------------------------------------------------
static void GenerateTrafficUlControl (Ptr<Socket> socket, InetSocketAddress dstaddr, uint32_t pktSize, Ptr<Node> currVehicle){
	Ptr<MobilityModel> currMobi = currVehicle->GetObject<MobilityModel>();
    std::string Xcoor = std::to_string(currMobi->GetPosition().x);
    std::string Ycoor = std::to_string(currMobi->GetPosition().y);
    std::string conMsgContent = std::to_string(currVehicle->GetId()) + "\t" + Xcoor + "\t" + Ycoor; // + "\t" + std::to_string(Simulator::Now ().GetSeconds ());
	const char * conMsgContentChar = conMsgContent.c_str();
	SeqTsHeader seqTsH = SeqTsHeader();

	socket->Connect (dstaddr);
	Ptr<Packet> p = Create<Packet>((uint8_t *)conMsgContentChar, pktSize);
	p->AddHeader(seqTsH);
	socket->Send (p);
	// socket->BindToNetDevice (netdev);
	// socket->SendTo(p, 0, dstaddr);
	// Simulator::Schedule (pktInterval, &GenerateTrafficUlControl, socket, netdev, dstaddr, pktSize, pktContent, pktInterval);
	// NS_LOG_UNCOND ("t" << "\t" << "UlC" << "\t" << Simulator::Now ().GetSeconds () << "\t" << conMsgContent);
}

// ** function generates the downlink data message through LTE method--------------------------------------------------------------------------------------------------------------------
static void GenerateTrafficLte (Ptr<Socket> socket, InetSocketAddress dstaddr, uint32_t pktSize, uint32_t pktCount, Time pktInterval){
  if (pktCount > 0){
    socket->Connect (dstaddr);
    Ptr<Packet> p = Create<Packet>((uint8_t *)"lteltelte", pktSize);
    socket->Send (p);
    // socket->BindToNetDevice (netdev);
    // socket->SendTo(p, 0, dstaddr);
    Simulator::Schedule (pktInterval, &GenerateTrafficLte, socket, dstaddr, pktSize, pktCount - 1, pktInterval);
    // NS_LOG_UNCOND ("t" << "\t" << "lte" << "\t" << p->GetUid() << "\t" << Simulator::Now ().GetSeconds () << "\t" << pktSize << "\t" << dstaddr.GetIpv4());
  }
}

// ** function generates the downlink data message through UAV method--------------------------------------------------------------------------------------------------------------------
static void GenerateTrafficUav (Ptr<Socket> socket, InetSocketAddress dstaddr, uint32_t pktSize, uint32_t pktCount, Time pktInterval){
  if (pktCount > 0){
  	socket->Connect (dstaddr);
    Ptr<Packet> p = Create<Packet>((uint8_t *)"uavuavuav", pktSize);
    socket->Send (p);
    // reinterpret_cast<uint8_t>("isthisworkUav");
    // socket->BindToNetDevice (netdev);
    // socket->SendTo(p, 0, dstaddr);
    Simulator::Schedule (pktInterval, &GenerateTrafficUav, socket, dstaddr, pktSize, pktCount - 1, pktInterval);
    // NS_LOG_UNCOND ("t" << "\t" << "uav" << "\t" << p->GetUid() << "\t" << Simulator::Now ().GetSeconds () << "\t" << pktSize << "\t" << dstaddr.GetIpv4());
  }
}

// ** function generates the downlink data message through SAT method--------------------------------------------------------------------------------------------------------------------
static void GenerateTrafficSat (Ptr<Socket> socket, InetSocketAddress dstaddr, uint32_t pktSize, uint32_t pktCount, Time pktInterval){
  if (pktCount > 0){
  	socket->Connect (dstaddr);
    Ptr<Packet> p = Create<Packet>((uint8_t *)"satsatsat", pktSize);
    socket->Send (p);
    // reinterpret_cast<uint8_t>("isthisworkUav");
    // socket->BindToNetDevice (netdev);
    // socket->SendTo(p, 0, dstaddr);
    Simulator::Schedule (pktInterval, &GenerateTrafficUav, socket, dstaddr, pktSize, pktCount - 1, pktInterval);
    // NS_LOG_UNCOND ("t" << "\t" << "uav" << "\t" << p->GetUid() << "\t" << Simulator::Now ().GetSeconds () << "\t" << pktSize << "\t" << dstaddr.GetIpv4());
  }
}

// ** function indicates that the vehicle received one downlink data message through LTE method------------------------------------------------------------------------------------
void ReceivePacketLte (Ptr<Socket> socket){
  Ptr<Packet> packetSocket;
  while ((packetSocket = socket->Recv ())){
  	Address toIpv4;
  	socket->GetSockName(toIpv4);
    uint32_t toNode = socket->GetNode()->GetId();
    // Ipv4Address oToIpv4 = InetSocketAddress::ConvertFrom(toIpv4).GetIpv4();
    uint8_t *packetContentPrt = new uint8_t[packetSocket->GetSize()];
    packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());

    // ** printout the information of received message
    // ** format: receiving time ~ method types(rLTE, rUAV, rSAT) ~ packet id ~ packet size ~ desitnation node number (vehicle number)
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << "rLTE" << "\t" << packetSocket->GetUid() << "\t" << packetSocket->GetSize() << "\t" << toNode);
  }
}

// ** function indicates that the vehicle received one downlink data message through UAV method------------------------------------------------------------------------------------
void ReceivePacketUav (Ptr<Socket> socket){
  Ptr<Packet> packetSocket;
  while ((packetSocket = socket->Recv ())){
  	Address toIpv4;
  	socket->GetSockName(toIpv4);
  	uint32_t toNode = socket->GetNode()->GetId();
  	// Ipv4Address oToIpv4 = InetSocketAddress::ConvertFrom(toIpv4).GetIpv4();
  	uint8_t *packetContentPrt = new uint8_t[packetSocket->GetSize()];
  	packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());
  	// uint32_t packetContent = packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());
  	NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << "rUAV" << "\t" << packetSocket->GetUid() << "\t" << packetSocket->GetSize() << "\t" << toNode);
  }
}

// ** function indicates that the vehicle received one downlink data message through SAT method------------------------------------------------------------------------------------
void ReceivePacketSat (Ptr<Socket> socket){
  Ptr<Packet> packetSocket;
  while ((packetSocket = socket->Recv ())){
    Address toIpv4;
    socket->GetSockName(toIpv4);
    uint32_t toNode = socket->GetNode()->GetId();
    // Ipv4Address oToIpv4 = InetSocketAddress::ConvertFrom(toIpv4).GetIpv4();
    uint8_t *packetContentPrt = new uint8_t[packetSocket->GetSize()];
  	packetSocket->CopyData(packetContentPrt, packetSocket->GetSize());
    NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "\t" << "rSAT" << "\t" << packetSocket->GetUid() << "\t" << packetSocket->GetSize() << "\t" << toNode);
  }
}


// ** the main function------------------------------------------------------------------------------------------------------------------------------------------------------------
int main (int argc, char *argv[]){

  std::string phyMode ("OfdmRate6MbpsBW10MHz"); 	// ** define physical mode for wifi(802.11p)

  bool verbose = false;  							// ** not using variable
  uint32_t vehicleNum = 30;  						// ** number of vehicles
  // uint32_t realuavNum = 3;
  uint32_t uavNum = 3; 								// ** number of UAVs
  uint32_t lteBSNum = 1; 
  double duration = 10.0; 							// ** simulation total time in seconds
  uint32_t packetSizeLte = 1024; 					// ** LTE packet size in bytes
  uint32_t packetSizeUav[] = {1024, 1024, 1024};    // ** UAV packet size in bytes
  uint32_t packetSizeSat = 1024;					// ** SAT packet size in bytes
  uint32_t numPackets = 50;       					// ** number of packets to be transmitted in one downlink data transmission schedule event
  // uint32_t numPacketsSat = 10;q
  uint32_t longIntervalTime = 598;					// ** scheduled message transmission times for uplink control message transmission
  Time interPacketInterval = Seconds(0.001);		// ** inter packets interval for downlink data transmission
  // double lteStartTime = 0.1;
  // double uavStartTime = 0.1;
  // double satStartTime = 0.1;
  double offset = 0.0001;							// ** offset time between vehicles to prevent simultaneous uplink transmissions
  double offsetUav = 0.005;							// ** offset time between UAVs to prevent simultaneous uplink transmissions (might be useless)
  bool noConMsgSim = true;							// ** trigger to switch between uplink controll message simulation (false) or downlink data message simulation (true)
  uint32_t dlTransGap = 2;							// ** transmission interval between down link data message transmissions

  std::string traceFile ("swsdefine/tracefile/waterlooNS2_10k_600.txt");        // ** import vehicle mobility trace (generated by VISSIM and transformed by parser)
  std::string traceFile1 ("swsdefine/tracefile/UAVPositionRoad_1");				// ** import UAV 1 mobility trace (generated by STK or optimized results and transformed by parser)
  std::string traceFile2 ("swsdefine/tracefile/UAVPositionRoad_2");				// ** import UAV 2 mobility trace (generated by STK or optimized results and transformed by parser)
  std::string traceFile3 ("swsdefine/tracefile/UAVPositionRoad_3");				// ** import UAV 3 mobility trace (generated by STK or optimized results and transformed by parser)
  std::string decisionFile ("swsdefine/tracefile/decision_learning_to_ns3_2");  // ** import controller decision file (generated by user defined optimization algorithm)
  // std::string logFile ("swsdefine/tracefile/ns2-mobility-trace.log");

  uint32_t conMsgPktSize = 128; 												// ** uplink control message packet size in byte
  double conMsgPktLongInterval = 0.2; 											// ** uplink control message packet transmission interval in second
  // uint32_t conMsgPktIterNum = 600;
  // double conMsgDelay = 1.3; // seconds
  double conMsgStartTimeLte = 0.0;												// ** uplink control message transmission start time for LTE user devices
  double conMsgStartTimeUav = 0.0;												// ** uplink control message transmission start time for UAV user devices
  double conMsgStartTimeSat = 0.0;												// ** uplink control message transmission start time for SAT user devices

  bool useCa = false;


  // ** logging of nodes, might be useful in future----------------------------------------------------------------------------------------------------------------------------------
  // Enable logging from the ns2 helper
  // LogComponentEnable("SpaceAirGroundSimple",LOG_LEVEL_DEBUG);
  // LogComponentEnable("OnOffHelper",LOG_LEVEL_DEBUG);

  // ** CommandLine (might be useless, used to control script in terminal)-----------------------------------------------------------------------------------------------------------
  CommandLine cmd;
  cmd.AddValue("verbose", "turn on all WifiNetDevice log components", verbose);
  // cmd.AddValue("useCa", "Whether to use carrier aggregation.", useCa);
  cmd.AddValue("duration", "Duration of Simulation", duration);
  cmd.Parse(argc, argv);

  // ** configuration of LTE parameters (need further investigation to be more realistic)--------------------------------------------------------------------------------------------
  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue (2));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (100));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (100));
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (10240*2));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlEarfcn", UintegerValue (20400));  // ** set as 800MHz bands to prevent interference from Wifi
  Config::SetDefault ("ns3::LteEnbNetDevice::DlEarfcn", UintegerValue (2400));
  if (useCa)
   {
     Config::SetDefault ("ns3::LteHelper::UseCa", BooleanValue (useCa));
     Config::SetDefault ("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue (3));
     Config::SetDefault ("ns3::LteHelper::EnbComponentCarrierManager", StringValue ("ns3::RrComponentCarrierManager"));
   }
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (35.0));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (33.0));
  Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (false));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(160));


  // Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (34.0));
  // Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (33.0));
  // Config::SetDefault ("ns3::LteUePhy::EnableUplinkPowerControl", BooleanValue (false));

  // ConfigStore inputConfig;
  // inputConfig.ConfigureDefaults();
  // parse again so you can override default values from the command line
  // cmd.Parse(argc, argv);


  // ** Creat node container (can be understant from vairable name)-------------------------------------------------------------------------------------------------------------------  
  NodeContainer vehicleNode;
  vehicleNode.Create(vehicleNum);
  NodeContainer allConnectedVehicle;
  uint32_t connectedVehicleLteNum = vehicleNum;
  uint32_t connectedVehicleUavNum = vehicleNum;
  for (uint32_t i=0; i<vehicleNum; i++){ allConnectedVehicle.Add(vehicleNode.Get(i)); }
  NodeContainer connectedVehicleLteNode;
  for (uint32_t i=0; i<connectedVehicleLteNum; i++){ connectedVehicleLteNode.Add(vehicleNode.Get(i)); }
  NodeContainer connectedVehicleUavNode;
  for (uint32_t i=0; i<connectedVehicleUavNum; i++){ connectedVehicleUavNode.Add(vehicleNode.Get(i)); }
  
  NodeContainer uavNode;
  uavNode.Create(uavNum);

  NodeContainer lteNode;
  lteNode.Create(lteBSNum);
  
  // ** Without loss of generality, we specify multiple remotehost controlled by same schedule function for each type of methods (lte, uav1, uav2, uav3, satellite) to simplify the configuration
  // ** Create RemoteHost (server, edge controller) for lte method
  NodeContainer remoteHostContainerLte;
  remoteHostContainerLte.Create (lteBSNum);

  // ** Create RemoteHost (server, edge controller) for UAV method
  NodeContainer remoteHostContainerUav;
  remoteHostContainerUav.Create (uavNum);


  // ** Satellite configuration ---------------------------------------------------------------------------------------------------------------------------------------------------------
  // ** this part of code is revised from SNS3 sample codes. Since now we still not fully implement the SNS3 simulationhelper defined node with our vehicle nodes (and remotehost)
  // ** in this simulation we just parallelly creat same number of UtUsers (vehicles) and GwUser (remotehost) for simulation.
  // ** one of the futhre task is to fully implement the SNS3 with our vehicle and remotehost nodes
  uint32_t beamId = 1;
  uint32_t endUsersPerUt = vehicleNum;
  uint32_t utsPerBeam = 1;

  // Create simulation helper
  auto simulationHelper = CreateObject<SimulationHelper> ("example-tutorial");

  simulationHelper->SetUtCountPerBeam (utsPerBeam);
  simulationHelper->SetUserCountPerUt (endUsersPerUt);
  // Set beam ID
  std::stringstream beamsEnabled; beamsEnabled  << beamId;
  simulationHelper->SetBeams (beamsEnabled.str ());

  simulationHelper->SetSimulationTime (Seconds (duration));

  Ptr<SatHelper> satHelper = simulationHelper->CreateSatScenario ();
  // for getting UT users
  NodeContainer utUsers = satHelper->GetUtUsers ();
  // get GW users
  NodeContainer gwUsers = satHelper->GetGwUsers ();

  // NS_LOG_UNCOND ("Number of utUser: " << utUsers.GetN () << ", gwUsers: " << gwUsers.GetN ());

  // // trace received/transmitted packets of each node
  // NodeContainer calculatedNode;
  // calculatedNode.Add(allConnectedVehicle);
  // // calculatedNode.Add(connectedVehicleUavNode);
  // calculatedNode.Add(remoteHost);
  // calculatedNode.Add(gwUsers.Get(0));
  // calculatedNode.Add(utUsers.Get(0));



  // ** Import trace files of vehicles, uavs-----------------------------------------------------------------------------------------------------------------------------------------------
  Ns2MobilityHelper ns2 = Ns2MobilityHelper (traceFile);
  ns2.Install ();  																		// ** vehicle trace
  Ns2MobilityHelper ns21 = Ns2MobilityHelper (traceFile1);
  ns21.Install ();																		// ** UAV 1 trace
  Ns2MobilityHelper ns22 = Ns2MobilityHelper (traceFile2);
  ns22.Install ();																		// ** UAV 2 trace
  Ns2MobilityHelper ns23 = Ns2MobilityHelper (traceFile3);
  ns23.Install ();																		// ** UAV 3 trace
  NS_LOG_INFO ("Tracefile installed.");

  MobilityHelper bsLocation;
  Ptr<ListPositionAllocator> bsLocAlloc = CreateObject<ListPositionAllocator>();
  bsLocAlloc->Add(Vector (1200, 1100, 0));
  // bsLocAlloc->Add(Vector (1000, 800, 0));
  // bsLocAlloc->Add(Vector (1000, 800, 0));
  bsLocation.SetPositionAllocator (bsLocAlloc);
  bsLocation.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  bsLocation.Install (lteNode);															// ** eNodeB location
  // bsLocation.Install (remoteHostContainerLte);
  // bsLocation.Install (remoteHostContainerUav);
  NS_LOG_INFO ("BS location installed.");



  // ** Install Internet protocol -----------------------------------------------------------------------------------------------------------------------------------------------------------
  InternetStackHelper internet;
  internet.Install (allConnectedVehicle);
  internet.Install (uavNode);
  internet.Install (remoteHostContainerLte);
  internet.Install (remoteHostContainerUav);
  // ** note: do not install internet for eNodeB and pgw here, the later ltehelper will install internet form them automatically


  // ** LTE Configure -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper>(); // ** create lte network

  // ** physical channel configuration
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::LogDistancePropagationLossModel"));
  lteHelper->SetPathlossModelAttribute("Exponent", DoubleValue(3));
  // if (useCa)
  //  {
  //    lteHelper->SetAttribute("UseCa", BooleanValue (useCa));
  //    lteHelper->SetAttribute("NumberOfComponentCarriers", UintegerValue (2));
  //    lteHelper->SetAttribute("EnbComponentCarrierManager", StringValue ("ns3::RrComponentCarrierManager"));
  //  }

  // lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(bandwidth));
  // lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(bandwidth));

  // ** create PGW node
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // ** connect PGW node with remotehost through p2p link
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevicesLte = p2ph.Install (remoteHostContainerLte.Get(0), pgw);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("9.2.0.0", "255.255.255.0");
  Ipv4InterfaceContainer internetIpIfacesLte = ipv4h.Assign (internetDevicesLte);
  // interface 0 is localhost, 1 is the p2p device
  // Ipv4Address remoteHostAddr = internetIpIfacesLte.GetAddress (0);

  // ** configure route between remotehost and LTE user 
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (internetIpIfacesLte.Get(0).first); //  remoteHost->GetObject<Ipv4>()
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // ** Install LTE Devices to the vehicles
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (lteNode);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (connectedVehicleLteNode);

  // ** Install the IP stack on the lte devices
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // ** Assign IP address to lte devices, and install applications
  for (uint32_t u = 0; u < connectedVehicleLteNum; u++){
      // ** Set the default gateway for the lte devices
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueIpIface.Get(u).first);
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      lteHelper->Attach (ueLteDevs.Get(u), enbLteDevs.Get(0));
    }
  

  // ** UAV (WIFI) Configure -----------------------------------------------------------------------------------------------------------------------------------------------------------------------

  // ** Wifi physical layer configuration
  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel", "Exponent", DoubleValue(3));
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);

  // ** ns-3 supports generate a pcap trace (now useless)
  wifiPhy.SetPcapDataLinkType (WifiPhyHelper::DLT_IEEE802_11);

  // ** Wifi mac layer configuration
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  if (verbose){ wifi80211p.EnableLogComponents ();}     // ** Turn on all Wifi 802.11p logging (now uesless)
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                      "DataMode",StringValue (phyMode),
                                      "ControlMode",StringValue (phyMode));

  // ** install wifi configuration to vehicle node
  NetDeviceContainer vehicleDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, connectedVehicleUavNode);

  // ** modify wifi transmission range for uav and install modified wifi configuration to UAVs
  wifiPhy.Set ("TxGain", DoubleValue(28) );
  wifiPhy.Set ("RxGain", DoubleValue(28) );
  NetDeviceContainer uavDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, uavNode);

  // ** configure IP for wifi nodes
  Ipv4AddressHelper ipv4Wifi;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4Wifi.SetBase ("8.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ip4vehicleDsrc = ipv4Wifi.Assign (vehicleDevices);
  Ipv4InterfaceContainer ip4uavDsrc = ipv4Wifi.Assign (uavDevices);

  // ** Connect UAVs to the remotehost
  PointToPointHelper p2pWifi;
  p2pWifi.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2pWifi.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2pWifi.SetChannelAttribute ("Delay", TimeValue (Seconds (0.050)));
  std::vector<NetDeviceContainer> internetDevicesWifiuavList; 
  std::vector<Ipv4InterfaceContainer> internetIpIfacesWifiuavList;  
  for(uint32_t i = 0; i < uavNum; i++){
    NetDeviceContainer internetDevicesWifi = p2pWifi.Install (remoteHostContainerUav.Get(i), uavNode.Get(i));
    Ipv4AddressHelper ipv4uav;
    std::string ipaddress = "9.1." + std::to_string(i) + ".0";
    ipv4uav.SetBase (Ipv4Address(ipaddress.c_str()), "255.255.255.0");
    Ipv4InterfaceContainer internetIpIfacesWifi = ipv4uav.Assign (internetDevicesWifi);
    Ipv4StaticRoutingHelper ipv4RoutingHelperWifi;
    Ptr<Ipv4StaticRouting> remoteHostStaticRoutingWifi = ipv4RoutingHelperWifi.GetStaticRouting (internetIpIfacesWifi.Get(0).first);
    remoteHostStaticRoutingWifi->AddNetworkRouteTo (Ipv4Address ("8.1.1.0"), Ipv4Mask ("255.255.255.0"), 1);
    internetDevicesWifiuavList.push_back(internetDevicesWifi);
    internetIpIfacesWifiuavList.push_back(internetIpIfacesWifi);

    // ** configure the route from vehicle to the remotehost
    for (uint32_t u = 0; u < connectedVehicleUavNum; u++){
      Ptr<Ipv4StaticRouting> uavVehicleStaticRouting = ipv4RoutingHelperWifi.GetStaticRouting (ip4vehicleDsrc.Get(u).first);
      uavVehicleStaticRouting->AddHostRouteTo (internetIpIfacesWifi.GetAddress(0), ip4uavDsrc.GetAddress(i), 2);
    }
  }



  // ** read decision file -----------------------------------------------------------------------------------------------------------------------------------------------------------------------
  uint32_t fileIntervalTime = longIntervalTime;
  uint32_t fileVehicleNum = vehicleNum+1;
  double decisionArray[fileIntervalTime][fileVehicleNum];
  std::ifstream infile;
  infile.open(decisionFile);
  for (uint32_t iTime = 0; iTime < fileIntervalTime; iTime++){
	for (uint32_t jVehicle = 0; jVehicle < fileVehicleNum; jVehicle++){
	  infile >> decisionArray[iTime][jVehicle];
	}
  }
  infile.close();

  
  // ** App Configure -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");   // ** use udp for uplink/downlink data transmission

  // ** configure socket for remoteHost of SAT
  Ptr<Socket> remoteSockSat;
  remoteSockSat = Socket::CreateSocket (gwUsers.Get (0), tid);
  InetSocketAddress remoteSockSatAddress = InetSocketAddress (satHelper->GetUserAddress (gwUsers.Get (0)), 92);
  remoteSockSat->Bind(remoteSockSatAddress);
  remoteSockSat->SetRecvCallback (MakeCallback (&ReceivePacketUlControlSat));

  // ** configure socket for SAT vehicle devices
  for(uint32_t i=0;i<utUsers.GetN();i++){
    Ptr<Socket> vehicleSockSat;
    vehicleSockSat = Socket::CreateSocket (utUsers.Get(i), tid);
    InetSocketAddress vehicleSockSatAddress = InetSocketAddress (satHelper->GetUserAddress (utUsers.Get (i)), 92);
    vehicleSockSat->Bind (vehicleSockSatAddress);
    vehicleSockSat->SetRecvCallback (MakeCallback (&ReceivePacketSat));   
    for (uint32_t k=0;k<longIntervalTime;k+=dlTransGap){
    	if (noConMsgSim){
    		if (decisionArray[k][i] == 4){
      		Simulator::Schedule(Seconds(decisionArray[k][fileVehicleNum-1]), &GenerateTrafficSat,
                                  remoteSockSat, vehicleSockSatAddress, packetSizeSat, numPackets, interPacketInterval);      // ** downlink data transmission schedule
	    	}
    	}else{
    		Simulator::Schedule(Seconds(conMsgStartTimeSat + k*conMsgPktLongInterval + i*offset), &GenerateTrafficUlControl,
                              vehicleSockSat, remoteSockSatAddress, conMsgPktSize, allConnectedVehicle.Get(i));			      // ** uplink control message transmission schedule
    	}
    }
  }


  // ** configure socket for remoteHost of LTE
  Ptr<Socket> remoteSockLte;
  remoteSockLte = Socket::CreateSocket (remoteHostContainerLte.Get(0), tid);
  InetSocketAddress remoteSockLteAddress = InetSocketAddress (internetIpIfacesLte.GetAddress(0), 91);
  remoteSockLte->Bind (remoteSockLteAddress);
  remoteSockLte->SetRecvCallback (MakeCallback (&ReceivePacketUlControlLte));

  // ** configure socket for LTE vehicle devices
  for (uint32_t i=0;i<connectedVehicleLteNum;i++){
    Ptr<Socket> vehicleSockLte;
    vehicleSockLte = Socket::CreateSocket (connectedVehicleLteNode.Get(i), tid);
    InetSocketAddress vehicleSockLteAddress = InetSocketAddress (ueIpIface.GetAddress(i), 91);
    vehicleSockLte->Bind (vehicleSockLteAddress);
    vehicleSockLte->SetRecvCallback (MakeCallback (&ReceivePacketLte));
    for (uint32_t k=0;k<longIntervalTime;k+=dlTransGap){
    	if (noConMsgSim){
			if (decisionArray[k][i] == 0){
    	    Simulator::Schedule(Seconds(decisionArray[k][fileVehicleNum-1]), &GenerateTrafficLte,
                                  remoteSockLte, vehicleSockLteAddress, packetSizeLte, numPackets, interPacketInterval);      // ** downlink data transmission schedule
    	    }
    	}else{
    		Simulator::Schedule(Seconds(conMsgStartTimeLte + k*conMsgPktLongInterval + i*offset), &GenerateTrafficUlControl,
            	                  vehicleSockLte, remoteSockLteAddress, conMsgPktSize, allConnectedVehicle.Get(i));           // ** uplink control message transmission schedule
        }
    }
  }
  

  // ** configure socket for remoteHost of UAV
  std::vector<InetSocketAddress> remoteSockUavAddressList;
  std::vector<Ptr<Socket>> remoteSockUavList;
  for(uint32_t i=0;i<uavNum;i++){
  	Ptr<Socket> remoteSockUav;
  	remoteSockUav = Socket::CreateSocket (remoteHostContainerUav.Get(i), tid); 
  	InetSocketAddress remoteSockUavAddress = InetSocketAddress (internetIpIfacesWifiuavList[i].GetAddress(0), 92+i);
  	// remoteSockUav = Socket::CreateSocket (uavNode.Get(i), tid); 
  	// InetSocketAddress remoteSockUavAddress = InetSocketAddress (ip4uavDsrc.GetAddress(i), 92+i); // ip4uavDsrc
  	remoteSockUav->Bind (remoteSockUavAddress);
  	remoteSockUav->SetRecvCallback (MakeCallback (&ReceivePacketUlControlUav));
  	remoteSockUavList.push_back(remoteSockUav);
  	remoteSockUavAddressList.push_back(remoteSockUavAddress);
  }

  // ** configure socket for UAV vehicle devices
  for(uint32_t i=0;i<connectedVehicleUavNum;i++){
    for(uint32_t j=0;j<uavNum;j++){
      Ptr<Socket> vehicleSockUav;
      vehicleSockUav = Socket::CreateSocket (connectedVehicleUavNode.Get(i), tid);
      InetSocketAddress vehicleSockUavAddress = InetSocketAddress (ip4vehicleDsrc.GetAddress(i), 92+j);
      vehicleSockUav->Bind (vehicleSockUavAddress);
      vehicleSockUav->SetRecvCallback (MakeCallback (&ReceivePacketUav));
      for (uint32_t k=0;k<longIntervalTime;k+=dlTransGap){
      	if (noConMsgSim){
			if (decisionArray[k][i] == (j+1)){
				// std::cout<<k<<" "<<i<<" "<<decisionArray[k][i]<<" "<<decisionArray[k][fileVehicleNum-1]<<std::endl;
          	Simulator::Schedule(Seconds(decisionArray[k][fileVehicleNum-1]), &GenerateTrafficUav,
                                  remoteSockUavList[j], vehicleSockUavAddress, packetSizeUav[j], numPackets, interPacketInterval);           // ** downlink data transmission schedule
	      	}
	    }else{
      		Simulator::Schedule(Seconds(conMsgStartTimeUav + k*conMsgPktLongInterval + i*offsetUav + j*offset), &GenerateTrafficUlControl,
            	                  vehicleSockUav, remoteSockUavAddressList[j], conMsgPktSize, allConnectedVehicle.Get(i)); // *3 + j*offset  // ** uplink control message transmission schedule
      	}
      }
    }
  }


  // ** run simulation -----------------------------------------------------------------------------------------------------------------
  Simulator::Stop(Seconds(duration));
  Simulator::Run();

  Simulator::Destroy();
  return 0;
}


  // ** Enable Animation --------------------------------------------------------------------------------------------------------------------
  // AnimationInterface anim("AnimXML/uav.xml");
  // anim.SetBackgroundImage("/home/wilsonsws/ns-allinone-3.28/ns-3.28/swsdefine/tracefile/googlemap.png", 0.0, -1999, 3.2, 3.2, 1);
  // for (uint32_t i=0;i<vehicleNum;i++){
  //  anim.UpdateNodeSize(i, 20.0, 20.0);
  // }
  // for (uint32_t i=0;i<uavNum;i++){
  //  anim.UpdateNodeColor(uavNode.Get(i), 0, 255, 255);
  //  anim.UpdateNodeSize(uavNode.Get(i)->GetId(), 30.0, 30.0);
  // }
  // anim.SetConstantPosition(lteNode.Get(0), 1200, 1000);
  // anim.UpdateNodeColor(lteNode.Get(0), 255, 0, 255);
  // anim.UpdateNodeSize(lteNode.Get(0)->GetId(), 40.0, 40.0);

  // anim.SetConstantPosition(remoteHost, 1000, 800);
  // anim.UpdateNodeSize(remoteHost->GetId(), 30.0, 30.0);
  // anim.UpdateNodeColor(remoteHost, 255, 0, 255);
  // anim.SetConstantPosition(pgw, 900, 900);
  // anim.SetMobilityPollInterval(Seconds(0.2));

  
  // AsciiTraceHelper traceStream;
  // Ptr<OutputStreamWrapper> streamLte = traceStream.CreateFileStream("swsdefine/tracefile/SpaceAirGroundSimpleUAV.tr");
  // internet.EnableAsciiIpv4(streamLte, remoteHost);


  // lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-epc-first");

  // internet.EnablePcapIpv4 ("LenaIpv4-10throughput.pcap", connectedVehicleNode.Get(10)->GetId (), 1, true);

  // ** Usefule trace ------------------------------------------------------------------------------------------------------------------
  // internet.EnableAsciiIpv4("what", calculatedNode);






// Ptr<Socket> recvSockLteCMsgUl;
  // recvSockLteCMsgUl = Socket::CreateSocket (remoteHost, tid);
  // InetSocketAddress sockRemoteLteCMsgUl = InetSocketAddress (internetIpIfacesLte.GetAddress(0), 91);
  // recvSockLteCMsgUl->Bind (sockRemoteLteCMsgUl);
  // recvSockLteCMsgUl->SetRecvCallback (MakeCallback (&ReceivePacketUlControlLte));
  // Ptr<Socket> recvSockSatCMsgUl;
  // recvSockSatCMsgUl = Socket::CreateSocket (gwUsers.Get (0), tid);
  // InetSocketAddress sockRemoteSatCMsgUl = InetSocketAddress (satHelper->GetUserAddress (gwUsers.Get (0)), 92);
  // recvSockSatCMsgUl->Bind (sockRemoteSatCMsgUl);
  // recvSockSatCMsgUl->SetRecvCallback (MakeCallback (&ReceivePacketUlControlSat));
  // sourceSat->SetRecvCallback (MakeCallback (&ReceivePacketUlControlSat));
  // std::vector<InetSocketAddress> sockRemoteUavCMsgUlList;
  // for(uint32_t i=0;i<uavNum;i++){
  // 	Ptr<Socket> recvSockUavCMsgUl;
  // 	recvSockUavCMsgUl = Socket::CreateSocket (remoteHost, tid);
  // 	InetSocketAddress sockRemoteUavCMsgUl = InetSocketAddress (internetIpIfacesWifiuavList[i].GetAddress(0), 92+i);
  // 	recvSockUavCMsgUl->Bind (sockRemoteUavCMsgUl);
  // 	recvSockUavCMsgUl->SetRecvCallback (MakeCallback (&ReceivePacketUlControlUav));
  // 	sockRemoteUavCMsgUlList.push_back(sockRemoteUavCMsgUl);
  // 	// recvSockPtrListUavCMsgUl[i] = recvSockUavCMsgUl;
  // }
  

  // for(uint32_t i=0;i<vehicleNum;i++){
  //   Ptr<Socket> sourceSockCMsgUl;
  //   sourceSockCMsgUl = Socket::CreateSocket (allConnectedVehicle.Get(i), tid);
  //   sourceSockCMsgUl->Bind();
  //   // Ptr<Socket> sourceSockCMsgUlSat;
  //   // sourceSockCMsgUlSat = Socket::CreateSocket (utUsers.Get(i), tid);
  //   // InetSocketAddress remoteSat = InetSocketAddress (satHelper->GetUserAddress (utUsers.Get (i)), 9);
  //   // sourceSockCMsgUlSat->Bind ();
  //   for (uint32_t k=0;k<conMsgPktIterNum;k++){
  //       Simulator::Schedule(Seconds (conMsgStartTime + k*conMsgPktLongInterval), &GenerateTrafficUlControl,
  //                                 sourceSockCMsgUl, sockRemoteLteCMsgUl, conMsgPktSize, allConnectedVehicle.Get(i));
  //       // Simulator::Schedule(Seconds (conMsgStartTime + k*conMsgPktLongInterval), &GenerateTrafficUlControl,
  //       //                           sourceSockCMsgUlSat, sockRemoteSatCMsgUl, conMsgPktSize, allConnectedVehicle.Get(i));
  //       for(uint32_t iu=0;iu<uavNum;iu++){
  //         Simulator::Schedule(Seconds (conMsgStartTime + k*conMsgPktLongInterval), &GenerateTrafficUlControl,
  //                                   sourceSockCMsgUl, sockRemoteUavCMsgUlList[iu], conMsgPktSize, allConnectedVehicle.Get(i));
  //   	}
  //   }
  // }
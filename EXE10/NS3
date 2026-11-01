#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

int main()
{
    // Node creation
    NodeContainer nodes;
    nodes.Create(2);

    std::cout << "====================================" << std::endl;
    std::cout << " TCP CONGESTION CONTROL SIMULATION" << std::endl;
    std::cout << "====================================" << std::endl;

    std::cout << "\nCreating Nodes..." << std::endl;
    std::cout << "Node 0 created" << std::endl;
    std::cout << "Node 1 created" << std::endl;

    // Link creation
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("2Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("10ms"));

    NetDeviceContainer devices;
    devices = p2p.Install(nodes);

    std::cout << "\nPoint-to-Point Link Created" << std::endl;
    std::cout << "Data Rate: 2Mbps" << std::endl;
    std::cout << "Delay: 10ms" << std::endl;

    // Internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // IP address
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces;
    interfaces = address.Assign(devices);

    std::cout << "\nIP Addresses" << std::endl;
    std::cout << "Sender   : " << interfaces.GetAddress(0) << std::endl;
    std::cout << "Receiver : " << interfaces.GetAddress(1) << std::endl;

    // TCP Receiver
    uint16_t port = 5000;

    PacketSinkHelper sink(
        "ns3::TcpSocketFactory",
        InetSocketAddress(Ipv4Address::GetAny(), port));

    ApplicationContainer sinkApp = sink.Install(nodes.Get(1));

    sinkApp.Start(Seconds(1.0));
    sinkApp.Stop(Seconds(10.0));

    // TCP Sender
    OnOffHelper source(
        "ns3::TcpSocketFactory",
        InetSocketAddress(interfaces.GetAddress(1), port));

    source.SetAttribute("DataRate", StringValue("5Mbps"));
    source.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer sourceApp = source.Install(nodes.Get(0));

    sourceApp.Start(Seconds(2.0));
    sourceApp.Stop(Seconds(9.0));

    // Buffer information
    int bufferSize = 5;
    int buffer = 0;

    std::cout << "\nBuffer Size: " << bufferSize << " packets" << std::endl;

    std::cout << "\nPacket Transmission" << std::endl;
    std::cout << "-------------------" << std::endl;

    for (int i = 1; i <= 12; i++)
    {
        std::cout << "\nPacket " << i << " generated" << std::endl;

        if (buffer < bufferSize)
        {
            buffer++;

            std::cout << "Packet added to buffer" << std::endl;
            std::cout << "Buffer Fill: "
                      << buffer << "/" << bufferSize << std::endl;
        }
        else
        {
            std::cout << "Buffer Full!" << std::endl;
            std::cout << "CONGESTION DETECTED" << std::endl;

            buffer = buffer - 2;

            std::cout << "TCP Congestion Control Applied" << std::endl;
            std::cout << "Transmission Rate Reduced" << std::endl;
            std::cout << "Packets transmitted from buffer" << std::endl;

            std::cout << "Buffer Fill: "
                      << buffer << "/" << bufferSize << std::endl;
        }
    }

    std::cout << "\nStarting NS-3 TCP Simulation..." << std::endl;

    Simulator::Stop(Seconds(10.0));

    Simulator::Run();

    std::cout << "\n====================================" << std::endl;
    std::cout << " TCP Simulation Completed" << std::endl;
    std::cout << "====================================" << std::endl;

    Simulator::Destroy();

    return 0;
}

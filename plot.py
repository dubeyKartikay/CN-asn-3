import scapy.all as scapy
import matplotlib.pyplot as plt

def analyze_pcap(pcap_file):
    packets = scapy.rdpcap(pcap_file)
    
    throughput = {}
    latency = {}
    for packet in packets:
        if packet.haslayer("TCP"):
            src_ip = packet["IP"].src
            dst_ip = packet["IP"].dst
            src_port = packet["TCP"].sport
            dst_port = packet["TCP"].dport
            if(src_ip != "10.0.2.15" and dst_ip != "10.0.2.15"):
                continue

            # TCP flow identifier
            flow_id = (src_ip, dst_ip, src_port, dst_port)

            # Calculate throughput (bits per second)
            if flow_id in throughput:
                throughput[flow_id] += len(packet)
            else:
                throughput[flow_id] = len(packet)

            # Calculate latency (milliseconds)
            if flow_id in latency:
                latency[flow_id].append(packet.time)
            else:
                latency[flow_id] = [packet.time]

    # Calculate average throughput and average latency for each flow
    avg_throughput = [throughput[flow_id] / (latency[flow_id][-1] - latency[flow_id][0]) * 8 for flow_id in throughput]
    avg_latency =  [(latency[flow_id][-1] - latency[flow_id][0]) * 1000 for flow_id in latency]

    return avg_throughput, avg_latency

def plot_metrics(throughput, latency):
    # Plot the metrics
    plt.figure()
    plt.plot(throughput, label="Throughput (bps)")
    plt.xlabel("Packet")
    plt.ylabel("Throughput")
    plt.title(f"avg throughput")
    plt.legend()
    plt.show()
    plt.figure()
    plt.plot(latency, label="Latency")
    plt.xlabel("Packet")
    plt.ylabel("Latency")
    plt.title(f"Latency")
    plt.legend()
    plt.show()

if __name__ == "__main__":
    pcap_file = "epoll_b.pcap"  # Replace with your pcap file path
    avg_throughput, avg_latency = analyze_pcap(pcap_file)
    # print(avg_throughput)
    # print(avg_latency)
    plot_metrics(avg_throughput, avg_latency)

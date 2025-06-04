# https://g.co/gemini/share/cc5b6f4297ad
from scapy.all import *
import pandas as pd

RATE = 3

def validate_layer(packet):
    required_layers = [Ether, IP, TCP]
    return all(layer in packet for layer in required_layers)

def update_tcp_flag_dict(packet, syn_count, syn_ack_count):
    if packet[TCP].flags == "S":
        syn_count[packet[IP].src] = syn_count.get(packet[IP].src, 0) + 1
    elif packet[TCP].flags == "SA":
        syn_ack_count[packet[IP].dst] = syn_ack_count.get(packet[IP].dst, 0) + 1

def count_S_SA(packets):
    syn_count = {}
    syn_ack_count = {}
    for p in packets:
        update_tcp_flag_dict(p, syn_count, syn_ack_count)
    return syn_count, syn_ack_count

def find_suspicious_ips(syn_counts, synack_counts):
    suspicious_ips = []
    for ip in syn_counts.keys():
        syn_sent = syn_counts[ip]
        synack_received = synack_counts.get(ip, 0) 

        if syn_sent > synack_received * RATE:
            suspicious_ips.append((ip, syn_sent, synack_received, syn_sent - synack_received))
    return suspicious_ips

def display_suspicious_ips(suspicious_list):
    df = pd.DataFrame(suspicious_list, columns=['IP', 'SYN_sent', 'SYN_ACK_received', 'Difference'])
    df.sort_values(by='Difference', ascending=False, inplace=True)
    print("\nSuspicious IPs:")
    print(df.to_string(index=False))
    print(f"\nTotal suspicious IPs found: {len(suspicious_list)}")


def main():
    if len(sys.argv) != 2:
        print("Usage: python syn_scanner.py <pcap_file>")
        return

    pcap_file = sys.argv[1]

    print(f"Reading {pcap_file}...")
    all_packets = rdpcap(pcap_file)
    print(f"Total packets read: {len(all_packets)}")
    
    print("Validating packets...")
    valid_packets = [p for p in all_packets if validate_layer(p)]
    print(f"Valid packets (Ether+IP+TCP): {len(valid_packets)}")
    
    print("Counting SYN and SYN-ACK packets...")
    syn_c, synack_c = count_S_SA(valid_packets)
    
    print("Finding suspicious IPs...")
    suspicious_list = find_suspicious_ips(syn_c, synack_c)
    
    display_suspicious_ips(suspicious_list)

if __name__ == "__main__":
    main()
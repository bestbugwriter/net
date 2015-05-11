int get_target_mac_addr(char *interface_name, u_int target_ip, u_char *target_mac);
int get_ipaddr_macaddr(char *interface_name, u_char *mac);
int get_iface_index(int fd, const char* interface_name);
int recv_arp_response(int sock, u_int target_ip, u_int src_ip, u_char *src_mac, u_char *target_mac);
int send_arp_request(int sock, u_int target_ip, u_int src_ip, u_char *dst_mac, u_char *src_mac, char *interface_name);
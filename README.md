# Files in the directory #
 --- ON.cpp (setting up the Server)
 --- VN.py  (Creating Virtual nodes with custom ips, server ip and a udp port as argument)

# Commands to run #
First start with the server by making an executable of server file : 
   ```bash
   g++ server.cpp (can create optional executable name with '-o' flag)
   ./a.out <Config.txt> <ON_IP> (ON ip can be edited directly in the code or can pass as argument)
   ```
For the Virtual Nodes :
    Run instances of these based on the number of nodes used in the config (anyhow more than number of nodes from config cannot be allocated to VN nodes)

*  Note1: You should connect the ON and VN machines with the same wifi and add the VN node's ip address which you want to use manually.
* Note2: The VN ip must be in the subnet range of the server ip.

    ```bash
      sudo ip addr add <VN_IP> dev <wifi_interface>
   ```
   ```bash
    python VN.py <ON_IP> <VN_IP> <udp_port_number>
   ```

* Checking results : The final graph (matrix) will be deduced by each VN and prints the graph as a **numpy**(should be availabe to be able to print the graph) matrix in the terminal where it is running
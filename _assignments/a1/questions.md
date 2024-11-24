# Questions

## Q1) In a few sentences, describe the general architecture of your entire layout with the network monitor and interface monitors. What does each do?

### Answer 01

- **Network Monitor:** The network monitor is the central system that coordinates and manages multiple interface monitors. It listens for user input, queries for the number and names of network interfaces, and starts interface monitor processes. It controls the monitoring process by instructing the interface monitors to monitor specific statistics at regular intervals. It also handles link-down events, sending instructions to bring interfaces back up, and manages graceful shutdowns.
- **Interface Monitors:** Each network interface has a corresponding interface monitor. These monitors are responsible for reading statistics from the `/sys/class/net/<interface-name>` directory, such as received and transmitted bytes, errors, and link state. If an interface goes down, the monitor sends a message to the network monitor, which instructs it to bring the link back up. The interface monitor also manages graceful shutdowns when it receives a shutdown signal from the network monitor.

## Q2) Could the interface monitor and network monitor all be contained within one process, if so how?

### Answer 02

Yes, it is possible for the interface monitor and network monitor to be contained within one process, though it might not be the most efficient solution. In this case, the network monitor could spawn separate threads for each interface, allowing each thread to independently monitor its respective network interface and report back to the main network monitor. However, using multiple processes allows for better isolation, scalability, and resource management.

## Q3) Could the interface monitor and network monitor all be contained within one process, assuming 128 network interfaces running at several Giga-bits per second, which require a polling interval of one millisecond per interface

### Answer 03

No, running 128 network interfaces at several Gbps with a polling interval of one millisecond per interface within a single process would be extremely inefficient and likely impractical. Each polling operation would need to be handled in real time, which would overwhelm a single process due to the high concurrency and the need for precise timing. A multi-process or multi-threaded approach is necessary to distribute the load efficiently and ensure the system can scale to handle the volume of traffic and the required polling frequency.

## Q4) What is a software defined network? In doing so, describe the applications layer, the control layer, and the forwarding layer

### Answer 04

- A **Software Defined Network (SDN)** is a networking approach that centralizes network control in a software-based controller, decoupling the control plane (which makes decisions about where traffic is sent) from the data plane (which forwards traffic). SDN enables more flexible and efficient management of network resources.
  - **Applications Layer:** This layer is where the network applications reside, interacting with the network to provide services such as monitoring, security, and optimization. It uses the SDN controller to communicate network requirements and receive network data.
  - **Control Layer:** The control layer is the brain of the SDN, where the SDN controller resides. This layer is responsible for making decisions about how data should flow through the network based on a global view of the network's state. The controller programs the network devices (such as switches and routers) to forward traffic according to these decisions.
  - **Forwarding Layer:** This is the layer where the actual physical network devices, such as switches and routers, reside. These devices forward data packets based on instructions provided by the control layer. The devices are simplified as they rely on the SDN controller for decision-making, enabling more efficient and flexible networking.

In SDN, the focus is on programmability and centralized control, allowing networks to be dynamically adjusted to meet changing needs and conditions, such as traffic loads or security requirements.

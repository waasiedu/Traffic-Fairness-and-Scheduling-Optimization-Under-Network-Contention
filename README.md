# ByteBalancer: Weighted Round Robin (WRR) Scheduler in srsRAN 5G

I built this project because *equal-share scheduling is polite, but not always useful* in real 5G systems.

In a practical lab setup, not all UEs are equal:
- some generate low-rate background traffic,
- others are latency-sensitive,
- and some demand sustained high throughput.

Classic **Round Robin (RR)** scheduling treats all UEs equally, which is fair — but fairness does not always satisfy **QoS intent**.  
This project implements **Weighted Round Robin (WRR)** inside the **srsRAN 5G MAC scheduler**, allowing selected UEs to receive proportionally more scheduling opportunities based on assigned weights.

The result is a scheduler that is:
- simple,
- deterministic,
- easy to reason about,
- and well-suited for experimental 5G research and teaching.

---

## One-minute overview

### Round Robin (RR)
Each UE gets one scheduling opportunity per cycle.

### Weighted Round Robin (WRR)
Each UE gets *weight* scheduling opportunities per cycle.

Example:
- UE₀ weight = 1
- UE₁ weight = 2
- UE₂ weight = 4

Under contention, UE₂ will receive roughly **4× more allocation attempts** than UE₀.

This aligns scheduling behavior with throughput or priority requirements while preserving predictable fairness.

---

## System architecture

**Radio Access Network**
- srsRAN gNB
- USRP (UHD driver)

**Core Network**
- Open5GS (AMF, SMF, UPF)

**Traffic**
- iperf3 UDP flows from UEs through the core network

High-level flow:
1. UEs attach and receive IP addresses.
2. UEs start traffic flows.
3. MAC scheduler iterates over active UEs.
4. WRR allocates resources based on UE weights.
5. Throughput shifts toward higher-weighted UEs as load increases.



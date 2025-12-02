# DMA Testing - Quick Reference Card

## ? Quick Commands

### Setup (One-Time)
```powershell
# Download PCILeech
.\scripts\Setup-PCILeech.ps1

# Navigate to PCILeech
cd C:\Tools\PCILeech
```

### Essential Tests
```powershell
# 1. Probe for device
.\pcileech.exe probe

# 2. Quick benchmark
.\pcileech.exe benchmark

# 3. Stability test
.\pcileech.exe testmemread -min 0x1000

# 4. Memory display
.\pcileech.exe display -min 0x1000
```

---

## ?? Expected Results (XC7A100T)

| Test | Metric | Elite | Amazing | Good | Warning | Low |
|------|--------|-------|---------|------|---------|-----|
| **Speed** | RPS | >7,500 | 6,500-7,500 | 5,200-6,500 | 4,000-5,200 | <4,000 |
| **Throughput** | MB/s | >220 | 200-220 | 150-200 | 125-150 | <125 |
| **Latency** | µs | <130 | 130-150 | 150-190 | 190-250 | >250 |
| **Stability** | Error % | 0% | 0% | 0% | <0.1% | >0.1% |

**Rating Guide:**
- ? **ELITE:** Competition-grade (Top 1%)
- ?? **AMAZING:** Professional-grade (Top 5%)
- ? **GOOD:** Above average
- ?? **WARNING:** Below expected
- ? **LOW:** Needs troubleshooting

---

## ?? Common Issues

### Issue: Device not found
```
? Check USB cable (blue USB 3.0 port)
? Install FTDI drivers
? Run as Administrator
```

### Issue: Low performance
```
? Try different USB 3.0 port
? Update USB controller drivers
? Disable VT-d/IOMMU in BIOS
```

### Issue: Intermittent errors
```
? Check power connections
? Re-flash FPGA firmware
? Try different memory range
```

---

## ?? Documentation

| Topic | Document |
|-------|----------|
| **Testing Guide** | `docs/DMA_TESTING_GUIDE.md` |
| **Benchmarking** | `docs/DMA_BENCHMARKING_GUIDE.md` |
| **Implementation** | `docs/BENCHMARK_TAB_SPEC.md` |
| **Troubleshooting** | `docs/TROUBLESHOOTING.md` |

---

## ?? Get Help

- ?? **Discord:** https://discord.gg/MfH9UHxkdP
- ?? **Docs:** docs/DMA_BENCHMARKING_GUIDE.md
- ?? **Issues:** https://github.com/ufrisk/pcileech/issues

---

**Quick Tip:** Save this card for easy reference during testing!

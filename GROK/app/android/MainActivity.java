/**
 * TBOS Hardware Detector - Android Java Wrapper
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

package com.tbos.detector;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.os.Build;
import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;

public class MainActivity extends Activity {

    // Native method declarations
    static {
        System.loadLibrary("tbos_detector");
    }

    private native String detectHardwareNative();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView resultText = findViewById(R.id.resultText);

        // Get hardware profile
        HardwareProfile hw = detectHardware();

        // Display results
        StringBuilder sb = new StringBuilder();
        sb.append("╔══════════════════════════════════════╗\n");
        sb.append("║  TBOS Hardware Detection (Android)  ║\n");
        sb.append("║  🕉️ Swamiye Saranam Aiyappa 🕉️      ║\n");
        sb.append("╚══════════════════════════════════════╝\n\n");

        sb.append("=== DEVICE INFORMATION ===\n");
        sb.append("Manufacturer: ").append(Build.MANUFACTURER).append("\n");
        sb.append("Model: ").append(Build.MODEL).append("\n");
        sb.append("Android Version: ").append(Build.VERSION.RELEASE).append("\n\n");

        sb.append("=== CPU INFORMATION ===\n");
        sb.append("Vendor: ").append(hw.cpuVendor).append("\n");
        sb.append("Cores: ").append(hw.cpuCores).append("\n");
        sb.append("Architecture: ").append(hw.architecture).append("\n\n");

        sb.append("=== MEMORY INFORMATION ===\n");
        sb.append("Total RAM: ").append(hw.totalMemoryMB).append(" MB\n");
        sb.append("Total RAM: ").append(String.format("%.2f", hw.totalMemoryMB / 1024.0)).append(" GB\n\n");

        sb.append("=== DEVICE CLASSIFICATION ===\n");
        sb.append("Device Class: ").append(getDeviceClassName(hw.deviceClass)).append("\n");
        sb.append("Class ID: ").append(hw.deviceClass).append("\n\n");

        sb.append("=== RECOMMENDED OS ===\n");
        sb.append(getRecommendedOS(hw.deviceClass)).append("\n");

        resultText.setText(sb.toString());
    }

    /**
     * Detect hardware using Java APIs
     */
    private HardwareProfile detectHardware() {
        HardwareProfile hw = new HardwareProfile();

        // Get CPU information
        hw.cpuVendor = getCPUVendor();
        hw.cpuCores = Runtime.getRuntime().availableProcessors();
        hw.architecture = Build.CPU_ABI;

        // Get memory information
        android.app.ActivityManager activityManager =
            (android.app.ActivityManager) getSystemService(ACTIVITY_SERVICE);
        android.app.ActivityManager.MemoryInfo memInfo =
            new android.app.ActivityManager.MemoryInfo();
        activityManager.getMemoryInfo(memInfo);

        hw.totalMemoryMB = memInfo.totalMem / (1024 * 1024);

        // Classify device
        hw.deviceClass = classifyDevice(hw.totalMemoryMB);

        return hw;
    }

    /**
     * Get CPU vendor from /proc/cpuinfo
     */
    private String getCPUVendor() {
        try {
            BufferedReader br = new BufferedReader(new FileReader("/proc/cpuinfo"));
            String line;
            while ((line = br.readLine()) != null) {
                if (line.startsWith("Hardware")) {
                    String[] parts = line.split(":");
                    if (parts.length > 1) {
                        String hardware = parts[1].trim();
                        if (hardware.contains("Qualcomm") || hardware.contains("Snapdragon")) {
                            return "Qualcomm";
                        } else if (hardware.contains("Exynos") || hardware.contains("Samsung")) {
                            return "Samsung";
                        } else if (hardware.contains("MediaTek") || hardware.contains("MTK")) {
                            return "MediaTek";
                        } else if (hardware.contains("Kirin")) {
                            return "HiSilicon";
                        }
                        return hardware;
                    }
                }
            }
            br.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return "ARM";
    }

    /**
     * Classify device based on memory (same algorithm as other platforms)
     */
    private int classifyDevice(long memoryMB) {
        if (memoryMB < 1) return 0;           // CALCULATOR
        if (memoryMB < 64) return 1;          // EMBEDDED
        if (memoryMB < 4 * 1024) return 2;    // MOBILE
        if (memoryMB < 16 * 1024) return 3;   // DESKTOP
        if (memoryMB < 32 * 1024) return 4;   // WORKSTATION
        if (memoryMB < 64 * 1024) return 5;   // SERVER
        if (memoryMB < 128 * 1024) return 6;  // CLUSTER
        return 7;                              // SUPERCOMPUTER
    }

    /**
     * Get device class name
     */
    private String getDeviceClassName(int deviceClass) {
        String[] names = {
            "CALCULATOR (512B-2KB)",
            "EMBEDDED (2KB-64KB)",
            "MOBILE (64KB-4MB)",
            "DESKTOP (4MB-16MB)",
            "WORKSTATION (16MB-32MB)",
            "SERVER (32MB-64MB)",
            "CLUSTER (64MB-128MB)",
            "SUPERCOMPUTER (128MB+)"
        };
        return names[deviceClass];
    }

    /**
     * Get recommended OS
     */
    private String getRecommendedOS(int deviceClass) {
        switch (deviceClass) {
            case 0: return "Recommended: TinyOS / Custom Embedded OS";
            case 1: return "Recommended: Alpine Linux Embedded";
            case 2: return "Recommended: Android / Alpine Linux Mobile";
            case 3: return "Recommended: Ubuntu Touch / LineageOS";
            case 4: return "Recommended: Desktop Linux / Chrome OS";
            default: return "Recommended: Server/Cluster OS";
        }
    }

    /**
     * Hardware profile data class
     */
    private static class HardwareProfile {
        String cpuVendor;
        int cpuCores;
        String architecture;
        long totalMemoryMB;
        int deviceClass;
    }
}

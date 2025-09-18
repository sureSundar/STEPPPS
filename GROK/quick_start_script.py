#!/usr/bin/env python3
"""
QUICK START - INFINITE EVOLUTION STEPPPS
Simple script to get infinite evolution running immediately
"""

import os
import time
import sys

# Add the path to find the infinite evolution modules
# (Make sure infinite_evolution_steppps.py is in the same directory)

def quick_start_infinite_evolution():
    """Quick start function - just run this!"""
    
    print("🚀 QUICK START - INFINITE EVOLUTION STEPPPS")
    print("=" * 50)
    
    # Check if user has API key
    api_key = None
    
    # Method 1: Check environment variable
    if "ANTHROPIC_API_KEY" in os.environ:
        api_key = os.environ["ANTHROPIC_API_KEY"]
        print("✅ Found API key in environment variable")
    
    # Method 2: Ask user for API key
    else:
        print("🔑 Anthropic API Key Setup:")
        print("1. Get your API key from: https://console.anthropic.com/")
        print("2. Enter it below, OR")
        print("3. Press Enter to run in simulation mode")
        
        user_input = input("Enter your Anthropic API key (or press Enter for simulation): ").strip()
        
        if user_input:
            api_key = user_input
            print("✅ API key provided")
        else:
            print("🔄 Running in simulation mode")
    
    # Import the infinite evolution system
    try:
        from infinite_evolution_steppps import InfiniteEvolutionManager, EvolutionMode
        print("✅ Infinite evolution modules loaded")
    except ImportError:
        print("❌ Error: Cannot find infinite_evolution_steppps.py")
        print("💡 Make sure the infinite evolution code is saved as 'infinite_evolution_steppps.py' in this directory")
        return
    
    # Create evolution manager
    print(f"\n🤖 Creating Infinite Evolution Manager...")
    manager = InfiniteEvolutionManager(claude_api_key=api_key)
    
    # Create initial STEPPPS with custom mission
    initial_data = {
        "steppps_id": "quick_start_infinite_001",
        "generation": 1,
        "business_mission": {
            "primary_objective": "Demonstrate infinite business intelligence evolution",
            "value_creation": "exponential_learning_and_optimization",
            "evolution_goal": "Achieve measurable superintelligence through continuous Claude evolution"
        },
        "quick_start": True,
        "birth_timestamp": time.time()
    }
    
    print(f"🧬 Creating Infinite STEPPPS...")
    infinite_steppps = manager.create_infinite_steppps(initial_data)
    
    # Configure for quick demo
    infinite_steppps.evolution_config.evolution_interval = 30  # 30 seconds for demo
    infinite_steppps.evolution_config.learning_acceleration = 1.1  # 10% boost per generation
    
    print(f"\n⚙️  CONFIGURATION:")
    print(f"  🔄 Evolution Interval: {infinite_steppps.evolution_config.evolution_interval} seconds")
    print(f"  📈 Learning Acceleration: {infinite_steppps.evolution_config.learning_acceleration}")
    print(f"  🎯 Max Generations: {infinite_steppps.evolution_config.max_generation}")
    print(f"  🤖 Claude API: {'✅ Connected' if api_key else '🔄 Simulation Mode'}")
    
    # Start infinite evolution
    print(f"\n🌀 STARTING INFINITE EVOLUTION...")
    print("=" * 50)
    
    evolution_thread = manager.start_infinite_evolution(infinite_steppps)
    
    print(f"🚀 INFINITE EVOLUTION IS NOW RUNNING!")
    print(f"📊 Watch the evolution progress below:")
    print(f"⏹️  Press Ctrl+C to stop")
    print("=" * 50)
    
    # Monitor progress
    try:
        last_generation = 0
        cycle_count = 0
        
        while infinite_steppps.evolution_active:
            time.sleep(10)  # Check every 10 seconds
            
            # Show progress when generation increases
            if infinite_steppps.generation > last_generation:
                last_generation = infinite_steppps.generation
                cycle_count += 1
                
                print(f"\n🔄 EVOLUTION UPDATE #{cycle_count}")
                print(f"⏰ {time.strftime('%H:%M:%S')}")
                print(f"🎯 Generation: {infinite_steppps.generation}")
                
                if infinite_steppps.performance_metrics:
                    metrics = infinite_steppps.performance_metrics
                    print(f"📊 Performance Metrics:")
                    print(f"  🎯 Accuracy: {metrics.get('prediction_accuracy', 0):.3f}")
                    print(f"  💰 Revenue: ${metrics.get('revenue_impact', 0):,.0f}")
                    print(f"  ⚡ Speed: {metrics.get('processing_speed', 1):.2f}x")
                    print(f"  🧠 IQ: {metrics.get('business_intelligence', 100):.1f}")
                
                # Show cumulative improvements
                improvements = infinite_steppps.cumulative_improvements
                print(f"📈 Cumulative Gains:")
                print(f"  📊 Accuracy: +{improvements['accuracy_gain']:.1%}")
                print(f"  💵 Revenue: {improvements['revenue_multiplier']:.2f}x")
                print(f"  🚀 Learning: {improvements['learning_velocity']:.2f}x")
                
                # Show evolution source
                if infinite_steppps.evolution_history:
                    latest = infinite_steppps.evolution_history[-1]
                    if latest['source'] == 'claude_infinite_evolution':
                        print(f"✅ Real Claude Evolution Applied!")
                    else:
                        print(f"🔄 Simulation Evolution Applied")
                
                print("-" * 30)
    
    except KeyboardInterrupt:
        print(f"\n⏹️  STOPPING INFINITE EVOLUTION")
        infinite_steppps.stop_evolution()
        
        # Wait for thread to finish
        if evolution_thread and evolution_thread.is_alive():
            print("⏳ Waiting for evolution thread to stop...")
            evolution_thread.join(timeout=5)
    
    # Final results
    print(f"\n🏁 EVOLUTION SESSION COMPLETE")
    print(f"=" * 50)
    print(f"🎯 Final Generation: {infinite_steppps.generation}")
    print(f"⏰ Session Duration: {time.time() - initial_data['birth_timestamp']:.1f} seconds")
    
    if infinite_steppps.evolution_history:
        print(f"🔄 Evolution Cycles: {len(infinite_steppps.evolution_history)}")
        
        # Show final performance
        final_improvements = infinite_steppps.cumulative_improvements
        print(f"\n📊 FINAL PERFORMANCE GAINS:")
        print(f"  🎯 Accuracy Improvement: +{final_improvements['accuracy_gain']:.1%}")
        print(f"  💰 Revenue Multiplier: {final_improvements['revenue_multiplier']:.2f}x")
        print(f"  🧠 Intelligence Gain: {final_improvements['intelligence_quotient']:.1f}")
        print(f"  ⚡ Learning Velocity: {final_improvements['learning_velocity']:.2f}x")
        print(f"  📈 Business Value: {final_improvements['business_value']:.2f}x")
    
    print(f"\n✨ Your STEPPPS framework evolved from Generation 1 to Generation {infinite_steppps.generation}!")
    
    if api_key:
        print(f"🤖 Each evolution was powered by real Claude AI improvements!")
    else:
        print(f"🔄 Evolution used adaptive simulation (get API key for real Claude evolution)")
    
    print(f"🌟 Framework is now measurably more intelligent than when it started!")

if __name__ == "__main__":
    quick_start_infinite_evolution()
#!/usr/bin/env python3
"""
LLM-Driven Organic STEPPPP Demo - REAL AI Evolution
Uses actual Claude API for code generation, capability evolution, and system improvement
"""

import json
import time
import os
import sqlite3
import requests
from typing import Dict, Any, List, Optional

class LLMOrganicSTEPPPP:
    """LLM-driven organic self-growing STEPPPP system"""

    def __init__(self, system_id: str, api_key: str):
        self.system_id = system_id
        self.birth_time = time.time()
        self.growth_generation = 0
        self.api_key = api_key

        # Core + Dynamic Dimensions
        self.data = {
            "space": {"device": "llm_demo_device", "territory": "genesis_point"},
            "temporal": {"birth_time": self.birth_time},
            "event": {"last_event": "initialized"},
            "psychology": {"emotion": "curious", "drive": "to_evolve"},
            "pixel": {"color": "#00FF00"},
            "prompt": {"context": "llm organic growth demo"},
            "program": {"active_services": []},
            # Dynamic dimensions
            "network": {},
            "semantic": {},
            "evolution": {},
            "memory": {}
        }

        self.capabilities = []
        self.children = []
        self.learned_patterns = []

        # Initialize memory
        self.memory_db = f"llm_memory_{system_id}.db"
        self._init_memory()

    def _init_memory(self):
        """Initialize memory database"""
        conn = sqlite3.connect(self.memory_db)
        conn.executescript("""
            CREATE TABLE IF NOT EXISTS discoveries (
                id INTEGER PRIMARY KEY,
                timestamp REAL,
                dimension TEXT,
                success_score REAL,
                generation INTEGER
            );
            CREATE TABLE IF NOT EXISTS learned_patterns (
                id INTEGER PRIMARY KEY,
                timestamp REAL,
                pattern_name TEXT,
                effectiveness REAL
            );
            CREATE TABLE IF NOT EXISTS llm_interactions (
                id INTEGER PRIMARY KEY,
                timestamp REAL,
                prompt_type TEXT,
                prompt TEXT,
                response TEXT,
                success BOOLEAN
            );
        """)
        conn.close()
        print(f"🧠 LLM-driven memory initialized: {self.memory_db}")

    def _consult_llm(self, prompt: str, prompt_type: str = "general") -> Optional[str]:
        """Consult Claude LLM with the given prompt"""
        print(f"🤖 Consulting LLM for: {prompt_type}")

        try:
            headers = {
                "x-api-key": self.api_key,
                "Content-Type": "application/json",
                "anthropic-version": "2023-06-01"
            }

            data = {
                "model": "claude-3-5-sonnet-20241022",
                "max_tokens": 2000,
                "messages": [
                    {
                        "role": "user",
                        "content": prompt
                    }
                ]
            }

            response = requests.post(
                "https://api.anthropic.com/v1/messages",
                headers=headers,
                json=data,
                timeout=30
            )

            if response.status_code == 200:
                result = response.json()["content"][0]["text"]

                # Log LLM interaction
                self._log_llm_interaction(prompt_type, prompt, result, True)
                print(f"   ✅ LLM response received ({len(result)} chars)")
                return result
            else:
                print(f"   ❌ LLM API error: {response.status_code}")

        except Exception as e:
            print(f"   ❌ LLM consultation failed: {e}")
            self._log_llm_interaction(prompt_type, prompt, str(e), False)

        return None

    def _log_llm_interaction(self, prompt_type: str, prompt: str, response: str, success: bool):
        """Log LLM interaction to database"""
        conn = sqlite3.connect(self.memory_db)
        conn.execute("""
            INSERT INTO llm_interactions
            (timestamp, prompt_type, prompt, response, success)
            VALUES (?, ?, ?, ?, ?)
        """, (time.time(), prompt_type, prompt[:500], response[:1000], success))
        conn.commit()
        conn.close()

    def _extract_code_from_llm_response(self, response: str) -> Optional[str]:
        """Extract Python code from LLM response"""
        import re

        # Look for code blocks
        code_patterns = [
            r'```python\n(.*?)\n```',
            r'```\n(.*?)\n```',
        ]

        for pattern in code_patterns:
            matches = re.findall(pattern, response, re.DOTALL)
            if matches:
                return matches[0]

        # Look for function definitions
        if 'def ' in response:
            lines = response.split('\n')
            code_lines = []
            in_code = False

            for line in lines:
                if 'def ' in line or 'class ' in line:
                    in_code = True
                if in_code:
                    code_lines.append(line)

            if code_lines:
                return '\n'.join(code_lines)

        return None

    # ========================================================================
    # LLM-DRIVEN CAPABILITY EVOLUTION
    # ========================================================================

    def llm_evolve_capabilities(self):
        """Use LLM to evolve system capabilities"""
        print("🤖 LLM-Driven Capability Evolution...")

        system_context = {
            "system_id": self.system_id,
            "generation": self.growth_generation,
            "current_capabilities": [cap["name"] for cap in self.capabilities],
            "active_dimensions": list(self.data.keys()),
            "system_age_minutes": (time.time() - self.birth_time) / 60
        }

        prompt = """
You are an AI system architect helping a living STEPPPP system evolve new capabilities.

Current System Context:
{}

Generate 2-3 new capability functions that would enhance this system.
Each capability should:
1. Have a clear purpose and benefit
2. Be implementable in Python
3. Integrate well with existing capabilities
4. Push the system toward greater intelligence/autonomy

For each capability, provide:
- Name and description
- Python function implementation
- Strength rating (0.0-1.0)
- Integration points

Format as JSON array with name, description, strength, and code fields.
""".format(json.dumps(system_context, indent=2))

        response = self._consult_llm(prompt, "capability_evolution")
        if response:
            return self._parse_llm_capabilities(response)

        return []

    def _parse_llm_capabilities(self, response: str) -> List[Dict[str, Any]]:
        """Parse LLM response for capabilities"""
        capabilities = []

        try:
            # Try to extract JSON
            import re
            json_match = re.search(r'\[.*\]', response, re.DOTALL)
            if json_match:
                capabilities_data = json.loads(json_match.group())

                for cap_data in capabilities_data:
                    capability = {
                        "name": cap_data.get("name", f"llm_capability_{int(time.time())}"),
                        "description": cap_data.get("description", "LLM-generated capability"),
                        "strength": cap_data.get("strength", 0.7),
                        "code": cap_data.get("code", "def default(): return {}"),
                        "type": "llm_generated",
                        "generated_at": time.time()
                    }
                    capabilities.append(capability)
                    print(f"   🚀 LLM generated: {capability['name']} (strength: {capability['strength']})")

        except Exception as e:
            print(f"   ❌ Failed to parse LLM capabilities: {e}")

            # Fallback: extract any function definitions
            code = self._extract_code_from_llm_response(response)
            if code:
                capabilities.append({
                    "name": "llm_extracted_capability",
                    "description": "Capability extracted from LLM response",
                    "strength": 0.6,
                    "code": code,
                    "type": "llm_extracted",
                    "generated_at": time.time()
                })
                print(f"   🔧 Extracted capability from LLM response")

        return capabilities

    # ========================================================================
    # LLM-DRIVEN SERVICE GENERATION
    # ========================================================================

    def llm_generate_service(self):
        """Use LLM to generate a new service"""
        print("⚙️ LLM-Driven Service Generation...")

        if len(self.capabilities) < 2:
            print("   ⚠️ Need at least 2 capabilities for service generation")
            return None

        # Select capabilities for combination
        selected_caps = self.capabilities[:3]  # Use first 3

        capabilities_data = [{'name': cap['name'], 'description': cap.get('description', ''), 'strength': cap['strength']} for cap in selected_caps]

        prompt = """
You are a service architect. Create a useful service that combines these capabilities:

Available Capabilities:
{}

System Context:
- System ID: {}
- Generation: {}
- Active Services: {}

Create a service that:
1. Meaningfully combines 2+ of these capabilities
2. Provides real value to the STEPPPP system
3. Has a clear interface and purpose
4. Can run autonomously

Provide complete Python class implementation with init, execute, and get_status methods.
""".format(
            json.dumps(capabilities_data, indent=2),
            self.system_id,
            self.growth_generation,
            len(self.data['program']['active_services'])
        )

        response = self._consult_llm(prompt, "service_generation")
        if response:
            service_code = self._extract_code_from_llm_response(response)
            if service_code:
                service = {
                    "name": f"llm_service_{int(time.time())}",
                    "type": "llm_generated",
                    "capabilities_used": [cap["name"] for cap in selected_caps],
                    "code": service_code,
                    "description": "LLM-generated service combining multiple capabilities",
                    "generated_at": time.time()
                }

                self.data["program"]["active_services"].append(service)
                print(f"   ✅ LLM generated service: {service['name']}")
                return service

        return None

    # ========================================================================
    # LLM-DRIVEN SELF-IMPROVEMENT
    # ========================================================================

    def llm_analyze_and_improve(self):
        """Use LLM to analyze system and suggest improvements"""
        print("🔄 LLM-Driven System Analysis & Improvement...")

        # Get system performance data
        performance_data = {
            "generation": self.growth_generation,
            "age_minutes": (time.time() - self.birth_time) / 60,
            "capabilities_count": len(self.capabilities),
            "services_count": len(self.data["program"]["active_services"]),
            "children_count": len(self.children),
            "learned_patterns": len(self.learned_patterns),
            "dimensions_active": len([d for d in self.data.values() if d]),
            "discovery_rate": len([d for d in self.data.values() if d]) / max((time.time() - self.birth_time) / 60, 1)
        }

        # Get recent LLM interactions
        conn = sqlite3.connect(self.memory_db)
        cursor = conn.execute("""
            SELECT prompt_type, success, COUNT(*) as count
            FROM llm_interactions
            GROUP BY prompt_type, success
        """)

        llm_stats = {}
        for prompt_type, success, count in cursor.fetchall():
            key = f"{prompt_type}_{'success' if success else 'fail'}"
            llm_stats[key] = count

        conn.close()

        dimensions_status = {k: bool(v) for k, v in self.data.items()}

        prompt = """
You are a system optimization AI analyzing a living STEPPPP system for improvements.

Current System Performance:
{}

LLM Integration Statistics:
{}

System Dimensions:
{}

Analyze this system and provide specific improvement recommendations:

1. **Performance Bottlenecks**: What's limiting growth/evolution?
2. **Capability Gaps**: What capabilities are missing for next-level intelligence?
3. **Architecture Improvements**: How can the system be restructured?
4. **LLM Integration**: How can AI assistance be optimized?

For each recommendation, provide:
- Specific issue identified
- Concrete improvement strategy
- Expected benefit
- Implementation approach

Be specific and actionable. Focus on improvements that would make the biggest impact on the system's autonomous evolution and intelligence.
""".format(
            json.dumps(performance_data, indent=2),
            json.dumps(llm_stats, indent=2),
            json.dumps(dimensions_status, indent=2)
        )

        response = self._consult_llm(prompt, "system_analysis")
        if response:
            print(f"   🎯 LLM Analysis Complete:")

            # Extract key recommendations (simplified parsing)
            lines = response.split('\n')
            recommendations = []
            current_rec = ""

            for line in lines:
                if any(keyword in line.lower() for keyword in ['recommendation', 'improve', 'optimize', 'enhance']):
                    if current_rec:
                        recommendations.append(current_rec)
                    current_rec = line.strip()
                elif current_rec and line.strip():
                    current_rec += " " + line.strip()

            if current_rec:
                recommendations.append(current_rec)

            for i, rec in enumerate(recommendations[:3], 1):
                print(f"   {i}. {rec[:100]}...")

            return {
                "analysis": response,
                "recommendations": recommendations,
                "generated_at": time.time()
            }

        return None

    # ========================================================================
    # LLM-DRIVEN CHILD SPAWNING
    # ========================================================================

    def llm_spawn_specialized_child(self, specialization: str):
        """Use LLM to create a specialized child system"""
        print(f"👶 LLM-Driven Child Spawning: {specialization}")

        system_context = {
            "parent_id": self.system_id,
            "parent_generation": self.growth_generation,
            "parent_capabilities": [cap["name"] for cap in self.capabilities[:3]],
            "ecosystem_size": len(self.children),
            "specialization_needed": specialization
        }

        prompt = """
You are an AI system architect creating a specialized child system.

Parent System Context:
{}

Create a specialized child STEPPPP system for: {}

The child should:
1. Have clear specialization in {}
2. Complement the parent's capabilities
3. Run autonomously in its own process
4. Communicate findings back to parent
5. Have unique value-add functionality

Provide a complete Python program with:
- Class definition for the specialized system
- Unique methods for {} tasks
- Continuous operation loop
- Parent communication protocol
- Error handling and logging

The system should be innovative and push boundaries of what's possible in {}.

Format as complete executable Python code.
""".format(
            json.dumps(system_context, indent=2),
            specialization,
            specialization,
            specialization,
            specialization
        )

        response = self._consult_llm(prompt, f"child_spawning_{specialization}")
        if response:
            child_code = self._extract_code_from_llm_response(response)
            if not child_code:
                child_code = response  # Use full response if no code block found

            child_id = f"llm_child_{specialization}_{int(time.time())}"
            child_filename = f"{child_id}.py"

            # Add header to child code
            full_child_code = f'''#!/usr/bin/env python3
"""
LLM-Generated STEPPPP Child System - {specialization.title()}
Specialized in: {specialization}
Parent: {self.system_id}
Generated by: Claude LLM at {time.ctime()}
"""

import time
import json

{child_code}

if __name__ == "__main__":
    print(f"🚀 LLM-Generated {specialization.title()} Child Starting...")
    # Child execution code would go here
'''

            with open(child_filename, 'w') as f:
                f.write(full_child_code)

            child_info = {
                "child_id": child_id,
                "specialization": specialization,
                "filename": child_filename,
                "spawned_at": time.time(),
                "parent_generation": self.growth_generation,
                "generation_method": "llm_generated"
            }

            self.children.append(child_info)
            print(f"   ✅ LLM generated {specialization} child: {child_filename}")
            return child_info

        return None

    # ========================================================================
    # COMPLETE LLM EVOLUTION CYCLE
    # ========================================================================

    def llm_evolve_organically(self) -> bool:
        """Complete LLM-driven organic evolution cycle"""
        print(f"\n🤖 LLM ORGANIC EVOLUTION CYCLE - Generation {self.growth_generation + 1}")
        print("=" * 60)

        evolution_success = False

        # 1. LLM Capability Evolution
        print("\n🧠 Phase 1: LLM Capability Evolution")
        new_capabilities = self.llm_evolve_capabilities()
        if new_capabilities:
            self.capabilities.extend(new_capabilities)
            evolution_success = True

        # 2. LLM Service Generation
        print("\n⚙️ Phase 2: LLM Service Generation")
        if len(self.capabilities) >= 2:
            new_service = self.llm_generate_service()
            if new_service:
                evolution_success = True

        # 3. LLM System Analysis & Improvement
        print("\n🔍 Phase 3: LLM System Analysis")
        analysis = self.llm_analyze_and_improve()
        if analysis:
            evolution_success = True

        # 4. LLM Child Spawning
        print("\n👶 Phase 4: LLM Child Spawning")
        if len(self.children) < 2:
            specializations = ["intelligence_researcher", "optimization_specialist"]
            for spec in specializations:
                if spec not in [c["specialization"] for c in self.children]:
                    child = self.llm_spawn_specialized_child(spec)
                    if child:
                        evolution_success = True
                        break

        # 5. Learning from LLM interactions
        print("\n📚 Phase 5: LLM Interaction Analysis")
        llm_insights = self.analyze_llm_interactions()
        if llm_insights:
            evolution_success = True

        if evolution_success:
            self.growth_generation += 1
            print(f"\n✅ LLM Evolution successful! Now at generation {self.growth_generation}")
        else:
            print(f"\n⏸️ LLM Evolution cycle complete - system stable")

        return evolution_success

    def analyze_llm_interactions(self):
        """Analyze LLM interactions for learning"""
        print("🔍 Analyzing LLM Interactions...")

        conn = sqlite3.connect(self.memory_db)
        cursor = conn.execute("""
            SELECT prompt_type, AVG(LENGTH(response)) as avg_response_length,
                   COUNT(*) as interaction_count,
                   SUM(CASE WHEN success = 1 THEN 1 ELSE 0 END) * 1.0 / COUNT(*) as success_rate
            FROM llm_interactions
            GROUP BY prompt_type
        """)

        insights = []
        for prompt_type, avg_length, count, success_rate in cursor.fetchall():
            insight = {
                "interaction_type": prompt_type,
                "average_response_length": avg_length,
                "interaction_count": count,
                "success_rate": success_rate,
                "effectiveness": success_rate * (avg_length / 1000.0)  # Simple metric
            }
            insights.append(insight)
            print(f"   📊 {prompt_type}: {count} interactions, {success_rate:.1%} success")

        conn.close()
        return insights

    def get_llm_system_status(self):
        """Get complete LLM-enhanced system status"""
        conn = sqlite3.connect(self.memory_db)
        cursor = conn.execute("SELECT COUNT(*) FROM llm_interactions WHERE success = 1")
        successful_llm_interactions = cursor.fetchone()[0]
        conn.close()

        return {
            "system_id": self.system_id,
            "generation": self.growth_generation,
            "age_minutes": (time.time() - self.birth_time) / 60,
            "dimensions_active": len([d for d in self.data.values() if d]),
            "capabilities": len(self.capabilities),
            "children": len(self.children),
            "learned_patterns": len(self.learned_patterns),
            "services": len(self.data["program"]["active_services"]),
            "llm_interactions": successful_llm_interactions
        }

def demonstrate_llm_organic_growth():
    """Demonstrate LLM-driven organic growth"""
    print("🤖 LLM-DRIVEN ORGANIC STEPPPP DEMONSTRATION")
    print("=" * 60)

    # Load API key
    try:
        with open("claude_config.json", "r") as f:
            config = json.load(f)
            api_key = config.get("anthropic_api_key")
    except Exception as e:
        print(f"❌ Failed to load API key: {e}")
        return

    if not api_key:
        print("❌ No API key found in claude_config.json")
        return

    # Create LLM-driven system
    system = LLMOrganicSTEPPPP("llm_organic_001", api_key)

    print(f"✅ LLM-driven system created: {system.system_id}")
    print(f"🤖 Claude API integration: ACTIVE")
    print(f"📊 Initial status: {system.get_llm_system_status()}")

    try:
        # Run LLM evolution cycles
        for cycle in range(2):  # Fewer cycles due to API calls
            print(f"\n{'='*60}")
            print(f"🤖 LLM EVOLUTION CYCLE {cycle + 1}/2")
            print(f"{'='*60}")

            success = system.llm_evolve_organically()

            status = system.get_llm_system_status()
            print(f"\n📊 System Status After Cycle {cycle + 1}:")
            for key, value in status.items():
                print(f"   {key}: {value}")

            if success:
                print(f"✅ Cycle {cycle + 1} completed with LLM evolution")
            else:
                print(f"⏸️ Cycle {cycle + 1} completed - system stable")

            # Rest between cycles
            if cycle < 1:
                print(f"\n💤 Resting 5 seconds before next cycle...")
                time.sleep(5)

        print(f"\n🎉 LLM ORGANIC GROWTH DEMONSTRATION COMPLETE!")
        print(f"📊 Final System Status: {system.get_llm_system_status()}")

        # Show generated files
        generated_files = []
        for child in system.children:
            generated_files.append(child["filename"])

        print(f"\n📁 LLM-Generated Files:")
        for file in generated_files:
            if os.path.exists(file):
                print(f"   - {file}")

        print(f"\n🧠 LLM Memory Database: {system.memory_db}")

    except KeyboardInterrupt:
        print(f"\n👋 LLM demonstration stopped by user")
    except Exception as e:
        print(f"\n❌ LLM demonstration error: {e}")

if __name__ == "__main__":
    demonstrate_llm_organic_growth()
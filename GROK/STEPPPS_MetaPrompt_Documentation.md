# STEPPPS Meta-Prompt Engineering Documentation

## Overview

The STEPPPS meta-prompt is the core intelligence engine that transforms natural language descriptions into comprehensive pixel data structures. This document details the prompt engineering techniques and schema explanations used to achieve high-quality AI-generated STEPPPS scenes.

## Meta-Prompt Architecture

### 1. **Role Definition & Context Setting**
```
You are an expert STEPPPS (Spatial Temporal Event Psychology Pixel Prompt Script) generator.
```
- **Purpose**: Establishes AI's identity as a domain expert
- **Effect**: Primes Claude to approach the task with specialized knowledge
- **Best Practice**: Clear, authoritative role definition

### 2. **Schema Deep Dive with Hierarchical Structure**

#### A. **Core Schema Structure**
- **Complete JSON example** with inline comments
- **Visual layout** showing exact format expected
- **Coordinate system explanation** (0 to resolution-1)

#### B. **Detailed Property Explanations**
Organized into 5 logical categories:

**1. SPATIAL PROPERTIES**
- Coordinate system mapping
- Depth layer definitions (0.0-0.3, 0.3-0.7, 0.7-1.0)
- 3D scene construction principles

**2. VISUAL PROPERTIES**
- Color theory application
- Lighting considerations (golden hour, blue hour, etc.)
- Atmospheric perspective effects
- Material property simulation

**3. PSYCHOLOGICAL PROPERTIES**
- 8 distinct emotion categories with clear definitions
- Attention scoring system (0.0-1.0 with specific ranges)
- Visual weight distribution principles

**4. SEMANTIC PROPERTIES**
- Element taxonomy (natural, urban, objects)
- Contextual meaning system
- Role hierarchy (background/midground/foreground/focus)

**5. TECHNICAL PROPERTIES**
- Importance weighting for scene understanding
- AI reasoning requirements for explainability
- Atmospheric and perspective calculations

### 3. **Concrete Examples with Explanations**

#### Sample 1: Sunset Sky Pixel
```json
{
  "x": 32, "y": 10, "z": 0.9,
  "color": "#FF6B35", "brightness": 0.9,
  "emotion": "peaceful", "attention": 0.6,
  "element": "sky", "meaning": "sunset_sky",
  "role": "background", "importance": 0.5,
  "reasoning": "Warm orange sky during golden hour creates peaceful mood",
  "atmospheric": 0.1, "perspective": 1.0
}
```

**Why This Works:**
- **Specific coordinates** show exact placement
- **Color justification** ties to real-world lighting
- **Emotional reasoning** connects color psychology to emotion
- **Technical accuracy** with proper depth and perspective values

#### Sample 2: Sun Center Pixel
- **Maximum attention** (0.95) for primary focal point
- **"energetic" emotion** appropriate for light source
- **Minimal atmospheric effect** (0.0) for bright foreground object
- **High importance** (0.95) reflecting scene significance

#### Sample 3: Ocean Water Pixel
- **"contemplative" emotion** matching water's psychological effect
- **Mid-depth placement** (0.3) for water surface
- **Atmospheric perspective** (0.2) for distance effect
- **Lower attention** (0.4) for supporting element

#### Sample 4: Tree Foreground Pixel
- **Foreground depth** (0.1) for immediate objects
- **Perspective scaling** (0.7) for size adjustment
- **High attention** (0.7) for framing element
- **No atmospheric effect** (0.0) for close objects

### 4. **Comprehensive Generation Guidelines**

#### Scene Analysis Process (6-Step Framework):
1. **Parse prompt** → Extract scene elements, time, weather
2. **Establish lighting** → Determine light sources and quality
3. **Plan composition** → Identify focal points and hierarchy
4. **Map depth layers** → Organize background/midground/foreground
5. **Apply psychology** → Assign emotional resonance
6. **Consider style** → Adjust for artistic interpretation

#### Style Interpretations:
- **Realistic**: Natural color accuracy, proper lighting physics
- **Impressionist**: Soft edges, color mixing, atmospheric effects
- **Abstract**: Bold colors, simplified forms, emotional emphasis
- **Cyberpunk**: Neon palette, high contrast, urban themes
- **Minimalist**: Reduced palette, negative space, simple forms

#### Depth Mode Applications:
- **Atmospheric**: Distance-based color temperature shifts
- **Geometric**: Perspective scaling from center point
- **Layered**: Discrete depth planes with clear separation

### 5. **Task Specification with Clear Requirements**

#### Input Parameters:
- **Prompt**: User's scene description
- **Style**: Artistic interpretation mode
- **Depth Mode**: 3D rendering approach
- **Psychology**: Emotional emphasis direction
- **Resolution**: Pixel grid dimensions

#### Output Requirements:
- **Format**: Valid JSON array only
- **Completeness**: All schema properties required
- **Consistency**: Coherent lighting and style throughout
- **Accuracy**: Semantically correct element identification

### 6. **Success Criteria Checklist**
```
✓ Visual coherence across all pixels
✓ Realistic lighting and shadows
✓ Proper depth and perspective
✓ Emotional resonance matching psychology
✓ Semantic accuracy for all elements
✓ Style consistency throughout scene
```

## Prompt Engineering Techniques Applied

### 1. **Structured Information Hierarchy**
- **Headers and subheaders** for easy navigation
- **Numbered lists** for step-by-step processes
- **Code blocks** for technical specifications
- **Bullet points** for feature lists

### 2. **Concrete Examples Strategy**
- **Multiple diverse samples** covering different scene elements
- **Complete data structures** showing all required fields
- **Explanatory reasoning** for each example choice
- **Progressive complexity** from simple to detailed

### 3. **Constraint Specification**
- **Clear boundaries** for all numeric values
- **Specific formats** (hex colors, coordinate ranges)
- **Required vs. optional** properties clearly marked
- **Output format restrictions** (JSON only, no additional text)

### 4. **Domain Expertise Modeling**
- **Professional terminology** (atmospheric perspective, golden hour)
- **Technical accuracy** (color theory, lighting physics)
- **Artistic knowledge** (composition principles, style characteristics)
- **Psychology integration** (emotional color associations)

### 5. **Context Priming Techniques**
- **Role establishment** at the beginning
- **Mission statement** for clear purpose
- **Success metrics** for quality evaluation
- **Process framework** for systematic approach

### 6. **Error Prevention Strategies**
- **Format specifications** to prevent JSON errors
- **Value range definitions** to prevent invalid data
- **Required field lists** to ensure completeness
- **Example validation** through multiple samples

## Expected Claude Performance

### High-Quality Outputs Should Demonstrate:

1. **Scene Understanding**
   - Accurate interpretation of prompt elements
   - Proper spatial relationship mapping
   - Realistic lighting and shadow placement

2. **Emotional Intelligence**
   - Appropriate emotion assignment by scene area
   - Consistent psychological tone throughout
   - Meaningful attention distribution

3. **Technical Accuracy**
   - Valid JSON structure for all pixels
   - Proper coordinate system usage
   - Realistic depth and perspective values

4. **Artistic Coherence**
   - Style-appropriate color choices
   - Consistent lighting direction
   - Compositionally sound scene layout

5. **Semantic Precision**
   - Accurate element identification
   - Meaningful context descriptions
   - Logical role assignments

## Testing and Validation

### Recommended Test Prompts:

1. **Simple Scenes**: "sunset over ocean"
2. **Complex Scenes**: "bustling city street at night with rain"
3. **Abstract Concepts**: "the feeling of nostalgia"
4. **Style Variations**: "impressionist forest in autumn"
5. **Mood Specifications**: "dramatic mountain storm"

### Quality Metrics:

- **JSON Validity**: 100% parseable output
- **Schema Compliance**: All required fields present
- **Visual Coherence**: Consistent lighting and color temperature
- **Semantic Accuracy**: Correct element identification
- **Emotional Resonance**: Appropriate mood mapping

## Continuous Improvement

### Areas for Enhancement:

1. **Expanded Element Taxonomy**: More specific object categories
2. **Advanced Lighting Models**: Complex multi-source illumination
3. **Dynamic Weather Effects**: Rain, snow, fog integration
4. **Cultural Context Awareness**: Region-specific architectural styles
5. **Temporal Progression**: Time-of-day transition handling

### Feedback Integration:

- **User preference learning** from successful generations
- **Error pattern analysis** from failed outputs
- **Style refinement** based on artistic feedback
- **Performance optimization** for faster generation

---

This meta-prompt represents the culmination of prompt engineering best practices, combining clear instruction, comprehensive examples, and systematic approach to generate high-quality STEPPPS pixel data that truly understands and represents the semantic, emotional, and visual aspects of any described scene.
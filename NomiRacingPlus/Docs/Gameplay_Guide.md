# NIO Racing Plus - Gameplay Guide

> Your complete guide to mastering NIO Racing Plus

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Controls](#controls)
3. [Game Modes](#game-modes)
4. [Vehicles](#vehicles)
5. [Tracks](#tracks)
6. [NOMI Co-Pilot](#nomi-co-pilot)
7. [Tips & Strategies](#tips--strategies)
8. [Achievements](#achievements)

---

## Getting Started

### First Launch

1. **Main Menu**: Start the game and navigate the main menu
2. **Select Vehicle**: Choose your NIO vehicle (EP9, ET7, or ES7)
3. **Select Track**: Pick a track to race on
4. **Select Mode**: Choose your game mode
5. **Start Racing**: Hit the track and compete!

### Quick Race

For immediate action:
1. Click "Quick Race" from the main menu
2. Default settings will be applied
3. Race against 7 AI opponents
4. Complete 3 laps to finish

---

## Controls

### Keyboard Controls

| Action | Key |
|--------|-----|
| **Accelerate** | W |
| **Brake** | S |
| **Steer Left** | A |
| **Steer Right** | D |
| **Handbrake** | Space |
| **Look Back** | C |
| **Change Camera** | V |
| **Horn** | H |
| **Toggle Headlights** | L |
| **Pause** | Escape |

### Gamepad Controls

| Action | Button |
|--------|--------|
| **Accelerate** | Right Trigger |
| **Brake** | Left Trigger |
| **Steer** | Left Stick |
| **Handbrake** | A Button |
| **Look Back** | Right Stick Button |
| **Change Camera** | Y Button |
| **Horn** | B Button |
| **Toggle Headlights** | X Button |
| **Pause** | Start |

### Advanced Controls

- **Trail Braking**: Gradually release brake while turning for better cornering
- **Throttle Control**: Modulate throttle to avoid wheel spin
- **Counter-Steer**: Steer opposite direction during drift to maintain control

### Camera Modes

Cycle through camera views with V (keyboard) or Y (gamepad). Seven modes are available:

| Mode | View | Best For |
|------|------|----------|
| **Chase** | Third-person behind car | General racing, spatial awareness |
| **Hood** | On the hood, forward view | Precision cornering |
| **Cockpit** | Interior dashboard view | Immersion, sim racing |
| **Bumper** | Front bumper, low angle | Speed sensation |
| **Free** | Orbit around car (mouse/stick) | Post-race exploration |
| **Cinematic** | Auto-directed dynamic shots | Spectating, replays |
| **Replay** | Playback with angle presets | Reviewing races |

**Cinematic sub-shots** (auto-cycles during Cinematic mode): Wide Orbit, Low Close-Up, Bird's Eye, Dramatic Low Angle, Tracking Side, Front Tracking, Dutch Angle, Over Shoulder, Helicopter, Static Bumper.

**Dynamic camera behavior**: FOV widens at high speed and during acceleration. Chase camera uses spring dynamics that respond to bumps, collisions, and drift. Camera shake intensity scales with impact force and drift angle.

---

## Game Modes

### Street GT

**Classic street racing**

- **Rules**: N laps, collisions allowed, no penalties
- **Tracks**: City circuits (NIO City Circuit, Shanghai Pudong)
- **Vehicles**: All vehicles available
- **Strategy**: Balance speed with collision avoidance

### NIO Championship

**NIO brand exclusive racing**

- **Rules**: N laps, NIO vehicles only
- **Tracks**: City circuits only
- **Vehicles**: EP9, ET7, ES7 (and ET5 if unlocked)
- **Special**: Battery management crucial, swap stations available
- **Strategy**: Manage battery, use regen braking, plan pit stops

### Baja Rally

**Desert off-road racing**

- **Rules**: Point-to-point, no laps, terrain affects grip
- **Tracks**: Desert Rally
- **Vehicles**: Off-road vehicles
- **Strategy**: Read terrain, manage traction, avoid obstacles

### AI Difficulty Levels

Four difficulty tiers affect all AI opponents:

| Difficulty | Speed | Cornering | Overtaking | Rubber Band | Reaction Time |
|------------|-------|-----------|------------|-------------|---------------|
| **Easy** | 85% | Cautious | Passive | Strong (40%) | 0.5s |
| **Normal** | 95% | Balanced | Moderate | Medium (30%) | 0.3s |
| **Hard** | 100% | Aggressive | Frequent | Light (20%) | 0.15s |
| **Expert** | 100% | Optimal | Very aggressive | None (0%) | 0.08s |

- **Rubber band effect** scales AI speed based on distance to the player -- AI behind you speeds up, AI ahead slows down. Stronger on lower difficulties, disabled on Expert.
- **Path deviation** decreases with difficulty -- Easy AI wanders up to 15% off the ideal line, Expert AI stays within 2%.
- **Braking distance** -- Easy AI brakes 30% earlier than needed, Expert AI brakes 15% later (riskier but faster).

---

## Vehicles

### NIO EP9 (Hypercar)

**The Track Weapon**

| Spec | Value |
|------|-------|
| Power | 1,000 kW (1,360 HP) |
| Torque | 1,480 Nm |
| 0-100 km/h | 2.7 seconds |
| Top Speed | 313 km/h |
| Drive | AWD Quad Motor |
| Mass | 1,735 kg |
| Battery | 70 kWh (427 km range) |
| Regen Strength | 40% |
| Downforce | Up to 2,000 kg |
| Suspension | Double wishbone (front & rear) |
| Brakes | Carbon ceramic |
| Tires | 265/35 R20 (front), 325/30 R21 (rear) |

**Physics notes**:
- Electric motor delivers peak torque from 0 RPM, flat to 4,000 RPM, then linearly decays to zero at 15,000 RPM
- Four independent motors (250 kW each) enable torque vectoring for sharper turn-in
- Highest downforce coefficient of any vehicle (3.5) -- cornering grip improves significantly above 150 km/h
- Lightest vehicle at 1,735 kg -- shortest braking distances

**Driving Tips**:
- Incredible acceleration - use it for overtaking
- High downforce - carry speed through corners
- Aggressive regen braking - lift off early for deceleration
- Best for: Track racing, time attacks

### NIO ET7 (Luxury Sedan)

**The Grand Tourer**

| Spec | Value |
|------|-------|
| Power | 480 kW (653 HP) |
| Torque | 850 Nm |
| 0-100 km/h | 3.8 seconds |
| Top Speed | 250 km/h |
| Drive | AWD Dual Motor |
| Mass | 2,379 kg |
| Battery | 100 kWh (580 km range) |
| Regen Strength | 30% |
| Downforce | Up to 200 kg |
| Suspension | Double wishbone (front), Multi-link (rear) |
| Brakes | Ventilated disc |
| Tires | 245/45 R20 (front & rear) |

**Physics notes**:
- Motor torque peaks at 0 RPM, holds flat to 3,500 RPM, decays to zero at 12,000 RPM
- Dual motor AWD with 240 kW per motor -- less torque vectoring than EP9 but smoother power delivery
- Lowest drag coefficient (0.23) -- most aerodynamic, sustains top speed efficiently
- Longest wheelbase (3,060 mm) -- most stable at high speed, widest turning radius
- Softer suspension damping (0.6) absorbs bumps well but allows more body roll

**Driving Tips**:
- Balanced performance - easy to drive
- Good range - less battery anxiety
- Comfortable handling - forgiving for beginners
- Best for: Street racing, long races

### NIO ES7 (Performance SUV)

**The Versatile Performer**

| Spec | Value |
|------|-------|
| Power | 480 kW (653 HP) |
| Torque | 850 Nm |
| 0-100 km/h | 3.9 seconds |
| Top Speed | 200 km/h |
| Drive | AWD Dual Motor |
| Mass | 2,400 kg |
| Battery | 100 kWh (530 km range) |
| Regen Strength | 25% |
| Downforce | Up to 150 kg |
| Suspension | Double wishbone (front), Multi-link (rear) |
| Brakes | Ventilated disc |
| Tires | 265/45 R21 (front & rear) |

**Physics notes**:
- Motor torque peaks at 0 RPM, holds flat to 3,000 RPM, decays to zero at 12,000 RPM
- Highest center of gravity (1,720 mm) -- body roll is pronounced in corners
- Most suspension travel (150 mm compression, 120 mm droop) -- absorbs terrain well but wallows under hard cornering
- Highest drag (0.32) and largest frontal area (2.8 m2) -- top speed limited by aero drag
- Widest tires (265 mm all around) -- good mechanical grip despite height

**Driving Tips**:
- Higher center of gravity - careful in corners
- Good acceleration despite size
- Stable in straight lines
- Best for: Mixed terrain, casual racing

---

## Tracks

### NIO City Circuit

**Neon-lit urban racing** (CARLA Town03)

- **Length**: 4.2 km
- **Laps**: 3
- **Checkpoints**: 12
- **Difficulty**: Medium
- **Environment**: Night, clear weather
- **Features**: Neon lights, wet surfaces, NIO swap station, city skyline
- **Lighting**: Night with neon glow, street lights, moon intensity 0.3
- **Tips**: Watch for slippery surfaces, use the swap station strategically. Wet patches reduce grip by ~20%. The neon reflections can mask the racing line -- focus on checkpoint markers.

### Shanghai Pudong

**Iconic cityscape racing** (CARLA Town06)

- **Length**: 5.1 km
- **Laps**: 3
- **Checkpoints**: 14
- **Difficulty**: Hard
- **Environment**: Sunset, partly cloudy
- **Features**: Oriental Pearl Tower, tunnel section, elevated highway, harbor view
- **Lighting**: Sunset with fog density 0.2, sun intensity 0.7
- **Tips**: Master the tunnel -- reduced visibility and acoustic changes can be disorienting. Watch for elevation changes on the elevated highway section. The sunset lighting creates long shadows that can hide track details.

### Speedway Oval

**High-speed oval racing** (Procedural)

- **Length**: 3.0 km
- **Laps**: 10
- **Checkpoints**: 4
- **Difficulty**: Easy
- **Environment**: Day, clear weather
- **Features**: Banked corners, grandstands, pit lane, high speeds
- **Lighting**: Noon, full sun intensity
- **Tips**: Maintain speed, use slipstream, minimal braking. The banked corners allow flat-out driving at speed. With 10 laps, tire management and battery conservation matter. Ideal track for learning drift initiation on the wide banked turns.

### Mountain Pass

**Technical mountain roads** (Custom)

- **Length**: 8.5 km
- **Laps**: 1
- **Checkpoints**: 20
- **Difficulty**: Hard
- **Environment**: Noon, misty (fog density 0.4)
- **Features**: Hairpin turns, elevation changes, cliff views, narrow road
- **Tips**: Smooth inputs are critical -- the narrow road leaves no room for error. Fog reduces visibility, so memorize the 20 checkpoints. Elevation changes affect braking distance: brake earlier on downhill sections, later on uphill. The EP9's downforce helps in fast sweepers but its wide track width makes tight hairpins tricky.

### Desert Rally

**Off-road desert racing** (Custom)

- **Length**: 12.0 km
- **Laps**: 1
- **Checkpoints**: 25
- **Difficulty**: Extreme
- **Environment**: Sunset, dusty (dust density 0.3)
- **Features**: Sand dunes, canyon sections, dust particles, point-to-point
- **Tips**: This is a point-to-point race (no laps). Terrain grip varies -- hard-packed sand is fast, soft dunes drain momentum. Dust clouds from opponents reduce visibility -- maintain safe distance or use the slipstream risk. The canyon section has walls on both sides with no runoff. Momentum is key: maintain throttle through soft sections rather than braking and re-accelerating.

---

## NOMI Co-Pilot

### What is NOMI?

NOMI is your in-car AI companion - a spherical robot that provides real-time commentary, encouragement, and assistance during races.

### NOMI Features

- **500+ Comments**: Context-aware commentary for every situation
- **Emotional Responses**: NOMI reacts to your driving with appropriate emotions
- **Smart Timing**: Comments are spaced to avoid overwhelming you
- **Frustration Protection**: Extra encouragement when you're struggling
- **Bilingual**: Comments delivered in Chinese with contextual references to driving situations

### How NOMI Works

NOMI uses a local matching engine to select comments based on race events. The system:

1. **Detects events** -- overtake, drift, lap completion, collision, high speed, position change
2. **Builds context** -- your position, lap, vehicle type, speed, drift duration
3. **Matches comments** -- selects from a pool of pre-written lines for that event
4. **Applies cooldowns** -- minimum 3 seconds between comments, max 2 queued
5. **Deduplicates** -- avoids repeating the last 10 comments

### Configuring NOMI

In Settings > Audio > NOMI Frequency (default: 0.7):
- **0.0** -- NOMI silent (disabled)
- **0.3** -- Minimal commentary (key moments only)
- **0.5** -- Moderate commentary
- **0.7** -- Default frequency
- **1.0** -- Maximum commentary (every detected event)

### NOMI Interaction Events

| Trigger | Example Comment |
|---------|----------------|
| Race start | "NOMI ready! Let's win this race!" |
| Overtake to 1st | "Beautiful overtake! You're in the lead!" |
| Overtake mid-pack | "Another one passed! Chasing the leaders!" |
| Being overtaken | "Don't worry, there's still time to fight back!" |
| Short drift (<2s) | "Nice drift through the corner!" |
| Long drift (2s+) | "Incredible! That drift lasted {duration} seconds!" |
| Fastest lap | "New fastest lap! {time}!" |
| Normal lap | "Lap {n} complete, {time}." |
| High speed | "Speed over {speed} km/h! Careful!" |
| Collision | "Oops! You hit something! Keep it together!" |
| Taking 1st place | "You're in first! Keep it up!" |
| Falling to last | "Don't give up! The race isn't over!" |
| Struggling | "Take a deep breath. You can do this!" |
| EP9-specific | "Four motors unleashed! 1,360 horsepower!" |
| ET7-specific | "ET7's comfort mode is smooth sailing!" |
| ES7-specific | "Who says SUVs can't race? ES7 proves them wrong!" |
| Battery swap station | "Swap station ahead! Want to swap batteries?" |

### NOMI Emotions

| Emotion | When It Happens |
|---------|-----------------|
| **Happy** | Good lap, clean racing |
| **Excited** | Overtakes, high speed |
| **Nervous** | Close calls, tight racing |
| **Surprised** | Unexpected events |
| **Celebrating** | Victories, records |
| **Concerned** | Collisions, mistakes |
| **Tired** | Long races |

### Comment Types

- **Overtake**: "Beautiful overtake! You're now in the lead!"
- **Drift**: "Amazing drift! 3.5 seconds of pure skill!"
- **Lap Complete**: "Lap 2 complete in 1:23.456"
- **Comfort**: "Don't worry, keep pushing! You've got this!"
- **NIO Specific**: "EP9's four motors are unleashed! 1,360 horses!"

---

## Tips & Strategies

### General Racing Tips

1. **Smooth Inputs**: Avoid jerky steering and sudden throttle/brake
2. **Look Ahead**: Anticipate corners and plan your line
3. **Consistency**: Consistent laps are faster than inconsistent hot laps
4. **Learn the Track**: Practice each track to learn braking points

### Electric Vehicle Physics

NIO vehicles use a unique electric motor torque curve: **peak torque at 0 RPM**, constant up to a decay point, then linearly dropping to zero at max RPM. This means:

- **Launch advantage**: Full torque available from a standstill -- no need to build RPM
- **Mid-range pull**: Torque stays flat through the mid-range (EP9: 0-4,000 RPM; ET7/ES7: 0-3,500 RPM)
- **Top-end fade**: Power drops off at high RPM -- shift your mental model from "gears" to "power band"
- **Regenerative braking**: Lifting off the throttle applies motor braking (EP9: 40%, ET7: 30%, ES7: 25%). This is not binary -- partial lift gives partial regen
- **Battery drain rate**: 0.01% per second under full throttle; regen recovers 0.005% per second
- **Motor overheating**: Extended high-speed driving raises motor temperature, which can reduce power output

### Electric Vehicle Tips

1. **Regen Braking**: Lift off throttle early to use regenerative braking as a third braking force
2. **Battery Management**: Monitor battery level, especially in long races. The EP9's 70 kWh battery drains faster than the ET7/ES7's 100 kWh
3. **Instant Torque**: Apply throttle progressively -- mashing the pedal causes wheel spin even with AWD
4. **Swap Stations**: Use NIO swap stations on NIO City Circuit to recharge instantly (NIO Championship mode only)
5. **Coasting**: On long straights, partial throttle uses less energy than full throttle with regen braking

### Cornering Technique

1. **Brake in a Straight Line**: Brake before the corner, not during
2. **Turn In**: Start turning at the apex
3. **Apex**: Hit the inside curb at the midpoint of the corner
4. **Exit**: Gradually apply throttle as you exit

### Drifting

1. **Initiation**: Use handbrake or lift-off oversteer to initiate drift
2. **Counter-Steer**: Steer opposite to the drift direction
3. **Throttle Control**: Modulate throttle to maintain drift angle
4. **Recovery**: Straighten wheels and apply throttle to exit drift

### Overtaking

1. **Slipstream**: Draft behind opponents to gain speed
2. **Inside Line**: Take the inside line for the next corner
3. **Late Braking**: Brake later than the opponent to pass
4. **Patience**: Wait for the right opportunity

---

## Achievements

### Starter Achievements

| Achievement | How to Unlock |
|-------------|---------------|
| **First Race** | Complete your first race |
| **First Win** | Win your first race |
| **Consistent Racer** | Complete 10 races |

### Skill Achievements

| Achievement | How to Unlock |
|-------------|---------------|
| **Speed Demon** | Reach 300 km/h |
| **Drift King** | Drift for a total of 60 seconds |
| **Perfect Lap** | Complete a lap without collisions |
| **Overtake Master** | Perform 100 overtakes |

### Challenge Achievements

| Achievement | How to Unlock |
|-------------|---------------|
| **Hard Difficulty** | Win on hard difficulty |
| **No Collision** | Complete a race without collisions |
| **Underdog** | Win from last place |
| **Comeback** | Win after being in last place |

### Collection Achievements

| Achievement | How to Unlock |
|-------------|---------------|
| **NIO Fan** | Race with all NIO vehicles |
| **All Tracks** | Race on all tracks |
| **Champion** | Win a championship |
| **Endurance** | Drive 100 km total |

---

## Unlockables

### Paint Jobs

- **Silver EP9**: Win 5 races with EP9
- **Gold EP9**: Win a championship

### Wheel Designs

- **Sport Wheels**: Complete 10 races
- **Luxury Wheels**: Reach 300 km/h

### NOMI Voices

- **Cheerful NOMI**: Drift for 30 seconds total
- **Professional NOMI**: Complete 10 races

---

## Replay System

After each race, a full replay is available from the post-race screen or by switching to Replay camera mode (V/Y key).

### Replay Controls

| Control | Action |
|---------|--------|
| **Playback speed** | 0.25x, 0.5x, 1.0x, 2.0x, 4.0x |
| **Timeline scrubbing** | Jump to any point in the race |
| **Pause/Resume** | Toggle playback freeze |
| **Camera angle** | Chase, Cinematic (auto-directed), Free Orbit, Track Side, Top Down, Bumper |

### Replay Camera Angles

- **Chase**: Follows behind the car as in normal gameplay
- **Cinematic**: Auto-directs between 10 different shot types (Wide Orbit, Bird's Eye, Dutch Angle, etc.) with smooth transitions
- **Free Orbit**: Mouse/stick-controlled orbit around the car
- **Track Side**: Fixed camera positions along the track
- **Top Down**: Bird's-eye view
- **Bumper**: Low front-bumper perspective

Replay data is recorded at approximately 30 Hz and stores up to 5 minutes of footage (300 seconds).

---

## Frequently Asked Questions

**Q: How do I save my progress?**
A: Progress is automatically saved after each race.

**Q: Can I change controls?**
A: Yes, controls can be customized in Settings > Controls.

**Q: How do I unlock new vehicles?**
A: Complete achievements and championships to unlock paint jobs and accessories.

**Q: What's the best vehicle for beginners?**
A: The NIO ET7 is the most balanced and forgiving vehicle.

**Q: How do I get better at drifting?**
A: Practice on the Speedway Oval - it has wide corners perfect for learning. Use handbrake (Space / A button) to initiate, then counter-steer and modulate throttle to hold the angle.

**Q: Why does NOMI comment so much?**
A: You can adjust comment frequency in Settings > Audio > NOMI Frequency. Set it to 0.0 to silence NOMI completely, or 0.3 for key moments only.

**Q: What's the difference between AI difficulties?**
A: Easy (85% speed, strong rubber band), Normal (95% speed, balanced), Hard (100% speed, aggressive overtaking), Expert (100% speed, no rubber band, near-perfect racing line). Start with Normal and work up.

**Q: Does the EP9 handle differently from the ET7/ES7?**
A: Yes. The EP9 is 645 kg lighter, has 2x the power, 4 motors instead of 2, carbon ceramic brakes, and generates 10x more downforce. It drives like a race car -- fast but demanding. The ET7 is the most forgiving; the ES7 has more body roll due to its height.

**Q: Can I watch replays of my races?**
A: Yes. Switch to Replay camera mode (V/Y key) after a race. You can scrub the timeline, change playback speed (0.25x to 4x), and switch between 6 camera angles including an auto-directed Cinematic mode.

---

*Happy Racing! 🏎️💨*

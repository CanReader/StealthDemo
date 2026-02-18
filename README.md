# StealthDemo

A third-person stealth action game built with **Unreal Engine 5.5** and **C++**. Navigate through increasingly challenging levels while avoiding AI-driven security cameras and patrol robots to reach the exit.

## Gameplay

The game features 3 mission levels with escalating difficulty:

| Level | Enemies | Fail Condition |
|-------|---------|----------------|
| **Level 1** | Security cameras | Any camera triggers an alert |
| **Level 2** | Cameras + patrol robots | AI alerts and kills the player |
| **Level 3** | Cameras + patrol robots | Same as Level 2, but the player is armed |

Players must use stealth mechanics — crouching, taking cover, and timing movements — to slip past enemies undetected. Each level is completed by reaching a trigger volume that advances to the next mission.

## AI System

Enemies operate on a multi-state awareness system powered by UE's **Perception System** (sight + hearing) and **Behavior Trees**.

### Awareness States

```
Unaware → Suspicious → Investigating → Alerted → Engaging
                                          ↓
                           LostTarget → Searching → ReturningToPatrol
                                          ↓
                                    AlarmTriggered
```

### Security Cameras

- **Patrol** — Sweep between random points with a green indicator light
- **Investigate** — On hearing a noise, the light turns yellow; the camera scans the sound origin for ~3 seconds before returning to patrol
- **Alert** — On confirmed visual contact, the light turns red and an alarm is triggered

### Patrol Robots

- **Patrol** — Roam between random reachable points in the level
- **Investigate** — Move to the location of a heard noise
- **Engage** — Chase and attack the player on sight

## Project Structure

```
Source/CanberkAIDemo/
├── AgentCharacter          # Player character
├── BaseCharacter           # Shared character base (combat + stealth components)
├── RoboAICharacter         # Robot enemy character
├── CameraPawn              # Security camera pawn
├── BaseAIController        # Core AI awareness & perception logic
├── RoboAIController        # Robot-specific AI behavior
├── CameraAIController      # Camera-specific AI behavior
├── CombatComponent         # Weapon management, HitScan, ammo
├── StealthComponent        # Crouch, cover system
├── InputHandlerComponent   # Enhanced Input action facade
├── MissionGameMode         # Level progression logic
├── Weapon                  # HitScan weapon system
├── CharacterAnimInstance   # Animation state synchronization
└── CharacterHUD / Overlay  # UI widgets
```

## Tech Stack

- **Engine**: Unreal Engine 5.5
- **Language**: C++ with Blueprint extensions
- **AI**: Behavior Trees, Blackboards, EQS, AI Perception (sight + hearing)
- **Input**: Enhanced Input System
- **Animation**: Animation Blueprints with AnimNotify-driven AI noise generation

## Building

Requires Unreal Engine 5.5 source or installed build.

```bash
# Generate project files
UnrealBuildTool -projectfiles -project="/path/to/StealthDemo.uproject" -game -engine

# Build (Development Editor)
UnrealBuildTool CanberkAIDemoEditor <Platform> Development -project="/path/to/StealthDemo.uproject"

# Package
RunUAT.sh BuildCookRun -project="StealthDemo.uproject" -platform=<Platform> -clientconfig=Development -cook -stage -pak
```

## License

This project is for demonstration and portfolio purposes.

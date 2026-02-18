# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

StealthDemo is a third-person stealth action game built in **Unreal Engine 5.5**. The module name is `CanberkAIDemo`. Players navigate levels avoiding/engaging AI enemies (robots and security cameras) to reach exit points across 3 mission levels.

## Build Commands

This is an Unreal Engine C++ project. All source is under `Source/CanberkAIDemo/`.

```bash
# Generate project files (from engine install, e.g. ~/UnrealEngine)
UnrealBuildTool -projectfiles -project="/path/to/StealthDemo.uproject" -game -engine

# Build (Development Editor, Linux example)
UnrealBuildTool CanberkAIDemoEditor Linux Development -project="/path/to/StealthDemo.uproject"

# Cook/Package
RunUAT.sh BuildCookRun -project="StealthDemo.uproject" -platform=Linux -clientconfig=Development -cook -stage -pak
```

No unit test framework is configured. There is no linter setup.

## Architecture

### Class Hierarchy

```
ACharacter → ABaseCharacter → AAgentCharacter (player)
                            → ARoboAICharacter (robot enemy)

AAIController → ABaseAIController → ARoboAIController (robot AI)
                                  → ACameraAIController (security camera AI)

APawn → ACameraPawn (security camera, not a Character)

AGameModeBase → AMissionGameMode (level progression: Level_1 → Level_2 → Level_3 → GameCompletion)
```

### Component Architecture

`ABaseCharacter` owns two key components:
- **UCombatComponent** — weapon management, HitScan firing, aiming, ammo tracking
- **UStealthComponent** — crouch mechanics (smooth capsule interpolation), cover system (wall detection via capsule traces)

`AAgentCharacter` (player) adds:
- **UInputHandlerComponent** — facade pattern centralizing all Enhanced Input actions

### AI System

`ABaseAIController` implements a 9-state awareness system (`EPlayerAwarenessState`): Unaware → Suspicious → Investigating → Alerted → Engaging → LostTarget → Searching → ReturningToPatrol → AlarmTriggered.

AI uses UE's **Perception System** (sight + hearing senses) and **Behavior Trees**. Custom BT tasks/services/decorators are Blueprint assets in `Content/AI/`. The base controller exposes virtual methods (`OnNoticedPlayer`, `OnDetectPlayer`, `OnHeardPlayer`, `OnLostPlayer`) that subclasses override.

### Weapon System

`AWeapon` — HitScan weapons with types: Rock, Pistol, Rifle, SubmachineGun. Supports primary/secondary slots, silencer attachments, scatter spread, and sound intensity for AI hearing.

### Animation

`UCharacterAnimInstance` syncs character state (speed, direction, aim offsets, weapon type, crouch) to animation blueprints. `UAnimNotify_CreateNoise` generates AI-audible noise during animations.

## Key Conventions

- **UE naming prefixes**: A (Actors), U (UObjects), F (structs), E (enums), b (booleans)
- **Code organization**: `#pragma region` blocks for logical grouping
- **Blueprint integration**: C++ base classes extended by Blueprints (BP_AgentCharacter, BP_RoboAI, BP_Camera)
- **Logging**: Custom categories `LOGAI`, `LogMainCharacter`; debug macros in `Debugger.h` (`message()`, `msg()`, `msg_clr()`)
- **Input**: Enhanced Input System with actions centralized in InputHandlerComponent

## Module Dependencies

Core, CoreUObject, Engine, InputCore, EnhancedInput, AnimGraphRuntime, AssetRegistry, AIModule (implicit via .uproject)

## Blueprints

### Blackboards (`Content/AI/`)

**BB_Base** (parent blackboard, inherited by BB_Camera and BB_Robot):
- `SelfActor` (Object) — self reference
- `Character` (Object) — detected character reference
- `LastSeenLocation` (Vector) — last known player position
- `HeardLocation` (Vector) — location of heard noise
- `bDidHear` (Bool) — whether AI heard something
- `bDidSee` (Bool) — whether AI saw the player
- `Awarness` (Enum) — current awareness state

**BB_Camera** (inherits BB_Base):
- All keys inherited from BB_Base, no additional keys

**BB_Robot** (inherits BB_Base, adds own keys):
- Inherited: all BB_Base keys
- `bIsAttacking` (Bool) — whether robot is currently attacking
- `TargetLocation` (Vector) — movement target for patrol/chase

**BB_NPC** (standalone, no inheritance):
- `SelfActor` (Object) — self reference
- `LastKnownLocation` (Vector) — last known target position
- `TargetActor` (Object) — target actor reference

### Behavior Trees (`Content/AI/`)

**BT_Camera** (blackboard: BB_Camera):
```
ROOT → Selector
├── [1] "Noise Detected" — Sequence
│   ├── Decorator: Blackboard (bDidHear Is Set)
│   ├── BTTask_ChangeAwarnessState → set to Investigating
│   ├── BTTask_CameraMove → look toward HeardLocation
│   └── Wait
├── [2] "The Player Detected" — Sequence
│   ├── Decorator: Blackboard (bDidSee Is Set)
│   ├── Service: BTService_FollowObject — tracks player position
│   ├── Service: BTService_CheckIfSeenInHeard — updates bDidSee/bDidHear
│   ├── BTTask_ChangeAwarnessState → set to Alerted/Engaging
│   ├── Selector
│   │   ├── "Try to Follow Player" — camera follows detected player
│   │   └── "Trigger Alarm" — escalate to alarm state
│   └── Wait
└── [3] "The Player Not Detected" — Sequence
    ├── Decorator: Blackboard (bDidSee Is Not Set)
    ├── BTTask_CameraRotateSpeed → set rotation speed
    ├── BTTask_CameraRandomPoint → pick random patrol point
    ├── BTTask_CameraMovePoint → move to patrol point
    └── Wait
```

**BT_Robot** (blackboard: BB_Robot):
```
ROOT → Selector
├── [1] "Noise Detected" — Sequence
│   ├── Decorator: Blackboard (bDidHear Is Set)
│   ├── BTTask_ChangeAwarnessState → set to Investigating
│   ├── BTTask_Investigate → move to HeardLocation
│   └── Wait
├── [2] "Attempt to Attack" — Sequence
│   ├── Decorator: Blackboard (bDidSee Is Set)
│   ├── Decorator: Blackboard (bIsAttacking Is Set)
│   ├── Service: BTService_Attacking — handles combat
│   ├── MoveTo → chase player
│   ├── RotateToFaceBBEntry → face target
│   └── Wait
└── [3] "The Player Not Detected" — Sequence
    ├── Decorator: Blackboard (bDidSee Is Not Set)
    ├── Service: BTService_GetRandomLocation — picks patrol point
    ├── BTTask_MoveRandom → move to TargetLocation
    ├── RotateToFaceBBEntry
    └── Wait
```

### Decorators (`Content/AI/Decorators/`)

**BTDecorator_PlayerInSight** — PerformConditionCheckAI:
1. Get AI Controller → Get Controlled Pawn
2. Get Blackboard Key Selector
3. Check sight perception on the controlled pawn
4. Return true/false based on whether player is within sight cone

### Services (`Content/AI/Services/`)

**BTService_CheckIfSeenInHeard** — ReceiveActivationAI:
1. Get AI Controller → Get Controlled Pawn
2. Read blackboard keys `bDidSee`, `bDidHear`
3. Query AI perception system for sight/hearing stimuli
4. Update blackboard: set `bDidSee`, `bDidHear`, `Character` (detected player ref)

**BTService_FollowObject** — ReceiveActivationAI:
1. Get AI Controller → Get Controlled Pawn → Cast to BP_Camera
2. Get camera's current focus target
3. Update target location to follow the detected player
4. On ReceiveDeactivation: clear/reset target

**BTService_GetRandomLocation** — ReceiveSearchStartAI:
1. Get Owner Controller → Get Controlled Pawn → Get Actor Location
2. Call GetRandomReachablePointInRadius around current location
3. Set blackboard value `TargetLocation` to the random point

**BTService_IsPlayerInSight** — ReceiveSearchStartAI:
1. Get Owner Controller → Cast to BP_AgentCharacter (get player ref)
2. Get AI perception → check dot product / line of sight
3. Set blackboard bool based on sight check result

**BTService_Attacking** — (used by BT_Robot, handles combat ticking)

**BTService_PlayerLoc** — (player location tracking)

### Tasks (`Content/AI/Tasks/`)

**BTTask_CameraMove** — ReceiveExecuteAI:
1. Get AI Controller → Get Controlled Pawn → Cast to BP_Camera
2. Set rotation speed on the camera pawn
3. Move camera to target location
4. Finish Execute (Success/Fail)

**BTTask_CameraMoveActor** — ReceiveExecuteAI:
1. Get AI Controller → Get Controlled Pawn → Cast to BP_Camera
2. AI MoveTo Actor (follow a specific actor)
3. Finish Execute with result

**BTTask_CameraMovePoint** — ReceiveExecuteAI:
1. Get AI Controller → Get Controlled Pawn → Cast to BP_Camera
2. Get target navigation point
3. AI MoveTo the point location
4. Finish Execute (Success/Fail)

**BTTask_CameraRandomPoint** — ReceiveExecuteAI:
1. Get AI Controller → Get Controlled Pawn → Cast to BP_Camera
2. GetRandomReachablePointInRadius from current location
3. Set destination from random point
4. Finish Execute

**BTTask_CameraRotateSpeed** — ReceiveExecuteAI:
1. Get AI Controller → Get Controlled Pawn → Cast to BP_Camera
2. Set RotateSpeed property on the camera pawn (exposed variable)
3. Finish Execute (Success)

**BTTask_ChangeAwarnessState** — ReceiveExecuteAI:
1. Get AI Controller → Cast to BaseAIController
2. Set awareness state to exposed variable value (e.g. Investigating, Alerted)
3. Finish Execute (Success)

**BTTask_Investigate** — ReceiveExecuteAI:
1. Get AI Controller → Get Controlled Pawn
2. Read blackboard: get `HeardLocation` or `LastSeenLocation`
3. AI MoveTo that location
4. On move complete: check result, update awareness state
5. Finish Execute (Success/Fail) based on movement result

**BTTask_MoveRandom** — ReceiveExecuteAI:
1. Get Owner Controller → Get Controlled Pawn
2. Read blackboard `TargetLocation`
3. Get Navigation System → AI MoveTo target location
4. On move complete → Finish Execute (Success/Fail)

**BTTask_Print** — Debug task for printing to screen/log

### Other AI Assets (`Content/AI/`)
- `BP_RoboAI` — Robot AI controller blueprint (extends `ARoboAIController`)
- `BP_CameraAI` — Camera AI controller blueprint (extends `ACameraAIController`)
- `E_Awarness` — Awareness enum (Blueprint)
- `EQS_Camera`, `EQS_Robot` — Environment Query assets

### Game Blueprints (`Content/Blueprints/`)
- `BP_AgentCharacter` — Player character (extends `AAgentCharacter`)
- `BP_Camera` — Security camera (extends `ACameraPawn`)
- `BP_HUD` — HUD (extends `ACharacterHUD`)
- `BP_Gamemode` — Game mode (extends `AMissionGameMode`)

### Widget Blueprints (`Content/Widgets/`)
- `WBP_Alert` — Alert widget (extends `UAlertWidget`)
- `WBP_HUD` — HUD overlay (extends `UCharacterOverlay`)

### Animation Blueprints
- `ABP_Agent` (`Content/Characters/Agent/Animations/`) — Agent anim blueprint (extends `UCharacterAnimInstance`)


## Notes:
There are 3 levels in the game:
In the first level, player are trying to escape without getting caught to Cameras. Player goes to level 2 if they enter to TriggerVolume, this handled by Level Blueprint.
There are 3 states of cameras:
Patrolling, Investigate, Noticed Player, Alerted
* By default, Camera will patrol from one point to another (or multiple or random, pick the best choice) while patrolling it will wait to hear or see something; There are two choices:
* When a camera hears something light will be turned to yellow. it will investigate sound from sound location, I mean it will look 3 points with 3 seconds wait. If don't see anything, Light will go back to green and state go back to patrol. If it sees player while investigating sound it will alert immediately.
* When a camera sees a player while patroling, make camera yellow and investigate state. It will follow player for 5 seconds, if lose sight then it will invastigate with faster speed. If still in sight after 5 sconds it will go alert 


In the first level there will be only cameras, level restarted a camera alerted
In the second level there will be cameras and robots, level restarted if any AI is alerted and killed Player
In the third level all same with level 2 but player has a weapon 




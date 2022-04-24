# AbilitySystemSetup
Collaborators: ChristianHinko, brian2524
Current engine version: 4.27.2

The pourpose of this plugin is to provide a reusable base system that extends Epic\'s Gameplay Ability System (GAS) plugin. It provides necissary code for the setup of GAS.

## Setup
Goto your project settings to configure game specific data required for GAS setup.
![Plugin Project Settings](/Images/Readme/PluginProjectSettings.png)
The plugin will ensure that entered values are correct and notify if you they aren\'t

## Key Features
- **Plugin Settings** - Configurable settings that appear in your project settings
- **Ability Sets** - Grants Abilities, Effects, and Attribute Sets as a group to an `AbilitySystemComponent`, providing an output handle for tracking.
- **Ability System Setup Component** - Provides common GAS initialization/uninitialization logic with `AbilitySets` that are granted while initialized.
	- Supports any `AbilitySystemComponent` location
	- Supports any Actor as the `AvatarActor`
	- Supports GAS input bindings
- **ASSEngineSubsystem** - Used to load global data tables and tags via ``UAbilitySystemGlobals::InitGlobalData()``
- **Engine only safety checks** - Ensures silent errors don\'t go unnoticed
- **Ability to reuse TargetActors**
- **Custom Target Data Filter** - An improved method of handling the filtering process
	- Class whitelist array
	- Class blacklist array
	- Option to only allow classes implementing `IAbilitySystemInterface`
- **Base classes for common types** - Provides quality of life features and extra functionality
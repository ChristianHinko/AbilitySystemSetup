# AbilitySystemSetup
Provides a reusable base Ability System that extends Epic\'s Gameplay Ability System (GAS) plugin. Provides the necessary code for the setup of GAS.

Collaborators: ChristianHinko, brian2524

Current engine version: 5.0.3

## Key Features
- **Ability Sets** - Grants Abilities, Effects, and Attribute Sets as a group to an `AbilitySystemComponent`, providing an output handle for tracking.
- **Ability System Setup Component** - Provides common GAS initialization/uninitialization logic with `AbilitySets` that are granted while initialized.
	- Supports any `AbilitySystemComponent` location
	- Supports any Actor as the `AvatarActor`
	- Supports GAS input bindings
- **ASSEngineSubsystem** - Used to load global data tables and tags via ``UAbilitySystemGlobals::InitGlobalData()``
- **Editor-only safety checks** - Ensures silent errors don\'t go unnoticed
- **Ability to reuse TargetActors**
- **Custom Target Data Filters** - Improved method of handling the filtering process
	- Class whitelist array
	- Class blacklist array
	- Option to only allow classes implementing `IAbilitySystemInterface`
- **Base classes for common types** - Provides quality of life features and extra functionality
- **Corrects `OnAvatarSet()` Engine bug** - The event doesn\'t properly get called on instanced abilities when only switching the AvatarActor. Resolved by marking the event as final and forwards working calls to `OnAvatarSetThatWorks()`.

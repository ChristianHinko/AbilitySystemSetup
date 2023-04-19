# AbilitySystemSetup
This is a C++ plugin extending Epic's Gameplay Ability System, providing a foundation and an efficient workflow for using it. Its goals are to speed up development, eliminate boilerplate code, and provide good design.



Collaborators: ChristianHinko, brian2524

Current engine version: 5.1.0

## Key Features
- **Ability Sets** - Grants Abilities, Effects, and Attribute Sets as a group to an `AbilitySystemComponent`, providing an output handle for tracking.
- **AvatarActorExtension Component** - Generalized component that can initialize/uninitialize an Avatar Actor, given an AbilitySystemComponent (and optional AbilitySets).
	- It stays agnostic to AbilitySystemComponent location
	- Supports any Avatar Actor
		- Pawn Avatar Actors use a more specialized "PawnAvatarActorExtension" component which inherits and provides extra GAS input binding logic.
- **ASSEngineSubsystem** - Used to load global data tables and tags via ``UAbilitySystemGlobals::InitGlobalData()``
- **Editor-only checks** - Ensures proper workflow is followed
- **TargetActor reusability**
- **Custom Target Data Filters** - Improved method of handling the filtering process
	- Class whitelist array
	- Class blacklist array
	- Option to only allow classes implementing `IAbilitySystemInterface`
- **Corrects `OnAvatarSet()` Engine bug** - The event doesn\'t properly get called on instanced abilities when only switching the AvatarActor. Resolved by marking the event as final and forwards working calls to `OnAvatarSetThatWorks()`.

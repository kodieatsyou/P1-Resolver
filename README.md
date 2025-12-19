## Ability / Status Resolver

This module implements a deterministic, data-driven ability resolution system
inspired by modern RPG combat engines.

Features:
- Abilities and statuses loaded from JSON
- Deterministic resolution order
- Status-driven modifier hooks (OnBeforeDealDamage / OnBeforeTakeDamage)
- Stack-aware modifiers
- Full resolution trace for debugging and testing
- Unit tests validating numeric outcomes and modifier application

Design goals:
- Separation of data loading, validation, and execution
- No runtime string lookups during resolution
- Stable ordering for replay and testing

# Performance Notes

This section tracks performance-oriented algorithm evolution by version.

---

## Current Version

**v0.9.3** tuned the variant inline dispatch threshold for common workloads.
See [v0.9.3 performance note](v0.9.3.md) for details.

v0.8.2 established the lowering engine baseline.
See [v0.8.2 performance note](v0.8.2.md) for benchmark data and algorithm details.

Releases from v0.8.3 to v0.9.2 focused on guard fixes, API ergonomics, and
compiler compatibility. No new dispatch algorithms were introduced.

---

## Version Index

- [v0.9.3](v0.9.3.md) (variant dispatch threshold tuning)
- [v0.8.2](v0.8.2.md) (lowering engine baseline)
- [v0.8.0](v0.8.0.md) (tiered variant dispatch)

---

## Scope

Performance notes focus on:

- dispatch strategy evolution
- fast-path and cold-path behavior
- algorithm-level tradeoffs and applicability

For API compatibility and migration notes, see:

- [Changelog Releases](../changelog/releases.md)
- [API Reference](../api.md)

# Maintainer docs (staged for openlabs)

These 3 files satisfy sub-issue #162's "maintainer docs" checklist item
("Challenge design, official solution, and validation evidence"). Unlike
VaultCorp's 4-file web-track set, this lab uses exactly the 3 names
M9nx's checklist named for the binary track, matching the structural
pattern of `labs/web/firmdrama/docs/` (the only existing lab with this
doc convention) scaled down for an offline, artifact-only binary lab —
no `operations.mdx` or `security-report.mdx`, since there is no running
service, no network trust boundary, and no deployment to operate.

Reorganized from the original narrative write-ups under
`../documentation/` (`Objective ....md`, `Walkthrough ....md`, `What
Happened and Why (Root Cause) ....md`, `Mitigation ....md`). `Public
Briefing ....md` is kept as-is — it's the player-facing brief and isn't
duplicated here.

Note: `labs/binary/` doesn't exist yet anywhere in `duckurity/openlabs` —
this will be the catalog's first binary-track lab, confirmed by checking
the live repository before writing these docs.

Staged here only, per the standing rule: nothing gets pushed to
`duckurity/openlabs` until all 3 sub-issues are agreed with the
maintainer. Final home once agreed:

```
labs/binary/dual-check/docs/challenge-design.mdx
labs/binary/dual-check/docs/official-solution.mdx
labs/binary/dual-check/docs/validation-report.mdx
```

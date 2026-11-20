# Development changelog VER024 — eight-lake storage and map stabilization


## Eight-lake development checkpoint — 2026-11-20
Remote package marker remains 028; Boat 014. Source now has eight SD lake profiles, sixteen mission slots and separate Home per profile, checked active selection, recoverable name and mission records, and A/B offline map installation. Lake selection UI uses standard LVGL keyboard and confirmed STOP/manual/connected-radio/inactive-motor gates. Mission execution remains blocked; boat-side synchronization is not implemented.
Latest observed before this update: host #41 passed (6a3d88c), packaging #977 passed, target #42 running. Host #41 did not exercise the new SD storage. This update adds a mock SD host test compiling actual lake_storage.cpp: eight profiles, safe switching, name persistence, isolated paths, map marker rename failure, short writes and unavailable card. Results pending. The mock does not test actual FAT/SD durability, mission serialization or LVGL. Verify new CI and fix real failures before bumping the release marker. Hardware remains untested.

Previous commits: 8570bd8 (manifest/overflow), c19e064 (lake foundation), 4afec26 (mission records), 9bf8751 (map paths), c6d6633 (A/B install), c6126bf (lake UI), 6a3d88c (compile correction).


## Stabilization audit continuation — 2026-11-20
- Host #42 verified passed all five groups on 6a05b2d, including the actual lake_storage.cpp mocked SD test; ZIP packaging #978 passed. Target #43 was still running at review. Prior successful full target compile #19 is historical, not evidence for lake changes.
- Audited lake, mission, map reader/import/render, UI, startup and safety interfaces. Added mission_storage.cpp/.h (same legacy record layout), actual 128-slot isolated mission serialization/CRC/failure tests, record validation and mission-screen async deletion. Fixed stale coverage stats and protected A/B fallback marker before overwriting inactive map slot; copied files now reopened for size verification.
- Next: observe new host and target build, correct actual errors, audit remaining Boat/Remote modules (full branch-wide review is not complete); run map importer/coverage parser fault tests and hardware checks. Do not enable autonomous movement or claim release readiness. No version bump until build/test gates pass.

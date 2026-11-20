# Eight-lake storage foundation
Eight directories Lake_01 through Lake_08, zero-based internal IDs, checked active selection with backup recovery. Selection requires confirmed STOP, manual mode, connected radio, inactive motors and no failsafe.
This foundation is not yet wired into mission or map readers. Do not enable lake selection in UI until all consumers reload atomically and navigation state is synchronized. Legacy migration, per-lake mission records, map transaction, tests and target compilation remain outstanding.


## Eight-lake development checkpoint — 2026-11-20
Remote package marker remains 028; Boat 014. Source now has eight SD lake profiles, sixteen mission slots and separate Home per profile, checked active selection, recoverable name and mission records, and A/B offline map installation. Lake selection UI uses standard LVGL keyboard and confirmed STOP/manual/connected-radio/inactive-motor gates. Mission execution remains blocked; boat-side synchronization is not implemented.
Latest observed before this update: host #41 passed (6a3d88c), packaging #977 passed, target #42 running. Host #41 did not exercise the new SD storage. This update adds a mock SD host test compiling actual lake_storage.cpp: eight profiles, safe switching, name persistence, isolated paths, map marker rename failure, short writes and unavailable card. Results pending. The mock does not test actual FAT/SD durability, mission serialization or LVGL. Verify new CI and fix real failures before bumping the release marker. Hardware remains untested.
